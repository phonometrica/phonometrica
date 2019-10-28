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

#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/lex.hpp>
#include <phon/runtime/parse.hpp>

#define LIST(h)        jsP_newnode(J, AST_LIST, h, 0, 0, 0)

#define EXP0(x)        jsP_newnode(J, EXP_ ## x, 0, 0, 0, 0)
#define EXP1(x, a)    jsP_newnode(J, EXP_ ## x, a, 0, 0, 0)
#define EXP2(x, a, b)    jsP_newnode(J, EXP_ ## x, a, b, 0, 0)
#define EXP3(x, a, b, c)    jsP_newnode(J, EXP_ ## x, a, b, c, 0)

#define STM0(x)        jsP_newnode(J, STM_ ## x, 0, 0, 0, 0)
#define STM1(x, a)    jsP_newnode(J, STM_ ## x, a, 0, 0, 0)
#define STM2(x, a, b)    jsP_newnode(J, STM_ ## x, a, b, 0, 0)
#define STM3(x, a, b, c)    jsP_newnode(J, STM_ ## x, a, b, c, 0)
#define STM4(x, a, b, c, d)    jsP_newnode(J, STM_ ## x, a, b, c, d)

namespace phonometrica {

static Ast *expression(Runtime *J, int notin);

static Ast *assignment(Runtime *J, int notin);

static Ast *memberexp(Runtime *J);

static Ast *statement(Runtime *J);

static Ast *funbody(Runtime *J);

PHON_NORETURN static void parse_error(Runtime *J, const char *fmt, ...) PHON_PRINTFLIKE(2, 3);

#define INCREC() if (++J->astdepth > PHON_ASTLIMIT) parse_error(J, "[Syntax error] too much recursion")
#define DECREC() --J->astdepth
#define SAVEREC() int SAVE=J->astdepth
#define POPREC() J->astdepth=SAVE

static void parse_error(Runtime *J, const char *fmt, ...)
{
    va_list ap;
    char buf[512];
    char msgbuf[256];

    va_start(ap, fmt);
    vsnprintf(msgbuf, 256, fmt, ap);
    va_end(ap);

    snprintf(buf, 256, "%s:%d: ", J->filename.data(), J->lexline);
    strcat(buf, msgbuf);
    throw std::runtime_error(buf);
}

static void jsP_warning(Runtime *J, const char *fmt, ...)
{
    va_list ap;
    char buf[512];
    char msg[256];

    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);

    snprintf(buf, sizeof buf, "%s:%d: warning: %s", J->filename.data(), J->lexline, msg);
    J->report(buf);
}

static Ast *jsP_newnode(Runtime *J, enum AstType type, Ast *a, Ast *b, Ast *c, Ast *d)
{
    auto node = new Ast;

    node->type = type;
    node->line = J->astline;
    node->a = a;
    node->b = b;
    node->c = c;
    node->d = d;
    node->number = 0;
    node->jumps = nullptr;
    node->casejump = 0;

    node->parent = nullptr;
    if (a) a->parent = node;
    if (b) b->parent = node;
    if (c) c->parent = node;
    if (d) d->parent = node;

    node->gcnext = J->gcast;
    J->gcast = node;

    return node;
}

static Ast *jsP_list(Ast *head)
{
    /* set parent pointers in list nodes */
    Ast *prev = head, *node = head->b;

    while (node)
    {
        node->parent = prev;
        prev = node;
        node = node->b;
    }
    return head;
}

static Ast *jsP_newstrnode(Runtime *J, enum AstType type, const String &s)
{
    Ast *node = jsP_newnode(J, type, 0, 0, 0, 0);
    node->string = s;
    return node;
}

static Ast *jsP_newnumnode(Runtime *J, enum AstType type, double n)
{
    Ast *node = jsP_newnode(J, type, 0, 0, 0, 0);
    node->number = n;
    return node;
}

static void jsP_freejumps(Runtime *J, JumpList *node)
{
    while (node)
    {
        JumpList *next = node->next;
        delete node;
        node = next;
    }
}

void free_parse(Runtime *J)
{
    Ast *node = J->gcast;
    while (node)
    {
        Ast *next = node->gcnext;
        jsP_freejumps(J, node->jumps);
        delete node;
        node = next;
    }
    J->gcast = nullptr;
}

/* Lookahead */

static void jsP_next(Runtime *J)
{
    J->lookahead = lex(J);
    J->astline = J->lexline;
}

#define jsP_accept(J, x) (J->lookahead == x ? (jsP_next(J), 1) : 0)

#define jsP_expect(J, x) if (!jsP_accept(J, x)) parse_error(J, "[Syntax error] unexpected token: %s (expected %s)", get_token_string(J->lookahead), get_token_string(x))

static void semicolon(Runtime *J)
{
    if (J->lookahead == ';')
    {
        jsP_next(J);
        return;
    }
    if (J->newline || J->lookahead == '}' || J->lookahead == 0)
        return;
    parse_error(J, "[Syntax error] unexpected token: %s (expected ';')", get_token_string(J->lookahead));
}

/* Literals */

static Ast *identifier(Runtime *J)
{
    Ast *a;
    if (J->lookahead == TK_IDENTIFIER)
    {
        a = jsP_newstrnode(J, AST_IDENTIFIER, J->text);
        jsP_next(J);
        return a;
    }
    parse_error(J, "[Syntax error] unexpected token: %s (expected identifier)", get_token_string(J->lookahead));
}

static Ast *identifieropt(Runtime *J)
{
    if (J->lookahead == TK_IDENTIFIER)
        return identifier(J);
    return nullptr;
}

static Ast *identifiername(Runtime *J)
{
    if (J->lookahead == TK_IDENTIFIER || J->lookahead >= TK_BREAK)
    {
        Ast *a = jsP_newstrnode(J, AST_IDENTIFIER, J->text);
        jsP_next(J);
        return a;
    }
    parse_error(J, "[Syntax error] unexpected token: %s (expected identifier or keyword)", get_token_string(J->lookahead));
}

static Ast *arrayelement(Runtime *J)
{
    if (J->lookahead == ',')
        return EXP0(UNDEF);
    return assignment(J, 0);
}

static Ast *arrayliteral(Runtime *J)
{
    Ast *head, *tail;
    if (J->lookahead == ']')
        return nullptr;
    head = tail = LIST(arrayelement(J));
    while (jsP_accept(J, ','))
    {
        if (J->lookahead != ']')
            tail = tail->b = LIST(arrayelement(J));
    }
    return jsP_list(head);
}

static Ast *propname(Runtime *J)
{
    Ast *name;
    if (J->lookahead == TK_NUMBER)
    {
        name = jsP_newnumnode(J, EXP_NUMBER, J->number);
        jsP_next(J);
    }
    else if (J->lookahead == TK_STRING)
    {
        name = jsP_newstrnode(J, EXP_STRING, J->text);
        jsP_next(J);
    }
    else
    {
        name = identifiername(J);
    }
    return name;
}

static Ast *propassign(Runtime *J)
{
    Ast *name, *value, *arg, *body;

    name = propname(J);

    if (J->lookahead != ':' && name->type == AST_IDENTIFIER)
    {
        if (name->string == "get")
        {
            name = propname(J);
            jsP_expect(J, '(');
            jsP_expect(J, ')');
            body = funbody(J);
            return EXP3(PROP_GET, name, nullptr, body);
        }
        if (name->string == "set")
        {
            name = propname(J);
            jsP_expect(J, '(');
            arg = identifier(J);
            jsP_expect(J, ')');
            body = funbody(J);
            return EXP3(PROP_SET, name, LIST(arg), body);
        }
    }

    jsP_expect(J, ':');
    value = assignment(J, 0);
    return EXP2(PROP_VAL, name, value);
}

static Ast *objectliteral(Runtime *J)
{
    Ast *head, *tail;
    if (J->lookahead == '}')
        return nullptr;
    head = tail = LIST(propassign(J));
    while (jsP_accept(J, ','))
    {
        if (J->lookahead == '}')
            break;
        tail = tail->b = LIST(propassign(J));
    }
    return jsP_list(head);
}

/* Functions */

static Ast *parameters(Runtime *J)
{
    Ast *head, *tail;
    if (J->lookahead == ')')
        return nullptr;
    head = tail = LIST(identifier(J));
    while (jsP_accept(J, ','))
    {
        tail = tail->b = LIST(identifier(J));
    }
    return jsP_list(head);
}

static Ast *fundec(Runtime *J)
{
    Ast *a, *b, *c;
    a = identifier(J);
    jsP_expect(J, '(');
    b = parameters(J);
    jsP_expect(J, ')');
    c = funbody(J);
    return jsP_newnode(J, AST_FUNDEC, a, b, c, 0);
}

static Ast *funstm(Runtime *J)
{
    Ast *a, *b, *c;
    a = identifier(J);
    jsP_expect(J, '(');
    b = parameters(J);
    jsP_expect(J, ')');
    c = funbody(J);
    /* rewrite function statement as "var X = function X() {}" */
    return STM1(VAR, LIST(EXP2(VAR, a, EXP3(FUN, a, b, c))));
}

static Ast *funexp(Runtime *J)
{
    Ast *a, *b, *c;
    a = identifieropt(J);
    jsP_expect(J, '(');
    b = parameters(J);
    jsP_expect(J, ')');
    c = funbody(J);
    return EXP3(FUN, a, b, c);
}

/* Expressions */

static Ast *primary(Runtime *J)
{
    Ast *a;

    if (J->lookahead == TK_IDENTIFIER)
    {
        a = jsP_newstrnode(J, EXP_IDENTIFIER, J->text);
        jsP_next(J);
        return a;
    }
    if (J->lookahead == TK_STRING)
    {
        a = jsP_newstrnode(J, EXP_STRING, J->text);
        jsP_next(J);
        return a;
    }
    if (J->lookahead == TK_REGEXP)
    {
        a = jsP_newstrnode(J, EXP_REGEXP, J->text);
        a->number = J->number;
        jsP_next(J);
        return a;
    }
    if (J->lookahead == TK_NUMBER)
    {
        a = jsP_newnumnode(J, EXP_NUMBER, J->number);
        jsP_next(J);
        return a;
    }

    if (jsP_accept(J, TK_THIS)) return EXP0(THIS);
    if (jsP_accept(J, TK_NULL)) return EXP0(NULL);
    if (jsP_accept(J, TK_TRUE)) return EXP0(TRUE);
    if (jsP_accept(J, TK_FALSE)) return EXP0(FALSE);
    if (jsP_accept(J, '{'))
    {
        a = EXP1(OBJECT, objectliteral(J));
        jsP_expect(J, '}');
        return a;
    }
    if (jsP_accept(J, '['))
    {
        a = EXP1(ARRAY, arrayliteral(J));
        jsP_expect(J, ']');
        return a;
    }
    if (jsP_accept(J, '('))
    {
        a = expression(J, 0);
        jsP_expect(J, ')');
        return a;
    }

    parse_error(J, "[Syntax error] unexpected token in expression: %s", get_token_string(J->lookahead));
}

static Ast *arguments(Runtime *J)
{
    Ast *head, *tail;
    if (J->lookahead == ')')
        return nullptr;
    head = tail = LIST(assignment(J, 0));
    while (jsP_accept(J, ','))
    {
        tail = tail->b = LIST(assignment(J, 0));
    }
    return jsP_list(head);
}

static Ast *newexp(Runtime *J)
{
    Ast *a, *b;

    if (jsP_accept(J, TK_NEW))
    {
        a = memberexp(J);
        if (jsP_accept(J, '('))
        {
            b = arguments(J);
            jsP_expect(J, ')');
            return EXP2(NEW, a, b);
        }
        return EXP1(NEW, a);
    }

    if (jsP_accept(J, TK_FUNCTION))
        return funexp(J);

    return primary(J);
}

static Ast *memberexp(Runtime *J)
{
    Ast *a = newexp(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, '.'))
    {
        a = EXP2(MEMBER, a, identifiername(J));
        goto loop;
    }
    if (jsP_accept(J, '['))
    {
        a = EXP2(INDEX, a, expression(J, 0));
        jsP_expect(J, ']');
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *callexp(Runtime *J)
{
    Ast *a = newexp(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, '.'))
    {
        a = EXP2(MEMBER, a, identifiername(J));
        goto loop;
    }
    if (jsP_accept(J, '['))
    {
        a = EXP2(INDEX, a, expression(J, 0));
        jsP_expect(J, ']');
        goto loop;
    }
    if (jsP_accept(J, '('))
    {
        a = EXP2(CALL, a, arguments(J));
        jsP_expect(J, ')');
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *postfix(Runtime *J)
{
    Ast *a = callexp(J);
    if (!J->newline && jsP_accept(J, TK_INC)) return EXP1(POSTINC, a);
    if (!J->newline && jsP_accept(J, TK_DEC)) return EXP1(POSTDEC, a);
    return a;
}

static Ast *unary(Runtime *J)
{
    Ast *a;
    INCREC();
    if (jsP_accept(J, TK_DELETE)) a = EXP1(DELETE, unary(J));
    else if (jsP_accept(J, TK_VOID)) a = EXP1(VOID, unary(J));
    else if (jsP_accept(J, TK_TYPEOF)) a = EXP1(TYPEOF, unary(J));
    else if (jsP_accept(J, TK_INC)) a = EXP1(PREINC, unary(J));
    else if (jsP_accept(J, TK_DEC)) a = EXP1(PREDEC, unary(J));
    else if (jsP_accept(J, '+')) a = EXP1(POS, unary(J));
    else if (jsP_accept(J, '-')) a = EXP1(NEG, unary(J));
    else if (jsP_accept(J, '~')) a = EXP1(BITNOT, unary(J));
    else if (jsP_accept(J, TK_NOT)) a = EXP1(LOGNOT, unary(J));
    else a = postfix(J);
    DECREC();
    return a;
}

static Ast *multiplicative(Runtime *J)
{
    Ast *a = unary(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, '*'))
    {
        a = EXP2(MUL, a, unary(J));
        goto loop;
    }
    if (jsP_accept(J, '/'))
    {
        a = EXP2(DIV, a, unary(J));
        goto loop;
    }
    if (jsP_accept(J, '%'))
    {
        a = EXP2(MOD, a, unary(J));
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *additive(Runtime *J)
{
    Ast *a = multiplicative(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, '+'))
    {
        a = EXP2(ADD, a, multiplicative(J));
        goto loop;
    }
    if (jsP_accept(J, '-'))
    {
        a = EXP2(SUB, a, multiplicative(J));
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *shift(Runtime *J)
{
    Ast *a = additive(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, TK_SHL))
    {
        a = EXP2(SHL, a, additive(J));
        goto loop;
    }
    if (jsP_accept(J, TK_SHR))
    {
        a = EXP2(SHR, a, additive(J));
        goto loop;
    }
    if (jsP_accept(J, TK_USHR))
    {
        a = EXP2(USHR, a, additive(J));
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *relational(Runtime *J, int notin)
{
    Ast *a = shift(J);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, '<'))
    {
        a = EXP2(LT, a, shift(J));
        goto loop;
    }
    if (jsP_accept(J, '>'))
    {
        a = EXP2(GT, a, shift(J));
        goto loop;
    }
    if (jsP_accept(J, TK_LE))
    {
        a = EXP2(LE, a, shift(J));
        goto loop;
    }
    if (jsP_accept(J, TK_GE))
    {
        a = EXP2(GE, a, shift(J));
        goto loop;
    }
    if (jsP_accept(J, TK_INSTANCEOF))
    {
        a = EXP2(INSTANCEOF, a, shift(J));
        goto loop;
    }
    if (!notin && jsP_accept(J, TK_IN))
    {
        a = EXP2(IN, a, shift(J));
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *equality(Runtime *J, int notin)
{
    Ast *a = relational(J, notin);
    SAVEREC();
    loop:
    INCREC();
    if (jsP_accept(J, TK_EQ))
    {
        a = EXP2(EQ, a, relational(J, notin));
        goto loop;
    }
    if (jsP_accept(J, TK_NE))
    {
        a = EXP2(NE, a, relational(J, notin));
        goto loop;
    }
    POPREC();
    return a;
}

static Ast *js_bitand(Runtime *J, int notin)
{
    Ast *a = equality(J, notin);
    SAVEREC();
    while (jsP_accept(J, '&'))
    {
        INCREC();
        a = EXP2(BITAND, a, equality(J, notin));
    }
    POPREC();
    return a;
}

static Ast *js_bitxor(Runtime *J, int notin)
{
    Ast *a = js_bitand(J, notin);
    SAVEREC();
    while (jsP_accept(J, '^'))
    {
        INCREC();
        a = EXP2(BITXOR, a, js_bitand(J, notin));
    }
    POPREC();
    return a;
}

static Ast *js_bitor(Runtime *J, int notin)
{
    Ast *a = js_bitxor(J, notin);
    SAVEREC();
    while (jsP_accept(J, '|'))
    {
        INCREC();
        a = EXP2(BITOR, a, js_bitxor(J, notin));
    }
    POPREC();
    return a;
}

static Ast *logand(Runtime *J, int notin)
{
    Ast *a = js_bitor(J, notin);
    if (jsP_accept(J, TK_AND))
    {
        INCREC();
        a = EXP2(LOGAND, a, logand(J, notin));
        DECREC();
    }
    return a;
}

static Ast *logor(Runtime *J, int notin)
{
    Ast *a = logand(J, notin);
    if (jsP_accept(J, TK_OR))
    {
        INCREC();
        a = EXP2(LOGOR, a, logor(J, notin));
        DECREC();
    }
    return a;
}

static Ast *conditional(Runtime *J, int notin)
{
    Ast *a = logor(J, notin);
    if (jsP_accept(J, '?'))
    {
        Ast *b, *c;
        INCREC();
        b = assignment(J, 0);
        jsP_expect(J, ':');
        c = assignment(J, notin);
        DECREC();
        return EXP3(COND, a, b, c);
    }
    return a;
}

static Ast *assignment(Runtime *J, int notin)
{
    Ast *a = conditional(J, notin);
    INCREC();
    if (jsP_accept(J, '=')) a = EXP2(ASS, a, assignment(J, notin));
    else if (jsP_accept(J, TK_MUL_ASS)) a = EXP2(ASS_MUL, a, assignment(J, notin));
    else if (jsP_accept(J, TK_DIV_ASS)) a = EXP2(ASS_DIV, a, assignment(J, notin));
    else if (jsP_accept(J, TK_MOD_ASS)) a = EXP2(ASS_MOD, a, assignment(J, notin));
    else if (jsP_accept(J, TK_ADD_ASS)) a = EXP2(ASS_ADD, a, assignment(J, notin));
    else if (jsP_accept(J, TK_SUB_ASS)) a = EXP2(ASS_SUB, a, assignment(J, notin));
    else if (jsP_accept(J, TK_SHL_ASS)) a = EXP2(ASS_SHL, a, assignment(J, notin));
    else if (jsP_accept(J, TK_SHR_ASS)) a = EXP2(ASS_SHR, a, assignment(J, notin));
    else if (jsP_accept(J, TK_USHR_ASS)) a = EXP2(ASS_USHR, a, assignment(J, notin));
    else if (jsP_accept(J, TK_AND_ASS)) a = EXP2(ASS_BITAND, a, assignment(J, notin));
    else if (jsP_accept(J, TK_XOR_ASS)) a = EXP2(ASS_BITXOR, a, assignment(J, notin));
    else if (jsP_accept(J, TK_OR_ASS)) a = EXP2(ASS_BITOR, a, assignment(J, notin));
    DECREC();
    return a;
}

static Ast *expression(Runtime *J, int notin)
{
    Ast *a = assignment(J, notin);
    SAVEREC();
    while (jsP_accept(J, ','))
    {
        INCREC();
        a = EXP2(COMMA, a, assignment(J, notin));
    }
    POPREC();
    return a;
}

/* Statements */

static Ast *vardec(Runtime *J, int notin)
{
    Ast *a = identifier(J);
    if (jsP_accept(J, '='))
        return EXP2(VAR, a, assignment(J, notin));
    return EXP1(VAR, a);
}

static Ast *vardeclist(Runtime *J, int notin)
{
    Ast *head, *tail;
    head = tail = LIST(vardec(J, notin));
    while (jsP_accept(J, ','))
        tail = tail->b = LIST(vardec(J, notin));
    return jsP_list(head);
}

static
bool is_end_block(int t)
{
    switch (t)
    {
    case TK_END:
    case TK_ELSE:
    case TK_ELSIF:
    case TK_UNTIL:
        return true;
    default:
        return false;
    }
}

static Ast *statementlist(Runtime *J)
{
    Ast *head, *tail;

    if (is_end_block(J->lookahead))
        return nullptr;
    head = tail = LIST(statement(J));
    while (!is_end_block(J->lookahead))
        tail = tail->b = LIST(statement(J));
    return jsP_list(head);
}

static Ast *block(Runtime *J)
{
    Ast *a;
    jsP_expect(J, '{');
    a = statementlist(J);
    jsP_expect(J, '}');
    return STM1(BLOCK, a);
}

static Ast *block_with_end(Runtime *J)
{
    Ast *a;
    a = statementlist(J);
    return STM1(BLOCK, a);
}

static Ast *importstatement(Runtime *J)
{
    // Rewrite "import module" as "var module = require('module')". This corresponds to the following AST:
    // (stm_var [(exp_var (identifier module) (exp_call (exp_identifier require) [(exp_string "module")]))])
    auto name = identifier(J);
    auto require = jsP_newstrnode(J, EXP_IDENTIFIER, "require");
    auto string = jsP_newstrnode(J, EXP_STRING, name->string);
    auto call = EXP2(CALL, require, jsP_list(LIST(string)));
    auto var = EXP2(VAR, name, call);

    return STM1(VAR, jsP_list(LIST(var)));
}

static Ast *exportstatement(Runtime *J)
{
    // TODO: add multiple exports
    // The statement:
    //      export PI = 3.14
    //  gets rewritten as:
    //      exports.PI = 3.14
    //  which corresponds to the following parse tree:
    //      (exp_ass (exp_member (exp_identifier exports) (identifier Pi)) (exp_number 3.14))
    // This "exports" global object is then loaded by the require() function on import.
    Ast *head, *tail;
    auto name = identifier(J);
    auto ident = jsP_newstrnode(J, EXP_IDENTIFIER, name->string);
    auto exports = jsP_newstrnode(J, EXP_IDENTIFIER, "exports");
    auto member = EXP2(MEMBER, exports, name);

    return EXP2(ASS, member, ident);

//    head = tail = LIST(EXP2(ASS, member, ident));

//    while (jsP_accept(J, ','))
//    {
//        auto name = identifier(J);
//        auto ident = jsP_newstrnode(J, EXP_IDENTIFIER, name->string);
//        auto exports = jsP_newstrnode(J, EXP_IDENTIFIER, "exports");
//        auto member = EXP2(MEMBER, exports, name);
//        tail = tail->b = LIST(EXP2(ASS, member, ident));
//    }
//
//    return STM1(VAR, jsP_list(head));
}

static Ast *forstatement(Runtime *J)
{
    Ast *a, *b, *c, *d;

    // In order to avoid messing with bytecode generation, we rewrite the for loop
    // as a Javascript loop that MuJS can understand. For example,
    // "for x = 1 to 10 step 2" is rewritten as "for x = 1; x <= 10; x += 2".
    auto declared = jsP_accept(J, TK_VAR);
    auto var = identifier(J);
    bool ascending = true;
    auto ident = jsP_newstrnode(J, EXP_IDENTIFIER, var->string);
    jsP_expect(J, '=');

    if (declared)
        a = LIST(EXP2(VAR, var, assignment(J, 1)));
    else
        a = EXP2(ASS, ident, assignment(J, 1));

    // Parse 'to' or 'downto'
    if(jsP_accept(J, TK_DOWNTO))
        ascending = false;
    else if (!jsP_accept(J, TK_TO))
        parse_error(J, "[Syntax error] expected 'to' or 'downto' in for-loop, not %s", get_token_string(J->lookahead));

    if (ascending)
        b = EXP2(LE, ident, expression(J, 1));
    else
        b = EXP2(GE, ident, expression(J, 1));

    // Optional 'step'
    Ast *step;
    if (jsP_accept(J, TK_STEP))
        step = assignment(J, 0);
    else
        step = jsP_newnumnode(J, EXP_NUMBER, 1);

    if (ascending)
        c = EXP2(ASS_ADD, ident, step);
    else
        c = EXP2(ASS_SUB, ident, step);

    // Parse block
    jsP_expect(J, TK_DO);
    d = block_with_end(J);
    jsP_expect(J, TK_END);

    return declared ? STM4(FOR_VAR, a, b, c, d) : STM4(FOR, a, b, c, d);
}

static Ast *foreachstatement(Runtime *J)
{
    Ast *a, *b, *c;

    if (jsP_accept(J, TK_VAR))
    {
        a = vardeclist(J, 1);

        if (jsP_accept(J, TK_IN))
        {
            b = expression(J, 0);
            jsP_expect(J, TK_DO);
            c = block_with_end(J);
            jsP_expect(J, TK_END);

            return STM3(FOR_IN_VAR, a, b, c);
        }
        parse_error(J, "[Syntax error] unexpected token in foreach-loop: %s", get_token_string(J->lookahead));
    }

    a = expression(J, 1);
    jsP_expect(J, TK_IN);
    b = expression(J, 0);
    jsP_expect(J, TK_DO);
    c = block_with_end(J);
    jsP_expect(J, TK_END);

    return STM3(FOR_IN, a, b, c);
}

static Ast *ifstatement(Runtime *J, bool elsif = false)
{
    Ast *a, *b, *c;

    a = expression(J, 0);
    jsP_expect(J, TK_THEN);
    b = block_with_end(J);

    if (jsP_accept(J, TK_ELSIF))
        c = ifstatement(J, true);
    else if (jsP_accept(J, TK_ELSE))
        c = block_with_end(J);
    else
        c = nullptr;

    if (!elsif) jsP_expect(J, TK_END);

    return STM3(IF, a, b, c);
}

static Ast *statement(Runtime *J)
{
    Ast *a, *b, *c, *d;
    Ast *stm;

    INCREC();

    if (jsP_accept(J, TK_VAR))
    {
        a = vardeclist(J, 0);
        semicolon(J);
        stm = STM1(VAR, a);
    }
        /* empty statement */
    else if (jsP_accept(J, ';'))
    {
        stm = STM0(EMPTY);
    }
    else if (jsP_accept(J, TK_IF))
    {
        stm = ifstatement(J);
    }
    else if (jsP_accept(J, TK_REPEAT))
    {
        a = block_with_end(J);
        jsP_expect(J, TK_UNTIL);
        b = expression(J, 0);
        semicolon(J);
        stm = STM2(REPEAT, a, b);
    }
    else if (jsP_accept(J, TK_WHILE))
    {
        a = expression(J, 0);
        jsP_expect(J, TK_DO);
        b = block_with_end(J);
        jsP_expect(J, TK_END);
        stm = STM2(WHILE, a, b);
    }
    else if (jsP_accept(J, TK_FOR))
    {
        stm = forstatement(J);
    }
    else if (jsP_accept(J, TK_FOREACH))
    {
        stm = foreachstatement(J);
    }
    else if (jsP_accept(J, TK_CONTINUE))
    {
        a = identifieropt(J);
        semicolon(J);
        stm = STM1(CONTINUE, a);
    }
    else if (jsP_accept(J, TK_BREAK))
    {
        a = identifieropt(J);
        semicolon(J);
        stm = STM1(BREAK, a);
    }
    else if (jsP_accept(J, TK_RETURN))
    {
        if (J->lookahead != ';' && J->lookahead != '}' && J->lookahead != 0)
            a = expression(J, 0);
        else
            a = nullptr;
        semicolon(J);
        stm = STM1(RETURN, a);
    }
    else if (jsP_accept(J, TK_DEBUG))
    {
        semicolon(J);
        stm = STM0(DEBUGGER);
    }
    else if (jsP_accept(J, TK_FUNCTION))
    {
        jsP_warning(J, "function statements are not standard");
        stm = funstm(J);
    }
    else if (jsP_accept(J, TK_EXPORT))
    {
        stm = exportstatement(J);
    }
    else if (jsP_accept(J, TK_IMPORT))
    {
        stm = importstatement(J);
    }
    else if (jsP_accept(J, TK_PASS))
    {
        stm = STM0(EMPTY);
    }
    /* labelled statement or expression statement */
    else if (J->lookahead == TK_IDENTIFIER)
    {
        a = expression(J, 0);
        if (a->type == EXP_IDENTIFIER && jsP_accept(J, ':'))
        {
            a->type = AST_IDENTIFIER;
            b = statement(J);
            stm = STM2(LABEL, a, b);
        }
        else
        {
            semicolon(J);
            stm = a;
        }
    }
    /* expression statement */
    else
    {
        stm = expression(J, 0);
        semicolon(J);
    }

    DECREC();
    return stm;
}

/* Program */

static Ast *scriptelement(Runtime *J)
{
    if (jsP_accept(J, TK_FUNCTION))
        return fundec(J);
    return statement(J);
}

static Ast *script(Runtime *J, int terminator)
{
    Ast *head, *tail;
    if (J->lookahead == terminator)
        return nullptr;
    head = tail = LIST(scriptelement(J));
    while (J->lookahead != terminator)
        tail = tail->b = LIST(scriptelement(J));
    return jsP_list(head);
}

static Ast *funbody(Runtime *J)
{
    Ast *a;
    a = script(J, TK_END);
    jsP_expect(J, TK_END);

    return a;
}

/* Constant folding */

static int toint32(double d)
{
    double two32 = 4294967296.0;
    double two31 = 2147483648.0;

    if (!std::isfinite(d) || d == 0)
        return 0;

    d = fmod(d, two32);
    d = d >= 0 ? floor(d) : ceil(d) + two32;
    if (d >= two31)
        return d - two32;
    else
        return d;
}

static unsigned int touint32(double d)
{
    return (unsigned int) toint32(d);
}

static int jsP_setnumnode(Ast *node, double x)
{
    node->type = EXP_NUMBER;
    node->number = x;
    node->a = node->b = node->c = node->d = nullptr;
    return 1;
}

static int setstrnode(Ast *node, String s)
{
	node->type = EXP_STRING;
	new (&node->string) String(std::move(s));
	node->a = node->b = node->c = node->d = nullptr;

	return 1;
}

static int jsP_foldconst(Ast *node)
{
    double x, y;
    int a, b;

    if (node->type == AST_LIST)
    {
        while (node)
        {
            jsP_foldconst(node->a);
            node = node->b;
        }
        return 0;
    }

    if (node->type == EXP_NUMBER)
        return 1;

    a = node->a ? jsP_foldconst(node->a) : 0;
    b = node->b ? jsP_foldconst(node->b) : 0;
    if (node->c) jsP_foldconst(node->c);
    if (node->d) jsP_foldconst(node->d);

    if (a)
    {
        x = node->a->number;
        switch (node->type)
        {
        default:
            break;
        case EXP_NEG:
            return jsP_setnumnode(node, -x);
        case EXP_POS:
            return jsP_setnumnode(node, x);
        case EXP_BITNOT:
            return jsP_setnumnode(node, ~toint32(x));
        }

        if (b)
        {
            y = node->b->number;
            switch (node->type)
            {
            default:
                break;
            case EXP_MUL:
                return jsP_setnumnode(node, x * y);
            case EXP_DIV:
                return jsP_setnumnode(node, x / y);
            case EXP_MOD:
                return jsP_setnumnode(node, fmod(x, y));
            case EXP_ADD:
                return jsP_setnumnode(node, x + y);
            case EXP_SUB:
                return jsP_setnumnode(node, x - y);
            case EXP_SHL:
                return jsP_setnumnode(node, toint32(x) << (touint32(y) & 0x1F));
            case EXP_SHR:
                return jsP_setnumnode(node, toint32(x) >> (touint32(y) & 0x1F));
            case EXP_USHR:
                return jsP_setnumnode(node, touint32(x) >> (touint32(y) & 0x1F));
            case EXP_BITAND: // this is now concat
            {
				String s;
				if (double(intptr_t(x)) == x)
					s.append(String::convert(intptr_t(x)));
				else
					s.append(String::convert(x));
				if (double(intptr_t(y)) == y)
					s.append(String::convert(intptr_t(y)));
				else
					s.append(String::convert(y));

				return setstrnode(node, std::move(s));
            }
            case EXP_BITXOR:
                return jsP_setnumnode(node, toint32(x) ^ toint32(y));
            case EXP_BITOR:
                return jsP_setnumnode(node, toint32(x) | toint32(y));
            }
        }
    }

    return 0;
}

/* Main entry point */

Ast *parse(Runtime *J, const String &filename, const String &source)
{
    Ast *p;

    init_lex(J, filename, source);
    jsP_next(J);
    J->astdepth = 0;
    p = script(J, 0);
    if (p)
        jsP_foldconst(p);

    return p;
}

Ast *parse_function(Runtime *J, const char *filename, const char *params, const String &body)
{
    Ast *p = nullptr;
    if (params)
    {
        init_lex(J, filename, params);
        jsP_next(J);
        J->astdepth = 0;
        p = parameters(J);
    }
    return EXP3(FUN, nullptr, p, parse(J, filename, body));
}

} // namespace phonometrica