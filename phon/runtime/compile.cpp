/**************************************************************************************
 * Copyright (C) 2013-2019, Artifex Software                                          *
 *           (C) 2019, Julien Eychenne <jeychenne@gmail.com>                          *
 *                                                                                    *
 * Permission to use, copy, modify, and/or distribute this software for any purpose   *
 * with or without fee is hereby granted, provided that the above copyright notice    *
 * and this permission notice appear in all copies.                                   *
 *                                                                                    *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH      *
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND    *
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, *
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,     *
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    *
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS        *
 * SOFTWARE.                                                                          *
 *                                                                                    *
 **************************************************************************************/

#include <math.h>
#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/lex.hpp>
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp> /* for jsV_numbertostring */


#define cexp jsC_cexp /* collision with math.h */

#define JF Runtime *J, Function *F

namespace phonometrica {


PHON_NORETURN void jsC_error(Runtime *J, Ast *node, const char *fmt, ...) PHON_PRINTFLIKE(3, 4);

static void cfunbody(JF, Ast *name, Ast *params, Ast *body);

static void cexp(JF, Ast *exp);

static void cstmlist(JF, Ast *list);

static void cstm(JF, Ast *stm);

void jsC_error(Runtime *J, Ast *node, const char *fmt, ...)
{
    va_list ap;
    char buf[512];
    char msgbuf[256];

    va_start(ap, fmt);
    vsnprintf(msgbuf, 256, fmt, ap);
    va_end(ap);

    snprintf(buf, 256, "%s:%d: ", J->filename.data(), node->line);
    strcat(buf, msgbuf);

    throw std::runtime_error(buf);
}

static const String futurewords[] = {
        "class", "const", "enum", "extends", "import", "super",
};

static const String strictfuturewords[] = {
        "implements", "interface", "let", "package", "private", "protected",
        "public", "static", "yield",
};

static void checkfutureword(JF, Ast *exp)
{
    if (find_word(exp->string, futurewords, nelem(futurewords)) >= 0)
    jsC_error(J, exp, "'%s' is a future reserved word", exp->string.data());

    if (F->strict)
    {
        if (find_word(exp->string, strictfuturewords, nelem(strictfuturewords)) >= 0)
            jsC_error(J, exp, "'%s' is a strict mode future reserved word", exp->string.data());
    }
}

static Function *newfun(Runtime *J, Ast *name, Ast *params, Ast *body, int script, int default_strict)
{
    auto F = new Function;
    F->gcmark = 0;
    F->gcnext = J->gcfun;
    J->gcfun = F;
    ++J->gccounter;

    F->filename = J->internalize(J->filename);
    F->line = name ? name->line : params ? params->line : body ? body->line : 1;
    F->script = script;
    F->strict = default_strict;
    F->name = name ? name->string : "";

    cfunbody(J, F, name, params, body);

    return F;
}

/* Emit opcodes, constants and jumps */

static void emitraw(JF, int value)
{
    if (value != (instruction_t) value)
        throw J->raise("Syntax error", "integer overflow in instruction coding");
    F->code.push_back(instruction_t(value));
}

static void emit(JF, int value)
{
    emitraw(J, F, value);
}

static void emit_size(JF, intptr_t value)
{
    SizeOpcode op = { value };

    for (int i = 0; i < OpSize; i++)
        F->code.push_back(op.ins[i]);
}

static void emitline(JF, Ast *node)
{
    if (F->lastline != node->line)
    {
        F->lastline = node->line;
        emit(J, F, OP_LINE);
        emitraw(J, F, node->line);
    }
}

static int addfunction(JF, Function *value)
{
    auto index  = (int) F->funtab.size();
    F->funtab.push_back(value);

    return index;
}

static int addnumber(JF, double value)
{
    int i;
    for (i = 0; i < F->numtab.size(); ++i)
        if (F->numtab[i] == value)
            return i;

    i = (int) F->numtab.size();
    F->numtab.push_back(value);

    return i;
}

static int addstring(JF, const String &value)
{
    for (auto i = 0; i < F->strtab.size(); ++i)
        if (F->strtab[i] == value)
            return i;

    auto index = (int) F->strtab.size();
    F->strtab.push_back(value);

    return index;
}

static void addlocal(JF, Ast *ident, int reuse)
{
    auto name = ident->string;

    if (F->strict)
    {
        if (name == "arguments")
            jsC_error(J, ident, "redefining 'arguments' is not allowed in strict mode");
        if (name == "eval")
            jsC_error(J, ident, "redefining 'eval' is not allowed in strict mode");
    }
    if (reuse || F->strict)
    {
        for (intptr_t i = 0; i < F->vartab.size(); ++i)
        {
            if (F->vartab[i] == name)
            {
                if (reuse)
                    return;
                if (F->strict)
                    jsC_error(J, ident, "duplicate formal parameter '%s'", name.data());
            }
        }
    }

    F->vartab.push_back(name);
}

static int findlocal(JF, const String &name)
{
    for (intptr_t i = F->vartab.size(); i > 0; --i)
        if (F->vartab[i - 1] == name)
            return i;
    return -1;
}

static void emitfunction(JF, Function *fun)
{
    emit(J, F, OP_CLOSURE);
    emitraw(J, F, addfunction(J, F, fun));
}

static void emitnumber(JF, double num)
{
    if (num == 0)
    {
        emit(J, F, OP_NUMBER_0);
        if (signbit(num))
            emit(J, F, OP_NEG);
    }
    else if (num == 1)
    {
        emit(J, F, OP_NUMBER_1);
    }
    else if (num == (instruction_t) num)
    {
        emit(J, F, OP_NUMBER_POS);
        emitraw(J, F, (instruction_t) num);
    }
    else if (num < 0 && -num == (instruction_t) (-num))
    {
        emit(J, F, OP_NUMBER_NEG);
        emitraw(J, F, (instruction_t) (-num));
    }
    else
    {
        emit(J, F, OP_NUMBER);
        emitraw(J, F, addnumber(J, F, num));
    }
}

static void emitstring(JF, int opcode, const String &str)
{
    emit(J, F, opcode);
    emitraw(J, F, addstring(J, F, str));
}

static void emitlocal(JF, int oploc, int opvar, Ast *ident)
{
    int i;
    checkfutureword(J, F, ident);
    if (F->strict && oploc == OP_SETLOCAL)
    {
        if (ident->string == "arguments")
            jsC_error(J, ident, "'arguments' is read-only in strict mode");
        if (ident->string == "eval")
            jsC_error(J, ident, "'eval' is read-only in strict mode");
    }
    if (F->lightweight)
    {
        i = findlocal(J, F, ident->string);
        if (i >= 0)
        {
            emit(J, F, oploc);
            emitraw(J, F, i);
            return;
        }
    }
    emitstring(J, F, opvar, ident->string);
}

static int here(JF)
{
    return (int) F->code.size();
}

static int emitjump(JF, int opcode)
{
    int inst = int(F->code.size() + 1);
    emit(J, F, opcode);
    emitraw(J, F, 0);
    return inst;
}

static void emitjumpto(JF, int opcode, int dest)
{
    emit(J, F, opcode);
    if (dest != (instruction_t) dest)
        throw J->raise("Syntax error", "jump address integer overflow");
    emitraw(J, F, dest);
}

static void labelto(JF, int inst, int addr)
{
    if (addr != (instruction_t) addr)
        throw J->raise("Syntax error", "jump address integer overflow");
    F->code[inst] = addr;
}

static void label(JF, int inst)
{
    labelto(J, F, inst, (int)F->code.size());
}

/* Expressions */

static void ctypeof(JF, Ast *exp)
{
    if (exp->type == EXP_IDENTIFIER)
        emitlocal(J, F, OP_GETLOCAL, OP_HASVAR, exp);
    else
        cexp(J, F, exp);
    emit(J, F, OP_TYPEOF);
}

static void cunary(JF, Ast *exp, int opcode)
{
    cexp(J, F, exp->a);
    emit(J, F, opcode);
}

static void cbinary(JF, Ast *exp, int opcode)
{
    cexp(J, F, exp->a);
    cexp(J, F, exp->b);
    emit(J, F, opcode);
}

static void clist(JF, Ast *list, bool is_array = false)
{
    int i = 0;

    // First get the length
    intptr_t size = 0;
    auto l = list;
    while (l)
    {
        size++;
        l = l->b;
    }
    if (!is_array) emit_size(J, F, size);

    // Emit fields.
    while (list)
    {
        emitnumber(J, F, ++i);
        emitline(J, F, list->a);
        cexp(J, F, list->a);
        emit(J, F, OP_INITPROP);

        list = list->b;
    }
}

static void carray(JF, Ast *array)
{
	auto nrow = reinterpret_cast<intptr_t>(array->c);
	auto ncol = reinterpret_cast<intptr_t>(array->d);
	emit_size(J, F, nrow);
	emit_size(J, F, ncol);

	if (nrow == 1)
	{
		// One dimensional array.
		clist(J, F, array, true);
		return;
	}

	intptr_t i = 1, j = 0;

	// Emit fields.
	while (array)
	{
		emitnumber(J, F, i);
		emitnumber(J, F, ++j);
		if (j == ncol)
		{
			i++;
			j = 0;
		}
		//emitline(J, F, array->a);
		cexp(J, F, array->a);
		emit(J, F, OP_SETPROPX);
		emit(J, F, 2); // argc

		array = array->b;
	}
}

static void checkdup(JF, Ast *list, Ast *end)
{
    char nbuf[32], sbuf[32];
    String needle, straw;

    if (end->a->type == EXP_NUMBER)
        needle = number_to_string(J, nbuf, end->a->number);
    else
        needle = end->a->string;

    while (list->a != end)
    {
        if (list->a->type == end->type)
        {
            Ast *prop = list->a->a;
            if (prop->type == EXP_NUMBER)
                straw = number_to_string(J, sbuf, prop->number);
            else
                straw = prop->string;
            if (needle == straw)
                jsC_error(J, list, "duplicate field '%s' in object literal", needle.data());
        }
        list = list->b;
    }
}

static void cobject(JF, Ast *list)
{
    Ast *head = list;

    while (list)
    {
        Ast *kv = list->a;
        Ast *prop = kv->a;

        if (prop->type == AST_IDENTIFIER || prop->type == EXP_STRING)
            emitstring(J, F, OP_STRING, prop->string);
        else if (prop->type == EXP_NUMBER)
            emitnumber(J, F, prop->number);
        else
            jsC_error(J, prop, "invalid field name in object initializer");

        if (F->strict)
            checkdup(J, F, head, kv);

        switch (kv->type)
        {
        default: /* impossible */ break;
        case EXP_PROP_VAL:
            emitline(J, F, kv->b);
            cexp(J, F, kv->b);
            emit(J, F, OP_INITPROP);
            break;
        case EXP_PROP_GET:
            emitfunction(J, F, newfun(J, nullptr, nullptr, kv->c, 0, F->strict));
            emit(J, F, OP_INITGETTER);
            break;
        case EXP_PROP_SET:
            emitfunction(J, F, newfun(J, nullptr, kv->b, kv->c, 0, F->strict));
            emit(J, F, OP_INITSETTER);
            break;
        }

        list = list->b;
    }
}

static int cargs(JF, Ast *list)
{
    int n = 0;
    while (list)
    {
        cexp(J, F, list->a);
        list = list->b;
        ++n;
    }
    return n;
}

static void cassign(JF, Ast *exp)
{
    Ast *lhs = exp->a;
    Ast *rhs = exp->b;
    switch (lhs->type)
    {
    case EXP_IDENTIFIER:
        cexp(J, F, rhs);
        emitlocal(J, F, OP_SETLOCAL, OP_SETVAR, lhs);
        break;
    case EXP_INDEX:
        cexp(J, F, lhs->a);
        if (lhs->b->type == EXP_COMMA)
        {
	        cexp(J, F, lhs->b->a);
	        cexp(J, F, lhs->b->b);
	        cexp(J, F, rhs);
	        emit(J, F, OP_SETPROPX);
			emit(J, F, 2); // argc
        }
        else
        {
	        cexp(J, F, lhs->b);
	        cexp(J, F, rhs);
	        emit(J, F, OP_SETPROP);
        }
        break;
    case EXP_MEMBER:
        cexp(J, F, lhs->a);
        cexp(J, F, rhs);
        emitstring(J, F, OP_SETPROP_S, lhs->b->string);
        break;
    default:
        jsC_error(J, lhs, "invalid l-value in assignment");
    }
}

static void cassignforin(JF, Ast *stm)
{
    Ast *lhs = stm->a;

    if (stm->type == STM_FOR_IN_VAR)
    {
        if (lhs->b)
            jsC_error(J, lhs->b, "more than one loop variable in for-in statement");
        emitlocal(J, F, OP_SETLOCAL, OP_SETVAR, lhs->a->a); /* list(var-init(ident)) */
        emit(J, F, OP_POP);
        return;
    }

    switch (lhs->type)
    {
    case EXP_IDENTIFIER:
        emitlocal(J, F, OP_SETLOCAL, OP_SETVAR, lhs);
        emit(J, F, OP_POP);
        break;
    case EXP_INDEX:
        cexp(J, F, lhs->a);
        cexp(J, F, lhs->b);
        emit(J, F, OP_ROT3);
        emit(J, F, OP_SETPROP);
        emit(J, F, OP_POP);
        break;
    case EXP_MEMBER:
        cexp(J, F, lhs->a);
        emit(J, F, OP_ROT2);
        emitstring(J, F, OP_SETPROP_S, lhs->b->string);
        emit(J, F, OP_POP);
        break;
    default:
        jsC_error(J, lhs, "invalid l-value in for-in loop assignment");
    }
}

static void cassignop1(JF, Ast *lhs)
{
    switch (lhs->type)
    {
    case EXP_IDENTIFIER:
        emitlocal(J, F, OP_GETLOCAL, OP_GETVAR, lhs);
        break;
    case EXP_INDEX:
        cexp(J, F, lhs->a);
        cexp(J, F, lhs->b);
        emit(J, F, OP_DUP2);
        emit(J, F, OP_GETPROP);
        break;
    case EXP_MEMBER:
        cexp(J, F, lhs->a);
        emit(J, F, OP_DUP);
        emitstring(J, F, OP_GETPROP_S, lhs->b->string);
        break;
    default:
        jsC_error(J, lhs, "invalid l-value in assignment");
    }
}

static void cassignop2(JF, Ast *lhs, int postfix)
{
    switch (lhs->type)
    {
    case EXP_IDENTIFIER:
        if (postfix) emit(J, F, OP_ROT2);
        emitlocal(J, F, OP_SETLOCAL, OP_SETVAR, lhs);
        break;
    case EXP_INDEX:
        if (postfix) emit(J, F, OP_ROT4);
        emit(J, F, OP_SETPROP);
        break;
    case EXP_MEMBER:
        if (postfix) emit(J, F, OP_ROT3);
        emitstring(J, F, OP_SETPROP_S, lhs->b->string);
        break;
    default:
        jsC_error(J, lhs, "invalid l-value in assignment");
    }
}

static void cassignop(JF, Ast *exp, int opcode)
{
    Ast *lhs = exp->a;
    Ast *rhs = exp->b;
    cassignop1(J, F, lhs);
    cexp(J, F, rhs);
    emit(J, F, opcode);
    cassignop2(J, F, lhs, 0);
}

static void cdelete(JF, Ast *exp)
{
    switch (exp->type)
    {
    case EXP_IDENTIFIER:
        if (F->strict)
            jsC_error(J, exp, "destroy on an unqualified name is not allowed in strict mode");
        emitlocal(J, F, OP_DELLOCAL, OP_DELVAR, exp);
        break;
    case EXP_INDEX:
        cexp(J, F, exp->a);
        cexp(J, F, exp->b);
        emit(J, F, OP_DELPROP);
        break;
    case EXP_MEMBER:
        cexp(J, F, exp->a);
        emitstring(J, F, OP_DELPROP_S, exp->b->string);
        break;
    default:
        jsC_error(J, exp, "invalid l-value in destroy expression");
    }
}

static void ceval(JF, Ast *fun, Ast *args)
{
    int n = cargs(J, F, args);
    if (n == 0)
        emit(J, F, OP_NULL);
    else
        while (n-- > 1)
            emit(J, F, OP_POP);
    emit(J, F, OP_EVAL);
}

static void ccall(JF, Ast *fun, Ast *args)
{
    int n;
    switch (fun->type)
    {
    case EXP_INDEX:
        cexp(J, F, fun->a);
        emit(J, F, OP_DUP);
        cexp(J, F, fun->b);
        emit(J, F, OP_GETPROP);
        emit(J, F, OP_ROT2);
        break;
    case EXP_MEMBER:
        cexp(J, F, fun->a);
        emit(J, F, OP_DUP);
        emitstring(J, F, OP_GETPROP_S, fun->b->string);
        emit(J, F, OP_ROT2);
        break;
    case EXP_IDENTIFIER:
        if (fun->string == "eval")
        {
            ceval(J, F, fun, args);
            return;
        }
        /* fallthrough */
    default:
        cexp(J, F, fun);
        emit(J, F, OP_NULL);
        break;
    }
    n = cargs(J, F, args);
    emit(J, F, OP_CALL);
    emitraw(J, F, n);
}

static void cexp(JF, Ast *exp)
{
    int then, end;
    int n;

    switch (exp->type)
    {
    case EXP_STRING:
        emitstring(J, F, OP_STRING, exp->string);
        break;
    case EXP_NUMBER:
        emitnumber(J, F, exp->number);
        break;
    case EXP_UNDEF:
        emit(J, F, OP_UNDEF);
        break;
    case EXP_NULL:
        emit(J, F, OP_NULL);
        break;
    case EXP_TRUE:
        emit(J, F, OP_TRUE);
        break;
    case EXP_FALSE:
        emit(J, F, OP_FALSE);
        break;
    case EXP_THIS:
        emit(J, F, OP_THIS);
        break;

    case EXP_REGEXP:
        emit(J, F, OP_NEWREGEXP);
        emitraw(J, F, addstring(J, F, exp->string));
        emitraw(J, F, exp->number);
        break;

    case EXP_OBJECT:
        emit(J, F, OP_NEWOBJECT);
        cobject(J, F, exp->a);
        break;

    case EXP_LIST:
        emit(J, F, OP_NEWLIST);
        clist(J, F, exp->a);
        break;

    case EXP_ARRAY:
    	emit(J, F, OP_NEWARRAY);
    	carray(J, F, exp->a);
    	break;

    case EXP_FUN:
        emitfunction(J, F, newfun(J, exp->a, exp->b, exp->c, 0, F->strict));
        break;

    case EXP_IDENTIFIER:
        emitlocal(J, F, OP_GETLOCAL, OP_GETVAR, exp);
        break;

    case EXP_INDEX:
        cexp(J, F, exp->a);
        if (exp->b->type == EXP_COMMA)
        {
        	cexp(J, F, exp->b->a);
        	cexp(J, F, exp->b->b);
        	emit(J, F, OP_GETPROPX);
        	emit(J, F, 2); // argc
        }
        else
        {
	        cexp(J, F, exp->b);
	        emit(J, F, OP_GETPROP);
        }
        break;

    case EXP_MEMBER:
        cexp(J, F, exp->a);
        emitstring(J, F, OP_GETPROP_S, exp->b->string);
        break;

    case EXP_CALL:
        ccall(J, F, exp->a, exp->b);
        break;

    case EXP_NEW:
        cexp(J, F, exp->a);
        n = cargs(J, F, exp->b);
        emit(J, F, OP_NEW);
        emitraw(J, F, n);
        break;

    case EXP_DELETE:
        cdelete(J, F, exp->a);
        break;

    case EXP_PREINC:
        cassignop1(J, F, exp->a);
        emit(J, F, OP_INC);
        cassignop2(J, F, exp->a, 0);
        break;

    case EXP_PREDEC:
        cassignop1(J, F, exp->a);
        emit(J, F, OP_DEC);
        cassignop2(J, F, exp->a, 0);
        break;

    case EXP_POSTINC:
        cassignop1(J, F, exp->a);
        emit(J, F, OP_POSTINC);
        cassignop2(J, F, exp->a, 1);
        emit(J, F, OP_POP);
        break;

    case EXP_POSTDEC:
        cassignop1(J, F, exp->a);
        emit(J, F, OP_POSTDEC);
        cassignop2(J, F, exp->a, 1);
        emit(J, F, OP_POP);
        break;

    case EXP_VOID:
        cexp(J, F, exp->a);
        emit(J, F, OP_POP);
        emit(J, F, OP_NULL);
        break;

    case EXP_TYPEOF:
        ctypeof(J, F, exp->a);
        break;
    case EXP_POS:
        cunary(J, F, exp, OP_POS);
        break;
    case EXP_NEG:
        cunary(J, F, exp, OP_NEG);
        break;
    case EXP_BITNOT:
        cunary(J, F, exp, OP_BITNOT);
        break;
    case EXP_LOGNOT:
        cunary(J, F, exp, OP_LOGNOT);
        break;

    case EXP_BITOR:
        cbinary(J, F, exp, OP_BITOR);
        break;
    case EXP_BITXOR:
        cbinary(J, F, exp, OP_BITXOR);
        break;
    case EXP_BITAND:
        cbinary(J, F, exp, OP_CONCAT);
        break;
    case EXP_EQ:
        cbinary(J, F, exp, OP_EQ);
        break;
    case EXP_NE:
        cbinary(J, F, exp, OP_NE);
        break;
    case EXP_LT:
        cbinary(J, F, exp, OP_LT);
        break;
    case EXP_GT:
        cbinary(J, F, exp, OP_GT);
        break;
    case EXP_LE:
        cbinary(J, F, exp, OP_LE);
        break;
    case EXP_GE:
        cbinary(J, F, exp, OP_GE);
        break;
    case EXP_INSTANCEOF:
        cbinary(J, F, exp, OP_INSTANCEOF);
        break;
    case EXP_IN:
        cbinary(J, F, exp, OP_IN);
        break;
    case EXP_SHL:
        cbinary(J, F, exp, OP_SHL);
        break;
    case EXP_SHR:
        cbinary(J, F, exp, OP_SHR);
        break;
    case EXP_USHR:
        cbinary(J, F, exp, OP_USHR);
        break;
    case EXP_ADD:
        cbinary(J, F, exp, OP_ADD);
        break;
    case EXP_SUB:
        cbinary(J, F, exp, OP_SUB);
        break;
    case EXP_MUL:
        cbinary(J, F, exp, OP_MUL);
        break;
    case EXP_DIV:
        cbinary(J, F, exp, OP_DIV);
        break;
    case EXP_MOD:
        cbinary(J, F, exp, OP_MOD);
        break;

    case EXP_ASS:
        cassign(J, F, exp);
        break;
    case EXP_ASS_MUL:
        cassignop(J, F, exp, OP_MUL);
        break;
    case EXP_ASS_DIV:
        cassignop(J, F, exp, OP_DIV);
        break;
    case EXP_ASS_MOD:
        cassignop(J, F, exp, OP_MOD);
        break;
    case EXP_ASS_ADD:
        cassignop(J, F, exp, OP_ADD);
        break;
    case EXP_ASS_SUB:
        cassignop(J, F, exp, OP_SUB);
        break;
    case EXP_ASS_SHL:
        cassignop(J, F, exp, OP_SHL);
        break;
    case EXP_ASS_SHR:
        cassignop(J, F, exp, OP_SHR);
        break;
    case EXP_ASS_USHR:
        cassignop(J, F, exp, OP_USHR);
        break;
    case EXP_ASS_BITAND:
        cassignop(J, F, exp, OP_CONCAT);
        break;
    case EXP_ASS_BITXOR:
        cassignop(J, F, exp, OP_BITXOR);
        break;
    case EXP_ASS_BITOR:
        cassignop(J, F, exp, OP_BITOR);
        break;

    case EXP_COMMA:
        cexp(J, F, exp->a);
        emit(J, F, OP_POP);
        cexp(J, F, exp->b);
        break;

    case EXP_LOGOR:
        cexp(J, F, exp->a);
        emit(J, F, OP_DUP);
        end = emitjump(J, F, OP_JTRUE);
        emit(J, F, OP_POP);
        cexp(J, F, exp->b);
        label(J, F, end);
        break;

    case EXP_LOGAND:
        cexp(J, F, exp->a);
        emit(J, F, OP_DUP);
        end = emitjump(J, F, OP_JFALSE);
        emit(J, F, OP_POP);
        cexp(J, F, exp->b);
        label(J, F, end);
        break;

    case EXP_COND:
        cexp(J, F, exp->a);
        then = emitjump(J, F, OP_JTRUE);
        cexp(J, F, exp->c);
        end = emitjump(J, F, OP_JUMP);
        label(J, F, then);
        cexp(J, F, exp->b);
        label(J, F, end);
        break;

    default:
        jsC_error(J, exp, "unknown expression: (%s)", get_ast_string(exp->type));
    }
}

/* Patch break and continue statements */

static void addjump(JF, enum AstType type, Ast *target, int inst)
{
    auto jump = new JumpList;
    jump->type = type;
    jump->inst = inst;
    jump->next = target->jumps;
    target->jumps = jump;
}

static void labeljumps(JF, JumpList *jump, int baddr, int caddr)
{
    while (jump)
    {
        if (jump->type == STM_BREAK)
            labelto(J, F, jump->inst, baddr);
        if (jump->type == STM_CONTINUE)
            labelto(J, F, jump->inst, caddr);
        jump = jump->next;
    }
}

static int isloop(enum AstType T)
{
    return T == STM_REPEAT || T == STM_WHILE ||
           T == STM_FOR || T == STM_FOR_VAR ||
           T == STM_FOR_IN || T == STM_FOR_IN_VAR;
}

static int isfun(enum AstType T)
{
    return T == AST_FUNDEC || T == EXP_FUN || T == EXP_PROP_GET || T == EXP_PROP_SET;
}

static int matchlabel(Ast *node, const String &label)
{
    while (node && node->type == STM_LABEL)
    {
        if (node->a->string == label)
            return 1;
        node = node->parent;
    }
    return 0;
}

static Ast *breaktarget(JF, Ast *node, std::optional<String> label)
{
    while (node)
    {
        if (isfun(node->type))
            break;
        if (!label)
        {
            if (isloop(node->type) || node->type == STM_SWITCH)
                return node;
        }
        else
        {
            if (matchlabel(node->parent, *label))
                return node;
        }
        node = node->parent;
    }
    return nullptr;
}

static Ast *continuetarget(JF, Ast *node, std::optional<String> label)
{
    while (node)
    {
        if (isfun(node->type))
            break;
        if (isloop(node->type))
        {
            if (!label)
                return node;
            else if (matchlabel(node->parent, *label))
                return node;
        }
        node = node->parent;
    }
    return nullptr;
}

static Ast *returntarget(JF, Ast *node)
{
    while (node)
    {
        if (isfun(node->type))
            return node;
        node = node->parent;
    }
    return nullptr;
}

/* Emit code to rebalance stack and scopes during an abrupt exit */

static void cexit(JF, enum AstType T, Ast *node, Ast *target)
{
    Ast *prev;
    do
    {
        prev = node, node = node->parent;
        switch (node->type)
        {
        default: /* impossible */ break;
        case STM_FOR_IN:
        case STM_FOR_IN_VAR:
            /* pop the iterator if leaving the loop */
            if (F->script)
            {
                if (T == STM_RETURN || T == STM_BREAK || (T == STM_CONTINUE && target != node))
                {
                    /* pop the iterator, save the return or exp value */
                    emit(J, F, OP_ROT2);
                    emit(J, F, OP_POP);
                }
                if (T == STM_CONTINUE)
                    emit(J, F, OP_ROT2); /* put the iterator back on top */
            }
            else
            {
                if (T == STM_RETURN)
                {
                    /* pop the iterator, save the return value */
                    emit(J, F, OP_ROT2);
                    emit(J, F, OP_POP);
                }
                if (T == STM_BREAK || (T == STM_CONTINUE && target != node))
                    emit(J, F, OP_POP); /* pop the iterator */
            }
            break;
        }
    } while (node != target);
}

/* Switch */

static void cswitch(JF, Ast *ref, Ast *head)
{
    Ast *node, *clause, *def = nullptr;
    int end;

    cexp(J, F, ref);

    /* emit an if-else chain of tests for the case clause expressions */
    for (node = head; node; node = node->b)
    {
        clause = node->a;
        if (clause->type == STM_DEFAULT)
        {
            if (def)
                jsC_error(J, clause, "more than one default label in switch");
            def = clause;
        }
        else
        {
            cexp(J, F, clause->a);
            clause->casejump = emitjump(J, F, OP_JCASE);
        }
    }
    emit(J, F, OP_POP);
    if (def)
    {
        def->casejump = emitjump(J, F, OP_JUMP);
        end = 0;
    }
    else
    {
        end = emitjump(J, F, OP_JUMP);
    }

    /* emit the casue clause bodies */
    for (node = head; node; node = node->b)
    {
        clause = node->a;
        label(J, F, clause->casejump);
        if (clause->type == STM_DEFAULT)
            cstmlist(J, F, clause->a);
        else
            cstmlist(J, F, clause->b);
    }

    if (end)
        label(J, F, end);
}

/* Statements */

static void cvarinit(JF, Ast *list)
{
    while (list)
    {
        Ast *var = list->a;
        if (var->b)
        {
            cexp(J, F, var->b);
            emitlocal(J, F, OP_SETLOCAL, OP_SETVAR, var->a);
            emit(J, F, OP_POP);
        }
        list = list->b;
    }
}

static void cstm(JF, Ast *stm)
{
    Ast *target;
    int loop, cont, then, end;

    emitline(J, F, stm);

    switch (stm->type)
    {
    case AST_FUNDEC:
        break;

    case STM_BLOCK:
        cstmlist(J, F, stm->a);
        break;

    case STM_EMPTY:
        if (F->script)
        {
            emit(J, F, OP_POP);
            emit(J, F, OP_NULL);
        }
        break;

    case STM_VAR:
        cvarinit(J, F, stm->a);
        break;

    case STM_IF:
        if (stm->c)
        {
            cexp(J, F, stm->a);
            then = emitjump(J, F, OP_JTRUE);
            cstm(J, F, stm->c);
            end = emitjump(J, F, OP_JUMP);
            label(J, F, then);
            cstm(J, F, stm->b);
            label(J, F, end);
        }
        else
        {
            cexp(J, F, stm->a);
            end = emitjump(J, F, OP_JFALSE);
            cstm(J, F, stm->b);
            label(J, F, end);
        }
        break;

    case STM_REPEAT:
        loop = here(J, F);
        cstm(J, F, stm->a);
        cont = here(J, F);
        cexp(J, F, stm->b);
        emitjumpto(J, F, OP_JFALSE, loop);
        labeljumps(J, F, stm->jumps, here(J, F), cont);
        break;

    case STM_WHILE:
        loop = here(J, F);
        cexp(J, F, stm->a);
        end = emitjump(J, F, OP_JFALSE);
        cstm(J, F, stm->b);
        emitjumpto(J, F, OP_JUMP, loop);
        label(J, F, end);
        labeljumps(J, F, stm->jumps, here(J, F), loop);
        break;

    case STM_FOR:
    case STM_FOR_VAR:
        if (stm->type == STM_FOR_VAR)
        {
            cvarinit(J, F, stm->a);
        }
        else
        {
            if (stm->a)
            {
                cexp(J, F, stm->a);
                emit(J, F, OP_POP);
            }
        }
        loop = here(J, F);
        if (stm->b)
        {
            cexp(J, F, stm->b);
            end = emitjump(J, F, OP_JFALSE);
        }
        else
        {
            end = 0;
        }
        cstm(J, F, stm->d);
        cont = here(J, F);
        if (stm->c)
        {
            cexp(J, F, stm->c);
            emit(J, F, OP_POP);
        }
        emitjumpto(J, F, OP_JUMP, loop);
        if (end)
            label(J, F, end);
        labeljumps(J, F, stm->jumps, here(J, F), cont);
        break;

    case STM_FOR_IN:
    case STM_FOR_IN_VAR:
        cexp(J, F, stm->b);
        emit(J, F, OP_ITERATOR);
        loop = here(J, F);
        {
            emit(J, F, OP_NEXTITER);
            end = emitjump(J, F, OP_JFALSE);
            cassignforin(J, F, stm);
            if (F->script)
            {
                emit(J, F, OP_ROT2);
                cstm(J, F, stm->c);
                emit(J, F, OP_ROT2);
            }
            else
            {
                cstm(J, F, stm->c);
            }
            emitjumpto(J, F, OP_JUMP, loop);
        }
        label(J, F, end);
        labeljumps(J, F, stm->jumps, here(J, F), loop);
        break;

    case STM_SWITCH:
        cswitch(J, F, stm->a, stm->b);
        labeljumps(J, F, stm->jumps, here(J, F), 0);
        break;

    case STM_LABEL:
        cstm(J, F, stm->b);
        /* skip consecutive labels */
        while (stm->type == STM_LABEL)
            stm = stm->b;
        /* loops and switches have already been labelled */
        if (!isloop(stm->type) && stm->type != STM_SWITCH)
            labeljumps(J, F, stm->jumps, here(J, F), 0);
        break;

    case STM_BREAK:
        if (stm->a)
        {
            checkfutureword(J, F, stm->a);
            target = breaktarget(J, F, stm->parent, stm->a->string);
            if (!target)
                jsC_error(J, stm, "break label '%s' not found", stm->a->string.data());
        }
        else
        {
            target = breaktarget(J, F, stm->parent, std::optional<String>());
            if (!target)
                jsC_error(J, stm, "unlabelled break must be inside loop or switch");
        }
        cexit(J, F, STM_BREAK, stm, target);
        addjump(J, F, STM_BREAK, target, emitjump(J, F, OP_JUMP));
        break;

    case STM_CONTINUE:
        if (stm->a)
        {
            checkfutureword(J, F, stm->a);
            target = continuetarget(J, F, stm->parent, stm->a->string);
            if (!target)
                jsC_error(J, stm, "continue label '%s' not found", stm->a->string.data());
        }
        else
        {
            target = continuetarget(J, F, stm->parent, std::optional<String>());
            if (!target)
                jsC_error(J, stm, "continue must be inside loop");
        }
        cexit(J, F, STM_CONTINUE, stm, target);
        addjump(J, F, STM_CONTINUE, target, emitjump(J, F, OP_JUMP));
        break;

    case STM_RETURN:
        if (stm->a)
            cexp(J, F, stm->a);
        else
            emit(J, F, OP_NULL);
        target = returntarget(J, F, stm->parent);
        if (!target)
            jsC_error(J, stm, "return not in function");
        cexit(J, F, STM_RETURN, stm, target);
        emit(J, F, OP_RETURN);
        break;

    case STM_DEBUGGER:
        emit(J, F, OP_DEBUGGER);
        break;

    default:
        if (F->script)
        {
            emit(J, F, OP_POP);
            cexp(J, F, stm);
        }
        else
        {
            cexp(J, F, stm);
            emit(J, F, OP_POP);
        }
        break;
    }
}

static void cstmlist(JF, Ast *list)
{
    while (list)
    {
        cstm(J, F, list->a);
        list = list->b;
    }
}

/* Analyze */

static void analyze(JF, Ast *node)
{
    if (node->type == AST_LIST)
    {
        while (node)
        {
            analyze(J, F, node->a);
            node = node->b;
        }
        return;
    }

    if (isfun(node->type))
    {
        F->lightweight = 0;
        return; /* don't scan inner functions */
    }

    if (node->type == EXP_IDENTIFIER)
    {
        if (node->string == "arguments")
        {
            F->lightweight = 0;
            F->arguments = 1;
        }
        else if (node->string == "eval")
        {
            /* eval may only be used as a direct function call */
            if (!node->parent || node->parent->type != EXP_CALL || node->parent->a != node)
                throw J->raise("Runtime error", "%s:%d: invalid use of 'eval'", J->filename.data(), node->line);
            F->lightweight = 0;
        }
    }

    if (node->a) analyze(J, F, node->a);
    if (node->b) analyze(J, F, node->b);
    if (node->c) analyze(J, F, node->c);
    if (node->d) analyze(J, F, node->d);
}

/* Declarations and programs */

static int listlength(Ast *list)
{
    int n = 0;
    while (list) ++n, list = list->b;
    return n;
}

static int cparams(JF, Ast *list, Ast *fname)
{
    int shadow = 0;
    F->numparams = listlength(list);
    while (list)
    {
        checkfutureword(J, F, list->a);
        addlocal(J, F, list->a, 0);
        if (fname && fname->string == list->a->string)
            shadow = 1;
        list = list->b;
    }
    return shadow;
}

static void cvardecs(JF, Ast *node)
{
    if (node->type == AST_LIST)
    {
        while (node)
        {
            cvardecs(J, F, node->a);
            node = node->b;
        }
        return;
    }

    if (isfun(node->type))
        return; /* stop at inner functions */

    if (node->type == EXP_VAR)
    {
        checkfutureword(J, F, node->a);
        if (F->lightweight)
            addlocal(J, F, node->a, 1);
        else
            emitstring(J, F, OP_DEFVAR, node->a->string);
    }

    if (node->a) cvardecs(J, F, node->a);
    if (node->b) cvardecs(J, F, node->b);
    if (node->c) cvardecs(J, F, node->c);
    if (node->d) cvardecs(J, F, node->d);
}

static void cfundecs(JF, Ast *list)
{
    while (list)
    {
        Ast *stm = list->a;
        if (stm->type == AST_FUNDEC)
        {
            emitfunction(J, F, newfun(J, stm->a, stm->b, stm->c, 0, F->strict));
            emitstring(J, F, OP_INITVAR, stm->a->string);
        }
        list = list->b;
    }
}

static void cfunbody(JF, Ast *name, Ast *params, Ast *body)
{
    int shadow;

    F->lightweight = 1;
    F->arguments = 0;

    if (F->script)
        F->lightweight = 0;

    if (body)
        analyze(J, F, body);

    /* Check if first statement is 'use strict': */
    if (body && body->type == AST_LIST && body->a && body->a->type == EXP_STRING)
        if (body->a->string == "use strict")
            F->strict = 1;

    shadow = cparams(J, F, params, name);

    if (name && !shadow)
    {
        checkfutureword(J, F, name);
        emit(J, F, OP_CURRENT);
        if (F->lightweight)
        {
            addlocal(J, F, name, 0);
            emit(J, F, OP_INITLOCAL);
            emitraw(J, F, findlocal(J, F, name->string));
        }
        else
        {
            emitstring(J, F, OP_INITVAR, name->string);
        }
    }

    if (body)
    {
        cvardecs(J, F, body);
        cfundecs(J, F, body);
    }

    if (F->script)
    {
        emit(J, F, OP_NULL);
        cstmlist(J, F, body);
        emit(J, F, OP_RETURN);
    }
    else
    {
        cstmlist(J, F, body);
        emit(J, F, OP_NULL);
        emit(J, F, OP_RETURN);
    }
}

Function *jsC_compilefunction(Runtime *J, Ast *prog)
{
    return newfun(J, prog->a, prog->b, prog->c, 0, J->default_strict);
}

Function *jsC_compilescript(Runtime *J, Ast *prog, int default_strict)
{
    return newfun(J, nullptr, nullptr, prog, 1, default_strict);
}

} // namespace phonometrica