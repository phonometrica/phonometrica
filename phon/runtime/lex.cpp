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

namespace phonometrica {

PHON_NORETURN static void jsY_error(Runtime *J, const char *fmt, ...) PHON_PRINTFLIKE(2, 3);

static void jsY_error(Runtime *J, const char *fmt, ...)
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

static const char *tokenstring[] = {
        "(end-of-file)",
        "'\\x01'", "'\\x02'", "'\\x03'", "'\\x04'", "'\\x05'", "'\\x06'", "'\\x07'",
        "'\\x08'", "'\\x09'", "'\\x0A'", "'\\x0B'", "'\\x0C'", "'\\x0D'", "'\\x0E'", "'\\x0F'",
        "'\\x10'", "'\\x11'", "'\\x12'", "'\\x13'", "'\\x14'", "'\\x15'", "'\\x16'", "'\\x17'",
        "'\\x18'", "'\\x19'", "'\\x1A'", "'\\x1B'", "'\\x1C'", "'\\x1D'", "'\\x1E'", "'\\x1F'",
        "' '", "'!'", "'\"'", "'#'", "'$'", "'%'", "'&'", "'\\''",
        "'('", "')'", "'*'", "'+'", "','", "'-'", "'.'", "'/'",
        "'0'", "'1'", "'2'", "'3'", "'4'", "'5'", "'6'", "'7'",
        "'8'", "'9'", "':'", "';'", "'<'", "'='", "'>'", "'?'",
        "'@'", "'A'", "'B'", "'C'", "'D'", "'E'", "'F'", "'G'",
        "'H'", "'I'", "'J'", "'K'", "'L'", "'M'", "'N'", "'O'",
        "'P'", "'Q'", "'R'", "'S'", "'T'", "'U'", "'V'", "'W'",
        "'X'", "'Y'", "'Z'", "'['", "'\'", "']'", "'^'", "'_'",
        "'`'", "'a'", "'b'", "'c'", "'d'", "'e'", "'f'", "'g'",
        "'h'", "'i'", "'j'", "'k'", "'l'", "'m'", "'n'", "'o'",
        "'p'", "'q'", "'r'", "'s'", "'t'", "'u'", "'v'", "'w'",
        "'x'", "'y'", "'z'", "'{'", "'|'", "'}'", "'~'", "'\\x7F'",

        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

        "(identifier)", "(number)", "(string)", "(regexp)",

        "'<='", "'>='", "'=='", "'!='", "'<<'", "'>>'",
        "'>>>'", "'+='", "'-='", "'*='", "'/='", "'%='",
        "'<<='", "'>>='", "'>>>='", "'&='", "'|='", "'^='",
        "'++'", "'--'",

        "'and'", "'break'", "'continue'", "'debug'",
        "'delete'", "'do'", "'downto'", "'else'", "'elsif'",
        "'end'", "'export'", "'false'", "'for'", "'foreach'", "'step'",
        "'function'", "'if'", "'import'", "'in'", "'instanceof'", "'new'", "'not'",
        "'null'", "'or'", "'pass'", "'repeat'", "'return'",
        "'then'", "'this'", "'to'", "'true'", "'typeof'", "'until'", "'var'",
        "'void'", "'while'",
};

const char *get_token_string(int token)
{
    if (token >= 0 && token < (int) nelem(tokenstring))
    if (tokenstring[token])
        return tokenstring[token];
    return "<unknown>";
}

static const String keywords[] = {
        "and", "break", "continue", "debug", "delete",
        "do", "downto", "else", "elsif", "end", "export", "false", "for", "foreach", "function", "if",
        "import", "in", "instanceof", "new", "not", "null", "or", "pass", "repeat", "return", "step",
        "then", "this", "to", "true", "typeof", "until", "var", "void", "while"
};

int find_word(const String &s, const String *list, int num)
{
    int l = 0;
    int r = num - 1;
    while (l <= r)
    {
        int m = (l + r) >> 1;
        int c = s.compare(list[m]);
        if (c < 0)
            r = m - 1;
        else if (c > 0)
            l = m + 1;
        else
            return m;
    }
    return -1;
}

static int jsY_findkeyword(Runtime *J, const String &s)
{
    int i = find_word(s, keywords, nelem(keywords));
    if (i >= 0)
    {
        J->text = keywords[i];
        return TK_AND + i; /* first keyword + i */
    }
    J->text = J->internalize(s);
    return TK_IDENTIFIER;
}

bool is_white(char32_t c)
{
    return c == 0x9 || c == 0xB || c == 0xC || c == 0x20 || c == 0xA0 || c == 0xFEFF;
}

bool is_new_line(char32_t c)
{
    return c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

#define isalpha(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define isdigit(c) (c >= '0' && c <= '9')
#define ishex(c) ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

static bool jsY_isidentifierstart(char32_t c)
{
    return isalpha(c) || c == '$' || c == '_' || String::is_letter(c);
}

static bool jsY_isidentifierpart(char32_t c)
{
    return isdigit(c) || isalpha(c) || c == '$' || c == '_' || String::is_letter(c);
}

static bool jsY_isdec(char32_t c)
{
    return isdigit(c);
}

bool is_hex(char32_t c)
{
    return isdigit(c) || ishex(c);
}

int to_hex(char32_t c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 0xA;
    if (c >= 'A' && c <= 'F') return c - 'A' + 0xA;
    return 0;
}

static void jsY_next(Runtime *J)
{
    char32_t c = J->read_char();

    /* consume CR LF as one unit */
    if (c == '\r' && *J->pos == '\n')
        c = J->read_char();

    if (is_new_line(c))
    {
        J->line++;
        c = '\n';
    }
    J->lexchar = c;
}

#define jsY_accept(J, x) (J->lexchar == x ? (jsY_next(J), 1) : 0)

#define jsY_expect(J, x) if (!jsY_accept(J, x)) jsY_error(J, "[Syntax error] expected '%c'", x)

static void jsY_unescape(Runtime *J)
{
    if (jsY_accept(J, '\\'))
    {
        if (jsY_accept(J, 'u'))
        {
            int x = 0;
            if (!is_hex(J->lexchar))
            { goto error; }
            x |= to_hex(J->lexchar) << 12;
            jsY_next(J);
            if (!is_hex(J->lexchar))
            { goto error; }
            x |= to_hex(J->lexchar) << 8;
            jsY_next(J);
            if (!is_hex(J->lexchar))
            { goto error; }
            x |= to_hex(J->lexchar) << 4;
            jsY_next(J);
            if (!is_hex(J->lexchar))
            { goto error; }
            x |= to_hex(J->lexchar);
            J->lexchar = x;
            return;
        }
        error:
        jsY_error(J, "[Syntax error] unexpected escape sequence");
    }
}

static void textpush(Runtime *J, char32_t c)
{
    J->lexbuf.append(c);
}

static void lexlinecomment(Runtime *J)
{
    while (J->lexchar && J->lexchar != '\n')
        jsY_next(J);
}

static int lexcomment(Runtime *J)
{
    /* already consumed initial '!' '*' sequence */
    while (J->lexchar != 0)
    {
        if (jsY_accept(J, '*'))
        {
            while (J->lexchar == '*')
                jsY_next(J);
            if (jsY_accept(J, '!'))
                return 0;
        }
        else
            jsY_next(J);
    }
    return -1;
}

static double lexhex(Runtime *J)
{
    double n = 0;
    if (!is_hex(J->lexchar))
        jsY_error(J, "[Syntax error] malformed hexadecimal number");
    while (is_hex(J->lexchar))
    {
        n = n * 16 + to_hex(J->lexchar);
        jsY_next(J);
    }
    return n;
}

#if 0

static double lexinteger(State *J)
{
    double n = 0;
    if (!jsY_isdec(J->lexchar))
        jsY_error(J, "[Syntax error] malformed number");
    while (jsY_isdec(J->lexchar)) {
        n = n * 10 + (J->lexchar - '0');
        jsY_next(J);
    }
    return n;
}

static double lexfraction(State *J)
{
    double n = 0;
    double d = 1;
    while (jsY_isdec(J->lexchar)) {
        n = n * 10 + (J->lexchar - '0');
        d = d * 10;
        jsY_next(J);
    }
    return n / d;
}

static double lexexponent(State *J)
{
    double sign;
    if (jsY_accept(J, 'e') || jsY_accept(J, 'E')) {
        if (jsY_accept(J, '-')) sign = -1;
        else if (jsY_accept(J, '+')) sign = 1;
        else sign = 1;
        return sign * lexinteger(J);
    }
    return 0;
}

static int lexnumber(State *J)
{
    double n;
    double e;

    if (jsY_accept(J, '0')) {
        if (jsY_accept(J, 'x') || jsY_accept(J, 'X')) {
            J->number = lexhex(J);
            return TK_NUMBER;
        }
        if (jsY_isdec(J->lexchar))
            jsY_error(J, "[Syntax error] number with leading zero");
        n = 0;
        if (jsY_accept(J, '.'))
            n += lexfraction(J);
    } else if (jsY_accept(J, '.')) {
        if (!jsY_isdec(J->lexchar))
            return '.';
        n = lexfraction(J);
    } else {
        n = lexinteger(J);
        if (jsY_accept(J, '.'))
            n += lexfraction(J);
    }

    e = lexexponent(J);
    if (e < 0)
        n /= pow(10, -e);
    else if (e > 0)
        n *= pow(10, e);

    if (jsY_isidentifierstart(J->lexchar))
        jsY_error(J, "[Syntax error] number with letter suffix");

    J->number = n;
    return TK_NUMBER;
}

#else

static int lexnumber(Runtime *J)
{
    const char *s = J->pos - 1;

    if (jsY_accept(J, '0'))
    {
        if (jsY_accept(J, 'x') || jsY_accept(J, 'X'))
        {
            J->number = lexhex(J);
            return TK_NUMBER;
        }
        if (jsY_isdec(J->lexchar))
            jsY_error(J, "[Syntax error] number with leading zero");
        if (jsY_accept(J, '.'))
        {
            while (jsY_isdec(J->lexchar))
                jsY_next(J);
        }
    }
    else if (jsY_accept(J, '.'))
    {
        if (!jsY_isdec(J->lexchar))
            return '.';
        while (jsY_isdec(J->lexchar))
            jsY_next(J);
    }
    else
    {
        while (jsY_isdec(J->lexchar))
            jsY_next(J);
        if (jsY_accept(J, '.'))
        {
            while (jsY_isdec(J->lexchar))
                jsY_next(J);
        }
    }

    if (jsY_accept(J, 'e') || jsY_accept(J, 'E'))
    {
        if (J->lexchar == '-' || J->lexchar == '+')
            jsY_next(J);
        if (jsY_isdec(J->lexchar))
            while (jsY_isdec(J->lexchar))
                jsY_next(J);
        else
            jsY_error(J, "[Syntax error] missing exponent");
    }

    if (jsY_isidentifierstart(J->lexchar))
        jsY_error(J, "[Syntax error] number with letter suffix");

    J->number = str_to_double(s, nullptr);
    return TK_NUMBER;
}

#endif

static int lexescape(Runtime *J)
{
    int x = 0;

    /* already consumed '\' */

    if (jsY_accept(J, '\n'))
        return 0;

    switch (J->lexchar)
    {
    case 0:
        jsY_error(J, "[Syntax error] unterminated escape sequence");
    case 'u':
        jsY_next(J);
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 12;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 8;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 4;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar);
            jsY_next(J);
        }
        textpush(J, x);
        break;
    case 'x':
        jsY_next(J);
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 4;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar);
            jsY_next(J);
        }
        textpush(J, x);
        break;
    case '0':
        jsY_next(J);
        break;
    case '\\':
        textpush(J, '\\');
        jsY_next(J);
        break;
    case '\'':
        textpush(J, '\'');
        jsY_next(J);
        break;
    case '"':
        textpush(J, '"');
        jsY_next(J);
        break;
    case 'b':
        textpush(J, '\b');
        jsY_next(J);
        break;
    case 'f':
        textpush(J, '\f');
        jsY_next(J);
        break;
    case 'n':
        textpush(J, '\n');
        jsY_next(J);
        break;
    case 'r':
        textpush(J, '\r');
        jsY_next(J);
        break;
    case 't':
        textpush(J, '\t');
        jsY_next(J);
        break;
    case 'v':
        textpush(J, '\v');
        jsY_next(J);
        break;
    default:
        textpush(J, J->lexchar);
        jsY_next(J);
        break;
    }
    return 0;
}

static int lexstring(Runtime *J)
{
    int q = J->lexchar;
    jsY_next(J);

    J->lexbuf.clear();

    while (J->lexchar != q)
    {
        if (J->lexchar == 0 || J->lexchar == '\n')
            jsY_error(J, "[Syntax error] string not terminated");
        if (jsY_accept(J, '\\'))
        {
            if (lexescape(J))
                jsY_error(J, "[Syntax error] malformed escape sequence");
        }
        else
        {
            textpush(J, J->lexchar);
            jsY_next(J);
        }
    }
    jsY_expect(J, q);

    J->text = J->internalize(J->lexbuf);
    return TK_STRING;
}

/* the ugliest language wart ever... */
static int isregexpcontext(int last)
{
    switch (last)
    {
    case ']':
    case ')':
    case '}':
    case TK_IDENTIFIER:
    case TK_NUMBER:
    case TK_STRING:
    case TK_FALSE:
    case TK_NULL:
    case TK_THIS:
    case TK_TRUE:
        return 0;
    default:
        return 1;
    }
}

static int lexregexp(Runtime *J)
{
    int g, m, i;
    int inclass = 0;

    /* already consumed initial '/' */

    J->lexbuf.clear();

    /* regexp body */
    while (J->lexchar != '/' || inclass)
    {
        if (J->lexchar == 0 || J->lexchar == '\n')
        {
            jsY_error(J, "[Syntax error] regular expression not terminated");
        }
        else if (jsY_accept(J, '\\'))
        {
            if (jsY_accept(J, '/'))
            {
                textpush(J, '/');
            }
            else
            {
                textpush(J, '\\');
                if (J->lexchar == 0 || J->lexchar == '\n')
                    jsY_error(J, "[Syntax error] regular expression not terminated");
                textpush(J, J->lexchar);
                jsY_next(J);
            }
        }
        else
        {
            if (J->lexchar == '[' && !inclass)
                inclass = 1;
            if (J->lexchar == ']' && inclass)
                inclass = 0;
            textpush(J, J->lexchar);
            jsY_next(J);
        }
    }
    jsY_expect(J, '/');

    /* regexp flags */
    g = i = m = 0;

    while (jsY_isidentifierpart(J->lexchar))
    {
        if (jsY_accept(J, 'g')) ++g;
        else if (jsY_accept(J, 'i')) ++i;
        else if (jsY_accept(J, 'm')) ++m;
        else jsY_error(J, "[Syntax error] illegal flag in regular expression: %c", J->lexchar);
    }

    if (g > 1 || i > 1 || m > 1)
        jsY_error(J, "[Syntax error] duplicated flag in regular expression");

    J->text = J->internalize(J->lexbuf);
    J->number = 0;
    if (g) J->number += PHON_REGEXP_G;
    if (i) J->number += PHON_REGEXP_I;
    if (m) J->number += PHON_REGEXP_M;
    return TK_REGEXP;
}

/* simple "return [no Line Terminator here] ..." contexts */
static int isnlthcontext(int last)
{
    switch (last)
    {
    case TK_BREAK:
    case TK_CONTINUE:
    case TK_RETURN:
        return 1;
    default:
        return 0;
    }
}

static int jsY_lexx(Runtime *J)
{
    J->newline = 0;

    while (1)
    {
        J->lexline = J->line; /* save location of beginning of token */

        while (is_white(J->lexchar))
            jsY_next(J);

        if (jsY_accept(J, '\n'))
        {
            J->newline = 1;
            if (isnlthcontext(J->lasttoken))
                return ';';
            continue;
        }

        if (jsY_accept(J, '#'))
        {
            lexlinecomment(J);
            continue;
        }

        if (jsY_accept(J, '/'))
        {
            if (isregexpcontext(J->lasttoken))
            {
                return lexregexp(J);
            }
            else if (jsY_accept(J, '='))
            {
                return TK_DIV_ASS;
            }
            else
            {
                return '/';
            }
        }

        if (J->lexchar >= '0' && J->lexchar <= '9')
        {
            return lexnumber(J);
        }

        switch (J->lexchar)
        {
        case '(':
            jsY_next(J);
            return '(';
        case ')':
            jsY_next(J);
            return ')';
        case ',':
            jsY_next(J);
            return ',';
        case ':':
            jsY_next(J);
            return ':';
        case ';':
            jsY_next(J);
            return ';';
        case '?':
            jsY_next(J);
            return '?';
        case '[':
            jsY_next(J);
            return '[';
        case ']':
            jsY_next(J);
            return ']';
        case '{':
            jsY_next(J);
            return '{';
        case '}':
            jsY_next(J);
            return '}';
        case '~':
            jsY_next(J);
            return '~';

        case '\'':
        case '"':
            return lexstring(J);

        case '.':
            return lexnumber(J);

        case '<':
            jsY_next(J);
            if (jsY_accept(J, '<'))
            {
                if (jsY_accept(J, '='))
                    return TK_SHL_ASS;
                return TK_SHL;
            }
            if (jsY_accept(J, '='))
                return TK_LE;
            return '<';

        case '>':
            jsY_next(J);
            if (jsY_accept(J, '>'))
            {
                if (jsY_accept(J, '>'))
                {
                    if (jsY_accept(J, '='))
                        return TK_USHR_ASS;
                    return TK_USHR;
                }
                if (jsY_accept(J, '='))
                    return TK_SHR_ASS;
                return TK_SHR;
            }
            if (jsY_accept(J, '='))
                return TK_GE;
            return '>';

        case '=':
            jsY_next(J);
            if (jsY_accept(J, '='))
            {
                return TK_EQ;
            }
            return '=';

        case '!':
            jsY_next(J);
            if (jsY_accept(J, '='))
            {
                return TK_NE;
            }
            else if (jsY_accept(J, '*'))
            {
                if (lexcomment(J))
                    jsY_error(J, "[Syntax error] multi-line comment not terminated");
                continue;
            }
            else
                return '!';

        case '+':
            jsY_next(J);
            if (jsY_accept(J, '+'))
                return TK_INC;
            if (jsY_accept(J, '='))
                return TK_ADD_ASS;
            return '+';

        case '-':
            jsY_next(J);
            if (jsY_accept(J, '-'))
                return TK_DEC;
            if (jsY_accept(J, '='))
                return TK_SUB_ASS;
            return '-';

        case '*':
            jsY_next(J);
            if (jsY_accept(J, '='))
                return TK_MUL_ASS;
            return '*';

        case '%':
            jsY_next(J);
            if (jsY_accept(J, '='))
                return TK_MOD_ASS;
            return '%';

        case '&':
            jsY_next(J);
            if (jsY_accept(J, '&'))
                return TK_AND;
            if (jsY_accept(J, '='))
                return TK_AND_ASS;
            return '&';

        case '|':
            jsY_next(J);
            if (jsY_accept(J, '|'))
                return TK_OR;
            if (jsY_accept(J, '='))
                return TK_OR_ASS;
            return '|';

        case '^':
            jsY_next(J);
            if (jsY_accept(J, '='))
                return TK_XOR_ASS;
            return '^';

        case 0:
            return 0; /* EOF */
        }

        /* Handle \uXXXX escapes in identifiers */
        jsY_unescape(J);
        if (jsY_isidentifierstart(J->lexchar))
        {
            J->lexbuf.clear();
            textpush(J, J->lexchar);

            jsY_next(J);
            jsY_unescape(J);
            while (jsY_isidentifierpart(J->lexchar))
            {
                textpush(J, J->lexchar);
                jsY_next(J);
                jsY_unescape(J);
            }

            return jsY_findkeyword(J, J->lexbuf);
        }

        if (J->lexchar >= 0x20 && J->lexchar <= 0x7E)
            jsY_error(J, "[Syntax error] unexpected character: '%c'", J->lexchar);
        jsY_error(J, "[Syntax error] unexpected character: \\u%04X", J->lexchar);
    }
}

void init_lex(Runtime *J, const String &filename, const String &source)
{
    J->filename = filename;
    J->source = source;
    J->pos = J->source.begin();
    J->line = 1;
    J->lasttoken = 0;
    jsY_next(J); /* load first lookahead character */
}

int lex(Runtime *J)
{
    return J->lasttoken = jsY_lexx(J);
}

static int lexjsonnumber(Runtime *J)
{
    const char *s = J->pos - 1;

    if (J->lexchar == '-')
        jsY_next(J);

    if (J->lexchar == '0')
        jsY_next(J);
    else if (J->lexchar >= '1' && J->lexchar <= '9')
        while (isdigit(J->lexchar))
            jsY_next(J);
    else
        jsY_error(J, "[Syntax error] unexpected non-digit");

    if (jsY_accept(J, '.'))
    {
        if (isdigit(J->lexchar))
            while (isdigit(J->lexchar))
                jsY_next(J);
        else
            jsY_error(J, "[Syntax error] missing digits after decimal point");
    }

    if (jsY_accept(J, 'e') || jsY_accept(J, 'E'))
    {
        if (J->lexchar == '-' || J->lexchar == '+')
            jsY_next(J);
        if (isdigit(J->lexchar))
            while (isdigit(J->lexchar))
                jsY_next(J);
        else
            jsY_error(J, "[Syntax error] missing digits after exponent indicator");
    }

    J->number = str_to_double(s, nullptr);
    return TK_NUMBER;
}

static int lexjsonescape(Runtime *J)
{
    int x = 0;

    /* already consumed '\' */

    switch (J->lexchar)
    {
    default:
        jsY_error(J, "[Syntax error] invalid escape sequence");
    case 'u':
        jsY_next(J);
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 12;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 8;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar) << 4;
            jsY_next(J);
        }
        if (!is_hex(J->lexchar)) return 1;
        else
        {
            x |= to_hex(J->lexchar);
            jsY_next(J);
        }
        textpush(J, x);
        break;
    case '"':
        textpush(J, '"');
        jsY_next(J);
        break;
    case '\\':
        textpush(J, '\\');
        jsY_next(J);
        break;
    case '/':
        textpush(J, '/');
        jsY_next(J);
        break;
    case 'b':
        textpush(J, '\b');
        jsY_next(J);
        break;
    case 'f':
        textpush(J, '\f');
        jsY_next(J);
        break;
    case 'n':
        textpush(J, '\n');
        jsY_next(J);
        break;
    case 'r':
        textpush(J, '\r');
        jsY_next(J);
        break;
    case 't':
        textpush(J, '\t');
        jsY_next(J);
        break;
    }
    return 0;
}

static int lexjsonstring(Runtime *J)
{
    J->lexbuf.clear();

    while (J->lexchar != '"')
    {
        if (J->lexchar == 0)
            jsY_error(J, "[Syntax error] unterminated string");
        else if (J->lexchar < 32)
            jsY_error(J, "[Syntax error] invalid control character in string");
        else if (jsY_accept(J, '\\'))
            lexjsonescape(J);
        else
        {
            textpush(J, J->lexchar);
            jsY_next(J);
        }
    }
    jsY_expect(J, '"');

    J->text = J->internalize(J->lexbuf);
    return TK_STRING;
}

int lex_json(Runtime *J)
{
    while (1)
    {
        J->lexline = J->line; /* save location of beginning of token */

        while (is_white(J->lexchar) || J->lexchar == '\n')
            jsY_next(J);

        if ((J->lexchar >= '0' && J->lexchar <= '9') || J->lexchar == '-')
            return lexjsonnumber(J);

        switch (J->lexchar)
        {
        case ',':
            jsY_next(J);
            return ',';
        case ':':
            jsY_next(J);
            return ':';
        case '[':
            jsY_next(J);
            return '[';
        case ']':
            jsY_next(J);
            return ']';
        case '{':
            jsY_next(J);
            return '{';
        case '}':
            jsY_next(J);
            return '}';

        case '"':
            jsY_next(J);
            return lexjsonstring(J);

        case 'f':
            jsY_next(J);
            jsY_expect(J, 'a');
            jsY_expect(J, 'l');
            jsY_expect(J, 's');
            jsY_expect(J, 'e');
            return TK_FALSE;

        case 'n':
            jsY_next(J);
            jsY_expect(J, 'u');
            jsY_expect(J, 'l');
            jsY_expect(J, 'l');
            return TK_NULL;

        case 't':
            jsY_next(J);
            jsY_expect(J, 'r');
            jsY_expect(J, 'u');
            jsY_expect(J, 'e');
            return TK_TRUE;

        case 0:
            return 0; /* EOF */
        }

        if (J->lexchar >= 0x20 && J->lexchar <= 0x7E)
            jsY_error(J, "[Syntax error] unexpected character: '%c'", J->lexchar);
        jsY_error(J, "[Syntax error] unexpected character: \\u%04X", J->lexchar);
    }
}

} // namespace phonometrica