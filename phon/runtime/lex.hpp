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

#ifndef PHONOMETRICA_LEX_HPP
#define PHONOMETRICA_LEX_HPP

namespace phonometrica {

enum Token
{
    TK_IDENTIFIER = 256,
    TK_NUMBER,
    TK_STRING,
    TK_REGEXP,

    /* multi-character punctuators */
    TK_LE,
    TK_GE,
    TK_EQ,
    TK_NE,
    TK_SHL,
    TK_SHR,
    TK_USHR,
    TK_ADD_ASS,
    TK_SUB_ASS,
    TK_MUL_ASS,
    TK_DIV_ASS,
    TK_MOD_ASS,
    TK_SHL_ASS,
    TK_SHR_ASS,
    TK_USHR_ASS,
    TK_AND_ASS,
    TK_OR_ASS,
    TK_XOR_ASS,
    TK_INC,
    TK_DEC,

    /* keywords */
    TK_AND,
    TK_BREAK,
    TK_CONTINUE,
    TK_DEBUG,
    TK_DELETE,
    TK_DO,
    TK_DOWNTO,
    TK_ELSE,
    TK_ELSIF,
    TK_END,
    TK_EXPORT,
    TK_FALSE,
    TK_FOR,
    TK_FOREACH,
    TK_FUNCTION,
    TK_IF,
    TK_IMPORT,
    TK_IN,
    TK_INSTANCEOF,
    TK_NEW,
    TK_NOT,
    TK_NULL,
    TK_OR,
    TK_PASS,
    TK_REPEAT,
    TK_RETURN,
    TK_STEP,
    TK_THEN,
    TK_THIS,
    TK_TO,
    TK_TRUE,
    TK_TYPEOF,
    TK_UNTIL,
    TK_VAR,
    TK_VOID,
    TK_WHILE
};

bool is_white(char32_t c);

bool is_new_line(char32_t c);

bool is_hex(char32_t c);

int to_hex(char32_t c);

const char *get_token_string(int token);

int find_word(const String &s, const String *list, int num);

void init_lex(Environment *J, const String &filename, const String &source);

int lex(Environment *J);

int lex_json(Environment *J);

} // namespace phonometrica

#endif // PHONOMETRICA_LEX_HPP
