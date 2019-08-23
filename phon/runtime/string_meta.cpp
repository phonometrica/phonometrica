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

#include <phon/error.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

static String checkstring(Environment *J, int idx)
{
    if (!J->is_coercible(idx))
        throw J->raise("Type error", "string function called on null or undefined");
    return J->to_string(idx);
}

static void jsB_new_String(Environment &env)
{
    env.new_string(env.top_count() > 1 ? env.to_string(1) : String());
}

static void jsB_String(Environment &env)
{
    env.push(env.top_count() > 1 ? env.to_string(1) : String());
}

static void string_to_string(Environment &env)
{
    Object *self = env.to_object(0);
    if (self->type != PHON_CSTRING) throw env.raise("Type error", "not a string");
    env.push(self->as.string);
}

static void string_to_value(Environment &env)
{
    Object *self = env.to_object(0);
    if (self->type != PHON_CSTRING) throw env.raise("Type error", "not a string");
    env.push(self->as.string);
}

static void string_char_at(Environment &env)
{
    auto s = checkstring(&env, 0);
    auto pos = env.to_integer(1);

    try
    {
	    String grapheme = s.next_grapheme(pos);
	    env.push(std::move(grapheme));
    }
    catch (std::exception &e)
    {
        throw env.raise("Index error", e);
    }
}

static void string_concat(Environment &env)
{
    auto top = env.top_count();
    int n;

    if (top == 1)
        return;

    String result = checkstring(&env, 0);

    for (int i = 1; i < top; ++i)
    {
        auto s = env.to_string(i);
        result.append(s);
    }

    env.push(std::move(result));
}

static void string_find(Environment &env)
{
    auto haystack = checkstring(&env, 0);
    auto needle = env.to_string(1);
    auto index = haystack.find(needle);
    env.push(index);
}

static void string_rfind(Environment &env)
{
    auto haystack = checkstring(&env, 0);
    auto needle = env.to_string(1);
    auto index = haystack.rfind(needle);
    env.push(index);
}

static void string_compare(Environment &env)
{
    auto a = checkstring(&env, 0);
    auto b = env.to_string(1);
    env.push(a.compare(b));
}

static void string_mid(Environment &env)
{
    auto str = checkstring(&env, 0);
    auto from = env.to_integer(1);
    auto count = env.is_defined(2) ? env.to_integer(2) : -1;

    try
    {
        auto result = str.mid(from, count);
        env.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw env.raise("Index error", e);
    }
}

static void string_left(Environment &env)
{
    auto str = checkstring(&env, 0);
    auto count = env.to_integer(1);

    try
    {
        auto result = str.left(count);
        env.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw env.raise("Index error", e);
    }
}

static void string_right(Environment &env)
{
    auto str = checkstring(&env, 0);
    auto count = env.to_integer(1);

    try
    {
        auto result = str.right(count);
        env.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw env.raise("Index error", e);
    }
}

static void string_to_lower(Environment &env)
{
    auto src = checkstring(&env, 0);
    env.push(src.to_lower());
}

static void string_starts_with(Environment &env)
{
    auto self = checkstring(&env, 0);
    auto prefix = env.to_string(1);
    env.push_boolean(self.starts_with(prefix));
}

static void string_ends_with(Environment &env)
{
    auto self = checkstring(&env, 0);
    auto suffix = env.to_string(1);
    env.push_boolean(self.ends_with(suffix));
}

static void string_contains(Environment &env)
{
    auto self = checkstring(&env, 0);
    auto infix = env.to_string(1);
    env.push_boolean(self.contains(infix));
}

static void string_count(Environment &env)
{
    auto self = checkstring(&env, 0);
    auto substring = env.to_string(1);
    env.push(self.count(substring));
}

static void string_to_upper(Environment &env)
{
    auto src = checkstring(&env, 0);
    env.push(src.to_upper());
}

static int istrim(int c)
{
    return c == 0x9 || c == 0xB || c == 0xC || c == 0x20 || c == 0xA0 || c == 0xFEFF ||
           c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

static void string_trim(Environment &env)
{
    auto s = checkstring(&env, 0);
    env.push(s.trim());
}

static void string_ltrim(Environment &env)
{
    auto s = checkstring(&env, 0);
    env.push(s.ltrim());
}

static void string_rtrim(Environment &env)
{
    auto s = checkstring(&env, 0);
    env.push(s.rtrim());
}

static void string_from_codepoint(Environment &env)
{
    auto top = env.top_count();
    String result;

    for (int i = 1; i < top; ++i)
    {
        char32_t c = env.to_uint32(i);
        result.append(String::encode(c).data);
    }

    env.push(std::move(result));
}

static void string_replace(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto before = env.to_string(1);
    auto after = env.to_string(2);
    env.push(source.replace(before, after));
}

static void string_replace_first(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto before = env.to_string(1);
    auto after = env.to_string(2);
    env.push(source.replace_first(before, after));
}

static void string_replace_last(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto before = env.to_string(1);
    auto after = env.to_string(2);
    env.push(source.replace_last(before, after));
}

static void string_replace_at(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto pos = env.to_integer(1);
    auto count = env.to_integer(2);
    auto after = env.to_string(3);
    env.push(source.replace(pos, count, after));
}

static void string_remove(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto substring = env.to_string(1);
    env.push(source.remove(substring));
}

static void string_remove_first(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto substring = env.to_string(1);
    env.push(source.remove_first(substring));
}

static void string_remove_last(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto substring = env.to_string(1);
    env.push(source.remove_last(substring));
}

static void string_remove_at(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto pos = env.to_integer(1);
    auto count = env.to_integer(2);
    env.push(source.remove(pos, count));
}

static void string_insert(Environment &env)
{
    auto source = checkstring(&env, 0);
    auto pos = env.to_integer(1);
    auto s = env.to_string(2);
    env.push(source.insert(pos, s));
}

static void string_reverse(Environment &env)
{
    auto self = checkstring(&env, 0);
    env.push(self.reverse());
}

static void split_with_regex(Environment &env)
{
    throw error("split regex not implemented");
#if 0
    js_Regexp *re;
    int limit, len, k;
    const char *p, *a, *b, *c, *e;
    Resub m;

    auto &text = checkstring(&env, 0);
    re = js_toregexp(&env, 1);
    limit = env.is_defined(2) ? env.to_integer(2) : 1 << 30;

    env.new_list();
    len = 0;

    e = text.end();

    /* splitting the empty string */
    if (e == text)
    {
        if (js_regexec((Reprog *) re->prog, text, &m, 0))
        {
            if (len == limit) return;
            env.push_literal("");
            js_setindex(&env, -2, 0);
        }
        return;
    }

    p = a = text;
    while (a < e)
    {
        if (js_regexec((Reprog *) re->prog, a, &m, a > text ? REG_NOTBOL : 0))
            break; /* no match */

        b = m.sub[0].sp;
        c = m.sub[0].ep;

        /* empty string at end of last match */
        if (b == p)
        {
            ++a;
            continue;
        }

        if (len == limit) return;
        env.push_lstring(p, b - p);
        js_setindex(&env, -2, len++);

        for (k = 1; k < m.nsub; ++k)
        {
            if (len == limit) return;
            env.push_lstring(m.sub[k].sp, m.sub[k].ep - m.sub[k].sp);
            js_setindex(&env, -2, len++);
        }

        a = p = c;
    }

    if (len == limit) return;
    env.push_string(p);
    js_setindex(&env, -2, len);
#endif
}

static void split_with_string(Environment &env)
{
    auto str = checkstring(&env, 0);
    auto sep = env.to_string(1);
    auto pieces = str.split(sep);
    env.new_list();

    for (int i = 1; i <= pieces.size(); i++)
    {
        env.push(std::move(pieces[i]));
        js_setindex(&env, -2, i-1); // TODO: changes indices to base 1 in split() and everywhere
    }
}

static void string_split(Environment &env)
{
    if (env.is_null(1))
    {
        env.new_list();
        env.copy(0);
        js_setindex(&env, -2, 0);
    }
    else if (env.is_regex(1))
    {
        split_with_regex(env);
    }
    else
    {
        split_with_string(env);
    }
}

static void string_is_empty(Environment &env)
{
    auto s = checkstring(&env, 0);
    env.push_boolean(s.empty());
}

void Environment::init_string()
{
    new (&string_meta->as.string) String;

    push(string_meta);
    {
        add_method("String.meta.is_empty", string_is_empty, 0);
        add_method("String.meta.to_string", string_to_string, 0);
        add_method("String.meta.to_value", string_to_value, 0);
        add_method("String.meta.at", string_char_at, 1);
        add_method("String.meta.concat", string_concat, 0); /* 1 */
        add_method("String.meta.find", string_find, 1);
        add_method("String.meta.find_last", string_rfind, 1);
        add_method("String.meta.compare", string_compare, 1);
        add_method("String.meta.replace", string_replace, 2);
        add_method("String.meta.replace_first", string_replace_first, 2);
        add_method("String.meta.replace_last", string_replace_last, 2);
        add_method("String.meta.replace_at", string_replace_at, 3);
        add_method("String.meta.mid", string_mid, 2);
        add_method("String.meta.left", string_left, 1);
        add_method("String.meta.right", string_right, 1);
        add_method("String.meta.starts_with", string_starts_with, 1);
        add_method("String.meta.ends_with", string_ends_with, 1);
        add_method("String.meta.contains", string_contains, 1);
        add_method("String.meta.count", string_count, 1);
        add_method("String.meta.split", string_split, 2);
        add_method("String.meta.to_lower", string_to_lower, 0);
        add_method("String.meta.to_upper", string_to_upper, 0);
        add_method("String.meta.trim", string_trim, 0);
        add_method("String.meta.ltrim", string_ltrim, 0);
        add_method("String.meta.rtrim", string_rtrim, 0);
        add_method("String.meta.remove", string_remove, 1);
        add_method("String.meta.remove_first", string_remove_first, 1);
        add_method("String.meta.remove_last", string_remove_last, 1);
        add_method("String.meta.remove_at", string_remove_at, 2);
        add_method("String.meta.reverse", string_reverse, 0);
        add_method("String.meta.insert", string_insert, 2);

    }
    new_native_constructor(jsB_String, jsB_new_String, "String", 0); /* 1 */
    {
        add_method("String.from_char_code", string_from_codepoint, 0); /* 1 */
    }
    def_global("String", PHON_DONTENUM);
}

} // namespace phonometrica