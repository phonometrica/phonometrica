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

#include <cassert>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/third_party/utf8/utf8.h>

namespace phonometrica {


static const char *astname[] = {
#include "astnames.inc"

        nullptr
};

static const char *opname[] = {
#include "opnames.inc"

        nullptr
};

static int minify = 0;

const char *get_ast_string(enum AstType type)
{
    if (type < nelem(astname)-1)
    return astname[type];
    return "<unknown>";
}

const char *jsC_opcodestring(enum js_OpCode opcode)
{
    if (opcode < nelem(opname)-1)
    return opname[opcode];
    return "<unknown>";
}

static int prec(enum AstType type)
{
    switch (type)
    {
    case AST_IDENTIFIER:
    case EXP_IDENTIFIER:
    case EXP_NUMBER:
    case EXP_STRING:
    case EXP_REGEXP:
    case EXP_UNDEF:
    case EXP_NULL:
    case EXP_TRUE:
    case EXP_FALSE:
    case EXP_THIS:
    case EXP_ARRAY:
    case EXP_OBJECT:
        return 170;

    case EXP_FUN:
    case EXP_INDEX:
    case EXP_MEMBER:
    case EXP_CALL:
    case EXP_NEW:
        return 160;

    case EXP_POSTINC:
    case EXP_POSTDEC:
        return 150;

    case EXP_DELETE:
    case EXP_VOID:
    case EXP_TYPEOF:
    case EXP_PREINC:
    case EXP_PREDEC:
    case EXP_POS:
    case EXP_NEG:
    case EXP_BITNOT:
    case EXP_LOGNOT:
        return 140;

    case EXP_MOD:
    case EXP_DIV:
    case EXP_MUL:
        return 130;

    case EXP_SUB:
    case EXP_ADD:
        return 120;

    case EXP_USHR:
    case EXP_SHR:
    case EXP_SHL:
        return 110;

    case EXP_IN:
    case EXP_INSTANCEOF:
    case EXP_GE:
    case EXP_LE:
    case EXP_GT:
    case EXP_LT:
        return 100;

    case EXP_NE:
    case EXP_EQ:
        return 90;

    case EXP_BITAND:
        return 80;
    case EXP_BITXOR:
        return 70;
    case EXP_BITOR:
        return 60;
    case EXP_LOGAND:
        return 50;
    case EXP_LOGOR:
        return 40;

    case EXP_COND:
        return 30;

    case EXP_ASS:
    case EXP_ASS_MUL:
    case EXP_ASS_DIV:
    case EXP_ASS_MOD:
    case EXP_ASS_ADD:
    case EXP_ASS_SUB:
    case EXP_ASS_SHL:
    case EXP_ASS_SHR:
    case EXP_ASS_USHR:
    case EXP_ASS_BITAND:
    case EXP_ASS_BITXOR:
    case EXP_ASS_BITOR:
        return 20;

#define COMMA 15

    case EXP_COMMA:
        return 10;

    default:
        return 0;
    }
}

static void pc(int c)
{
    putchar(c);
}

static void ps(const char *s)
{
    fputs(s, stdout);
}

static void in(int d)
{
    if (minify < 1)
        while (d-- > 0)
            putchar('\t');
}

static void nl(void)
{
    if (minify < 2)
        putchar('\n');
}

static void sp(void)
{
    if (minify < 1)
        putchar(' ');
}

static void comma(void)
{
    putchar(',');
    sp();
}

/* Pretty-printed Javascript syntax */

static void pstmlist(int d, Ast *list);

static void pexpi(int d, int i, Ast *exp);

static void pstm(int d, Ast *stm);

static void slist(int d, Ast *list);

static void sblock(int d, Ast *list);

static void pargs(int d, Ast *list)
{
    while (list)
    {
        assert(list->type == AST_LIST);
        pexpi(d, COMMA, list->a);
        list = list->b;
        if (list)
            comma();
    }
}

static void parray(int d, Ast *list)
{
    pc('[');
    while (list)
    {
        assert(list->type == AST_LIST);
        pexpi(d, COMMA, list->a);
        list = list->b;
        if (list)
            comma();
    }
    pc(']');
}

static void pobject(int d, Ast *list)
{
    pc('{');
    if (list)
    {
        nl();
        in(d + 1);
    }
    while (list)
    {
        Ast *kv = list->a;
        assert(list->type == AST_LIST);
        switch (kv->type)
        {
        default:
            break;
        case EXP_PROP_VAL:
            pexpi(d + 1, COMMA, kv->a);
            pc(':');
            sp();
            pexpi(d + 1, COMMA, kv->b);
            break;
        case EXP_PROP_GET:
            ps("get ");
            pexpi(d + 1, COMMA, kv->a);
            ps("()");
            sp();
            pc('{');
            nl();
            pstmlist(d + 1, kv->c);
            in(d + 1);
            pc('}');
            break;
        case EXP_PROP_SET:
            ps("set ");
            pexpi(d + 1, COMMA, kv->a);
            pc('(');
            pargs(d + 1, kv->b);
            pc(')');
            sp();
            pc('{');
            nl();
            pstmlist(d + 1, kv->c);
            in(d + 1);
            pc('}');
            break;
        }
        list = list->b;
        if (list)
        {
            pc(',');
            nl();
            in(d + 1);
        }
        else
        {
            nl();
            in(d);
        }
    }
    pc('}');
}

static void pstr(const char *s)
{
    static const char *HEX = "0123456789ABCDEF";
    char32_t c;
    pc(minify ? '\'' : '"');
    while (*s)
    {
        c = utf8::unchecked::next(s);
        switch (c)
        {
        case '\'':
            ps("\\'");
            break;
        case '"':
            ps("\\\"");
            break;
        case '\\':
            ps("\\\\");
            break;
        case '\b':
            ps("\\b");
            break;
        case '\f':
            ps("\\f");
            break;
        case '\n':
            ps("\\n");
            break;
        case '\r':
            ps("\\r");
            break;
        case '\t':
            ps("\\t");
            break;
        default:
            if (c < ' ' || c > 127)
            {
                ps("\\u");
                pc(HEX[(c >> 12) & 15]);
                pc(HEX[(c >> 8) & 15]);
                pc(HEX[(c >> 4) & 15]);
                pc(HEX[c & 15]);
            }
            else
            {
                pc(c);
                break;
            }
        }
    }
    pc(minify ? '\'' : '"');
}

static void pregexp(const char *prog, int flags)
{
    pc('/');
    ps(prog);
    pc('/');
    if (flags & PHON_REGEXP_G) pc('g');
    if (flags & PHON_REGEXP_I) pc('i');
    if (flags & PHON_REGEXP_M) pc('m');
}

static void pbin(int d, int p, Ast *exp, const char *op)
{
    pexpi(d, p, exp->a);
    sp();
    ps(op);
    sp();
    pexpi(d, p, exp->b);
}

static void puna(int d, int p, Ast *exp, const char *pre, const char *suf)
{
    ps(pre);
    pexpi(d, p, exp->a);
    ps(suf);
}

static void pexpi(int d, int p, Ast *exp)
{
    int tp, paren;

    if (!exp) return;

    tp = prec(exp->type);
    paren = 0;
    if (tp < p)
    {
        pc('(');
        paren = 1;
    }
    p = tp;

    switch (exp->type)
    {
    case AST_IDENTIFIER:
        ps(exp->string.data());
        break;
    case EXP_IDENTIFIER:
        ps(exp->string.data());
        break;
    case EXP_NUMBER:
        printf("%.9g", exp->number);
        break;
    case EXP_STRING:
        pstr(exp->string.data());
        break;
    case EXP_REGEXP:
        pregexp(exp->string.data(), exp->number);
        break;

    case EXP_UNDEF:
        break;
    case EXP_NULL:
        ps("null");
        break;
    case EXP_TRUE:
        ps("true");
        break;
    case EXP_FALSE:
        ps("false");
        break;
    case EXP_THIS:
        ps("this");
        break;

    case EXP_OBJECT:
        pobject(d, exp->a);
        break;
    case EXP_ARRAY:
        parray(d, exp->a);
        break;

    case EXP_DELETE:
        puna(d, p, exp, "destroy ", "");
        break;
    case EXP_VOID:
        puna(d, p, exp, "void ", "");
        break;
    case EXP_TYPEOF:
        puna(d, p, exp, "typeof ", "");
        break;
    case EXP_PREINC:
        puna(d, p, exp, "++", "");
        break;
    case EXP_PREDEC:
        puna(d, p, exp, "--", "");
        break;
    case EXP_POSTINC:
        puna(d, p, exp, "", "++");
        break;
    case EXP_POSTDEC:
        puna(d, p, exp, "", "--");
        break;
    case EXP_POS:
        puna(d, p, exp, "+", "");
        break;
    case EXP_NEG:
        puna(d, p, exp, "-", "");
        break;
    case EXP_BITNOT:
        puna(d, p, exp, "~", "");
        break;
    case EXP_LOGNOT:
        puna(d, p, exp, "!", "");
        break;

    case EXP_LOGOR:
        pbin(d, p, exp, "||");
        break;
    case EXP_LOGAND:
        pbin(d, p, exp, "&&");
        break;
    case EXP_BITOR:
        pbin(d, p, exp, "|");
        break;
    case EXP_BITXOR:
        pbin(d, p, exp, "^");
        break;
    case EXP_BITAND:
        pbin(d, p, exp, "&");
        break;
    case EXP_EQ:
        pbin(d, p, exp, "==");
        break;
    case EXP_NE:
        pbin(d, p, exp, "!=");
        break;
    case EXP_LT:
        pbin(d, p, exp, "<");
        break;
    case EXP_GT:
        pbin(d, p, exp, ">");
        break;
    case EXP_LE:
        pbin(d, p, exp, "<=");
        break;
    case EXP_GE:
        pbin(d, p, exp, ">=");
        break;
    case EXP_IN:
        pbin(d, p, exp, "in");
        break;
    case EXP_SHL:
        pbin(d, p, exp, "<<");
        break;
    case EXP_SHR:
        pbin(d, p, exp, ">>");
        break;
    case EXP_USHR:
        pbin(d, p, exp, ">>>");
        break;
    case EXP_ADD:
        pbin(d, p, exp, "+");
        break;
    case EXP_SUB:
        pbin(d, p, exp, "-");
        break;
    case EXP_MUL:
        pbin(d, p, exp, "*");
        break;
    case EXP_DIV:
        pbin(d, p, exp, "/");
        break;
    case EXP_MOD:
        pbin(d, p, exp, "%");
        break;
    case EXP_ASS:
        pbin(d, p, exp, "=");
        break;
    case EXP_ASS_MUL:
        pbin(d, p, exp, "*=");
        break;
    case EXP_ASS_DIV:
        pbin(d, p, exp, "/=");
        break;
    case EXP_ASS_MOD:
        pbin(d, p, exp, "%=");
        break;
    case EXP_ASS_ADD:
        pbin(d, p, exp, "+=");
        break;
    case EXP_ASS_SUB:
        pbin(d, p, exp, "-=");
        break;
    case EXP_ASS_SHL:
        pbin(d, p, exp, "<<=");
        break;
    case EXP_ASS_SHR:
        pbin(d, p, exp, ">>=");
        break;
    case EXP_ASS_USHR:
        pbin(d, p, exp, ">>>=");
        break;
    case EXP_ASS_BITAND:
        pbin(d, p, exp, "&=");
        break;
    case EXP_ASS_BITXOR:
        pbin(d, p, exp, "^=");
        break;
    case EXP_ASS_BITOR:
        pbin(d, p, exp, "|=");
        break;

    case EXP_INSTANCEOF:
        pexpi(d, p, exp->a);
        ps(" instanceof ");
        pexpi(d, p, exp->b);
        break;

    case EXP_COMMA:
        pexpi(d, p, exp->a);
        pc(',');
        sp();
        pexpi(d, p, exp->b);
        break;

    case EXP_COND:
        pexpi(d, p, exp->a);
        sp();
        pc('?');
        sp();
        pexpi(d, p, exp->b);
        sp();
        pc(':');
        sp();
        pexpi(d, p, exp->c);
        break;

    case EXP_INDEX:
        pexpi(d, p, exp->a);
        pc('[');
        pexpi(d, 0, exp->b);
        pc(']');
        break;

    case EXP_MEMBER:
        pexpi(d, p, exp->a);
        pc('.');
        pexpi(d, 0, exp->b);
        break;

    case EXP_CALL:
        pexpi(d, p, exp->a);
        pc('(');
        pargs(d, exp->b);
        pc(')');
        break;

    case EXP_NEW:
        ps("new ");
        pexpi(d, p, exp->a);
        pc('(');
        pargs(d, exp->b);
        pc(')');
        break;

    case EXP_FUN:
        if (p == 0) pc('(');
        ps("function ");
        pexpi(d, 0, exp->a);
        pc('(');
        pargs(d, exp->b);
        pc(')');
        sp();
        pc('{');
        nl();
        pstmlist(d, exp->c);
        in(d);
        pc('}');
        if (p == 0) pc(')');
        break;

    default:
        ps("<UNKNOWN>");
        break;
    }

    if (paren) pc(')');
}

static void pexp(int d, Ast *exp)
{
    pexpi(d, 0, exp);
}

static void pvar(int d, Ast *var)
{
    assert(var->type == EXP_VAR);
    pexp(d, var->a);
    if (var->b)
    {
        sp();
        pc('=');
        sp();
        pexp(d, var->b);
    }
}

static void pvarlist(int d, Ast *list)
{
    while (list)
    {
        assert(list->type == AST_LIST);
        pvar(d, list->a);
        list = list->b;
        if (list)
            comma();
    }
}

static void pblock(int d, Ast *block)
{
    assert(block->type == STM_BLOCK);
    pc('{');
    nl();
    pstmlist(d, block->a);
    in(d);
    pc('}');
}

static void pstmh(int d, Ast *stm)
{
    if (stm->type == STM_BLOCK)
    {
        sp();
        pblock(d, stm);
    }
    else
    {
        nl();
        pstm(d + 1, stm);
    }
}

static void pcaselist(int d, Ast *list)
{
    while (list)
    {
        Ast *stm = list->a;
        if (stm->type == STM_CASE)
        {
            in(d);
            ps("case ");
            pexp(d, stm->a);
            pc(':');
            nl();
            pstmlist(d, stm->b);
        }
        if (stm->type == STM_DEFAULT)
        {
            in(d);
            ps("default:");
            nl();
            pstmlist(d, stm->a);
        }
        list = list->b;
    }
}

static void pstm(int d, Ast *stm)
{
    if (stm->type == STM_BLOCK)
    {
        pblock(d, stm);
        return;
    }

    in(d);

    switch (stm->type)
    {
    case AST_FUNDEC:
        ps("function ");
        pexp(d, stm->a);
        pc('(');
        pargs(d, stm->b);
        pc(')');
        sp();
        pc('{');
        nl();
        pstmlist(d, stm->c);
        in(d);
        pc('}');
        break;

    case STM_EMPTY:
        pc(';');
        break;

    case STM_VAR:
        ps("var ");
        pvarlist(d, stm->a);
        pc(';');
        break;

    case STM_IF:
        ps("if");
        sp();
        pc('(');
        pexp(d, stm->a);
        pc(')');
        pstmh(d, stm->b);
        if (stm->c)
        {
            nl();
            in(d);
            ps("else");
            pstmh(d, stm->c);
        }
        break;

    case STM_REPEAT:
        ps("repeat");
        pstmh(d, stm->a);
        nl();
        in(d);
        ps("until");
        sp();
        pc('(');
        pexp(d, stm->b);
        pc(')');
        pc(';');
        break;

    case STM_WHILE:
        ps("while");
        sp();
        pc('(');
        pexp(d, stm->a);
        pc(')');
        pstmh(d, stm->b);
        break;

    case STM_FOR:
        ps("for");
        sp();
        pc('(');
        pexp(d, stm->a);
        pc(';');
        sp();
        pexp(d, stm->b);
        pc(';');
        sp();
        pexp(d, stm->c);
        pc(')');
        pstmh(d, stm->d);
        break;
    case STM_FOR_VAR:
        ps("for");
        sp();
        ps("(var ");
        pvarlist(d, stm->a);
        pc(';');
        sp();
        pexp(d, stm->b);
        pc(';');
        sp();
        pexp(d, stm->c);
        pc(')');
        pstmh(d, stm->d);
        break;
    case STM_FOR_IN:
        ps("for");
        sp();
        pc('(');
        pexp(d, stm->a);
        ps(" in ");
        pexp(d, stm->b);
        pc(')');
        pstmh(d, stm->c);
        break;
    case STM_FOR_IN_VAR:
        ps("for");
        sp();
        ps("(var ");
        pvarlist(d, stm->a);
        ps(" in ");
        pexp(d, stm->b);
        pc(')');
        pstmh(d, stm->c);
        break;

    case STM_CONTINUE:
        ps("continue");
        if (stm->a)
        {
            pc(' ');
            pexp(d, stm->a);
        }
        pc(';');
        break;

    case STM_BREAK:
        ps("break");
        if (stm->a)
        {
            pc(' ');
            pexp(d, stm->a);
        }
        pc(';');
        break;

    case STM_RETURN:
        ps("return");
        if (stm->a)
        {
            pc(' ');
            pexp(d, stm->a);
        }
        pc(';');
        break;

    case STM_SWITCH:
        ps("switch");
        sp();
        pc('(');
        pexp(d, stm->a);
        pc(')');
        sp();
        pc('{');
        nl();
        pcaselist(d, stm->b);
        in(d);
        pc('}');
        break;

    case STM_THROW:
        ps("throw ");
        pexp(d, stm->a);
        pc(';');
        break;

    case STM_TRY:
        ps("try");
        if (minify && stm->a->type != STM_BLOCK)
            pc(' ');
        pstmh(d, stm->a);
        if (stm->b && stm->c)
        {
            nl();
            in(d);
            ps("catch");
            sp();
            pc('(');
            pexp(d, stm->b);
            pc(')');
            pstmh(d, stm->c);
        }
        if (stm->d)
        {
            nl();
            in(d);
            ps("finally");
            pstmh(d, stm->d);
        }
        break;

    case STM_LABEL:
        pexp(d, stm->a);
        pc(':');
        sp();
        pstm(d, stm->b);
        break;

    case STM_DEBUGGER:
        ps("debugger");
        pc(';');
        break;

    default:
        pexp(d, stm);
        pc(';');
    }
}

static void pstmlist(int d, Ast *list)
{
    while (list)
    {
        assert(list->type == AST_LIST);
        pstm(d + 1, list->a);
        nl();
        list = list->b;
    }
}

void dump_syntax(Environment *J, Ast *prog, int dominify)
{
    minify = dominify;
    if (prog->type == AST_LIST)
        pstmlist(-1, prog);
    else
    {
        pstm(0, prog);
        nl();
    }
    if (minify > 1)
        putchar('\n');
}

/* S-expression list representation */

static void snode(int d, Ast *node)
{
    void (*afun)(int, Ast *) = snode;
    void (*bfun)(int, Ast *) = snode;
    void (*cfun)(int, Ast *) = snode;
    void (*dfun)(int, Ast *) = snode;

    if (!node)
    {
        return;
    }

    if (node->type == AST_LIST)
    {
        slist(d, node);
        return;
    }

    pc('(');
    ps(astname[node->type]);
    switch (node->type)
    {
    default:
        break;
    case AST_IDENTIFIER:
        pc(' ');
        ps(node->string.data());
        break;
    case EXP_IDENTIFIER:
        pc(' ');
        ps(node->string.data());
        break;
    case EXP_STRING:
        pc(' ');
        pstr(node->string.data());
        break;
    case EXP_REGEXP:
        pc(' ');
        pregexp(node->string.data(), node->number);
        break;
    case EXP_NUMBER:
        printf(" %.9g", node->number);
        break;
    case STM_BLOCK:
        afun = sblock;
        break;
    case AST_FUNDEC:
    case EXP_FUN:
        cfun = sblock;
        break;
    case EXP_PROP_GET:
        cfun = sblock;
        break;
    case EXP_PROP_SET:
        cfun = sblock;
        break;
    case STM_SWITCH:
        bfun = sblock;
        break;
    case STM_CASE:
        bfun = sblock;
        break;
    case STM_DEFAULT:
        afun = sblock;
        break;
    }
    if (node->a)
    {
        pc(' ');
        afun(d, node->a);
    }
    if (node->b)
    {
        pc(' ');
        bfun(d, node->b);
    }
    if (node->c)
    {
        pc(' ');
        cfun(d, node->c);
    }
    if (node->d)
    {
        pc(' ');
        dfun(d, node->d);
    }
    pc(')');
}

static void slist(int d, Ast *list)
{
    pc('[');
    while (list)
    {
        assert(list->type == AST_LIST);
        snode(d, list->a);
        list = list->b;
        if (list)
            pc(' ');
    }
    pc(']');
}

static void sblock(int d, Ast *list)
{
    ps("[\n");
    in(d + 1);
    while (list)
    {
        assert(list->type == AST_LIST);
        snode(d + 1, list->a);
        list = list->b;
        if (list)
        {
            nl();
            in(d + 1);
        }
    }
    nl();
    in(d);
    pc(']');
}

void dump_list(Environment *J, Ast *prog)
{
    minify = 0;
    if (prog->type == AST_LIST)
        sblock(0, prog);
    else
        snode(0, prog);
    nl();
}

/* Compiled code */

void jsC_dumpfunction(Environment *J, Function *F)
{
    auto p = F->code.data();
    instruction_t *end = F->code.data() + F->code.size();

    minify = 0;

    // TODO: replace printf with utils::printf
    printf("%s(%d)\n", F->name.data(), F->numparams);
    if (F->lightweight) printf("\tlightweight\n");
    if (F->arguments) printf("\targuments\n");
    printf("\tsource %s:%d\n", F->filename.data(), F->line);
    for (size_t i = 0; i < F->funtab.size(); ++i)
        printf("\tfunction %lu %s\n", i, F->funtab[i]->name.data());

    for (size_t i = 0; i < F->vartab.size(); ++i)
        printf("\tlocal %lu %s\n", i + 1, F->vartab[i].data());

    printf("{\n");
    while (p < end)
    {
        int c = *p++;

        printf("% 5d: ", (int) (p - F->code.data()) - 1);
        ps(opname[c]);

        switch (c)
        {
        case OP_NUMBER:
            printf(" %.9g", F->numtab[*p++]);
            break;
        case OP_STRING:
            pc(' ');
            pstr(F->strtab[*p++].data());
            break;
        case OP_NEWREGEXP:
            pc(' ');
            pregexp(F->strtab[p[0]].data(), p[1]);
            p += 2;
            break;

        case OP_INITVAR:
        case OP_DEFVAR:
        case OP_GETVAR:
        case OP_HASVAR:
        case OP_SETVAR:
        case OP_DELVAR:
        case OP_GETPROP_S:
        case OP_SETPROP_S:
        case OP_DELPROP_S:
            pc(' ');
            ps(F->strtab[*p++].data());
            break;

        case OP_LINE:
        case OP_CLOSURE:
        case OP_INITLOCAL:
        case OP_GETLOCAL:
        case OP_SETLOCAL:
        case OP_DELLOCAL:
        case OP_NUMBER_POS:
        case OP_NUMBER_NEG:
        case OP_CALL:
        case OP_NEW:
        case OP_JUMP:
        case OP_JTRUE:
        case OP_JFALSE:
        case OP_JCASE:
            printf(" %d", *p++);
            break;
        }

        nl();
    }
    printf("}\n");

    for (size_t i = 0; i < F->funtab.size(); ++i)
    {
        if (F->funtab[i] != F)
        {
            printf("function %lu ", i);
            jsC_dumpfunction(J, F->funtab[i]);
        }
    }
}

/* Runtime values */

void dump_variant(Environment *J, const Variant &v)
{
    minify = 0;
    switch (v.type)
    {
    case PHON_TNULL:
        printf("null");
        break;
    case PHON_TBOOLEAN:
        printf(v.as.boolean ? "true" : "false");
        break;
    case PHON_TNUMBER:
        printf("%.9g", v.as.number);
        break;
    case PHON_TSTRING:
        printf("'%s'", v.as.string.data());
        break;
    case PHON_TOBJECT:
        if (v.as.object == J->G)
        {
            printf("[Global]");
            break;
        }
        switch (v.as.object->type)
        {
        case PHON_COBJECT:
            printf("[Object %p]", (void *) v.as.object);
            break;
        case PHON_CLIST:
            printf("[List %p]", (void *) v.as.object);
            break;
        case PHON_CFUNCTION:
            printf("[Function %p, %s, %s:%d]",
                   (void *) v.as.object,
                   v.as.object->as.f.function->name.data(),
                   v.as.object->as.f.function->filename.data(),
                   v.as.object->as.f.function->line);
            break;
        case PHON_CSCRIPT:
            printf("[Script %s]", v.as.object->as.f.function->filename.data());
            break;
        case PHON_CCFUNCTION:
            printf("[CFunction %s]", v.as.object->as.c.name.data());
            break;
        case PHON_CBOOLEAN:
            printf("[Boolean %d]", v.as.object->as.boolean);
            break;
        case PHON_CNUMBER:
            printf("[Number %g]", v.as.object->as.number);
            break;
        case PHON_CSTRING:
            printf("[String'%s']", v.as.object->as.string.data());
            break;
        case PHON_CERROR:
            printf("[Error]");
            break;
        case PHON_CITERATOR:
            printf("[Iterator %p]", (void *) v.as.object);
            break;
        case PHON_CUSERDATA: // TODO: print userdata
            printf("[Userdata %s %p]", v.as.object->as.user.tag, &v.as.object->as.user.data);
            break;
        default:
            printf("[Object %p]", (void *) v.as.object);
            break;
        }
        break;
    }
}

void dump_object(Environment *J, Object *obj)
{
    minify = 0;
    utils::printf("{\n");
    for (auto &f : obj->fields)
    {
        utils::printf("\t%: ", f.second.name);
        dump_variant(J, f.second.value);
        utils::printf(",\n");
    }
    utils::printf("}\n");
}

void dump_strings(Environment *J)
{
    utils::printf("interned strings {\n");
    for (auto &s : J->strings)
    {
        utils::printf("'\t%'\n", s);
    }
    utils::printf("}\n");
}

} // namespace phonometrica