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

#ifndef PHONOMETRICA_PARSE_HPP
#define PHONOMETRICA_PARSE_HPP

namespace phonometrica {

enum AstType
{
    AST_LIST,
    AST_FUNDEC,
    AST_IDENTIFIER,

    EXP_IDENTIFIER,
    EXP_NUMBER,
    EXP_STRING,
    EXP_REGEXP,

    /* literals */
    EXP_UNDEF, /* for array elisions */
    EXP_NULL,
    EXP_TRUE,
    EXP_FALSE,
    EXP_THIS,

    EXP_ARRAY,
    EXP_OBJECT,
    EXP_PROP_VAL,
    EXP_PROP_GET,
    EXP_PROP_SET,

    EXP_FUN,

    /* expressions */
    EXP_INDEX,
    EXP_MEMBER,
    EXP_CALL,
    EXP_NEW,

    EXP_POSTINC,
    EXP_POSTDEC,

    EXP_DELETE,
    EXP_VOID,
    EXP_TYPEOF,
    EXP_PREINC,
    EXP_PREDEC,
    EXP_POS,
    EXP_NEG,
    EXP_BITNOT,
    EXP_LOGNOT,

    EXP_MOD,
    EXP_DIV,
    EXP_MUL,
    EXP_SUB,
    EXP_ADD,
    EXP_USHR,
    EXP_SHR,
    EXP_SHL,
    EXP_IN,
    EXP_INSTANCEOF,
    EXP_GE,
    EXP_LE,
    EXP_GT,
    EXP_LT,
    EXP_NE,
    EXP_EQ,
    EXP_BITAND,
    EXP_BITXOR,
    EXP_BITOR,
    EXP_LOGAND,
    EXP_LOGOR,

    EXP_COND,

    EXP_ASS,
    EXP_ASS_MUL,
    EXP_ASS_DIV,
    EXP_ASS_MOD,
    EXP_ASS_ADD,
    EXP_ASS_SUB,
    EXP_ASS_SHL,
    EXP_ASS_SHR,
    EXP_ASS_USHR,
    EXP_ASS_BITAND,
    EXP_ASS_BITXOR,
    EXP_ASS_BITOR,

    EXP_COMMA,

    EXP_VAR, /* var initializer */

    /* statements */
    STM_BLOCK,
    STM_EMPTY,
    STM_VAR,
    STM_IF,
    STM_REPEAT,
    STM_WHILE,
    STM_FOR,
    STM_FOR_VAR,
    STM_FOR_IN,
    STM_FOR_IN_VAR,
    STM_CONTINUE,
    STM_BREAK,
    STM_RETURN,
    STM_SWITCH,
    STM_THROW,
    STM_TRY,
    STM_DEBUGGER,

    STM_LABEL,
    STM_CASE,
    STM_DEFAULT,
};

struct JumpList
{
    enum AstType type;
    int inst;
    JumpList *next;
};

struct Ast
{
    enum AstType type;
    int line;
    Ast *parent, *a, *b, *c, *d;
    double number;
    String string;
    JumpList *jumps; /* list of break/continue jumps to patch */
    int casejump; /* for switch case clauses */
    Ast *gcnext; /* next in alloc list */
};

Ast *parse_function(Runtime *J, const char *filename, const char *params, const String &body);

Ast *parse(Runtime *J, const String &filename, const String &source);

void free_parse(Runtime *J);

const char *get_ast_string(enum AstType type);

void dump_syntax(Runtime *J, Ast *prog, int minify);

void dump_list(Runtime *J, Ast *prog);

} // namespace phonometrica

#endif // PHONOMETRICA_PARSE_HPP
