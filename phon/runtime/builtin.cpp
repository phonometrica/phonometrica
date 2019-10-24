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
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

static void jsB_parse_int(Runtime &rt)
{
    auto str = rt.to_string(1);
    auto s = str.data();
    int radix = rt.is_defined(2) ? rt.to_integer(2) : 10;
    double sign = 1;
    double n;
    char *e;

    while (is_white(*s) || is_new_line(*s))
        ++s;
    if (*s == '-')
    {
        ++s;
        sign = -1;
    }
    else if (*s == '+')
    {
        ++s;
    }
    if (radix == 0)
    {
        radix = 10;
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        {
            s += 2;
            radix = 16;
        }
    }
    else if (radix < 2 || radix > 36)
    {
        rt.push(NAN);
        return;
    }
    n = strtol(s, &e, radix);
    if (s == e)
        rt.push(NAN);
    else
        rt.push(n * sign);
}

static void jsB_parse_float(Runtime &rt)
{
    auto str = rt.to_string(1);
    auto s = str.data();
    char *e;
    double n;

    while (is_white(*s) || is_new_line(*s)) ++s;
    if (!strncmp(s, "Infinity", 8))
        rt.push(INFINITY);
    else if (!strncmp(s, "+Infinity", 9))
        rt.push(INFINITY);
    else if (!strncmp(s, "-Infinity", 9))
        rt.push(-INFINITY);
    else
    {
        n = string_to_float(s, &e);
        if (e == s)
            rt.push(NAN);
        else
            rt.push(n);
    }
}

static void jsB_is_nan(Runtime &rt)
{
    double n = rt.to_number(1);
    rt.push_boolean(std::isnan(n));
}

static void jsB_is_finite(Runtime &rt)
{
    double n = rt.to_number(1);
    rt.push_boolean(std::isfinite(n));
}

static void Encode(Runtime *J, const String &s, const char *unescaped)
{
    String buffer;
    auto str = s.data();
    static const char *HEX = "0123456789ABCDEF";

    while (*str)
    {
        int c = (unsigned char) *str++;
        if (strchr(unescaped, c))
            buffer.append(c);
        else
        {
            buffer.append('%');
            buffer.append(HEX[(c >> 4) & 0xf]);
            buffer.append(HEX[c & 0xf]);
        }
    }
    J->push(std::move(buffer));
}

static void Decode(Runtime *J, const String &s, const char *reserved)
{
    String buffer;
    int a, b;
    auto str = s.data();

    while (*str)
    {
        int c = (unsigned char) *str++;
        if (c != '%')
            buffer.append(c);
        else
        {
            if (!str[0] || !str[1])
                throw J->raise("URI error", "truncated escape sequence");
            a = *str++;
            b = *str++;
            if (!is_hex(a) || !is_hex(b))
                throw J->raise("URI error", "invalid escape sequence");
            c = to_hex(a) << 4 | to_hex(b);
            if (!strchr(reserved, c))
                buffer.append(c);
            else
            {
                buffer.append('%');
                buffer.append(a);
                buffer.append(b);
            }
        }
    }

    J->push(std::move(buffer));
}

#define URIRESERVED ";/?:@&=+$,"
#define URIALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define URIDIGIT "0123456789"
#define URIMARK "-_.!~*`()"
#define URIUNESCAPED URIALPHA URIDIGIT URIMARK

static void jsB_decode_uri(Runtime &rt)
{
    Decode(&rt, rt.to_string(1), URIRESERVED "#");
}

static void jsB_decode_uri_component(Runtime &rt)
{
    Decode(&rt, rt.to_string(1), "");
}

static void jsB_encode_uri(Runtime &rt)
{
    Encode(&rt, rt.to_string(1), URIUNESCAPED URIRESERVED "#");
}

static void jsB_encode_uri_component(Runtime &rt)
{
    Encode(&rt, rt.to_string(1), URIUNESCAPED);
}

void Runtime::initialize()
{
    /* Create the prototype objects here, before the constructors */
    object_meta = new Object(*this, PHON_COBJECT, nullptr);
    list_meta = new Object(*this, PHON_CLIST, object_meta);
    function_meta = new Object(*this, PHON_CCFUNCTION, object_meta);
    boolean_meta = new Object(*this, PHON_CBOOLEAN, object_meta);
    number_meta = new Object(*this, PHON_CNUMBER, object_meta);
    string_meta = new Object(*this, PHON_CSTRING, object_meta);
    regex_meta = new Object(*this, PHON_COBJECT, object_meta);
    array_meta = new Object(*this, PHON_CARRAY, object_meta);
    date_meta = new Object(*this, PHON_CDATE, object_meta);
    file_meta = new Object(*this, PHON_CFILE, object_meta);

    /* Create the constructors and fill out the prototype objects */
    init_object();
    init_list();
    init_function();
    init_boolean();
    init_number();
    init_string();
    init_regexp();
    init_array();
    init_file();
    init_date();
    init_math();
    init_json();
    init_system();

    /* Initialize the global object */
    push(NAN);
    def_global("undefined", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    push(INFINITY);
    def_global("Infinity", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    add_global_function("parse_int", jsB_parse_int, 1);
    add_global_function("parse_float", jsB_parse_float, 1);
    add_global_function("is_undefined", jsB_is_nan, 1);
    add_global_function("is_finite", jsB_is_finite, 1);

    add_global_function("decode_uri", jsB_decode_uri, 1);
    add_global_function("decode_uri_component", jsB_decode_uri_component, 1);
    add_global_function("encode_uri", jsB_encode_uri, 1);
    add_global_function("encode_uri_component", jsB_encode_uri_component, 1);
}

} // namespace phonometrica
