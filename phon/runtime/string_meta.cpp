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
#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

static String checkstring(Runtime *J, int idx)
{
    if (!J->is_coercible(idx))
        throw J->raise("Type error", "string function called on null or undefined");
    return J->to_string(idx);
}

static void jsB_new_String(Runtime &rt)
{
    rt.new_string(rt.top_count() > 1 ? rt.to_string(1) : String());
}

static void jsB_String(Runtime &rt)
{
    rt.push(rt.top_count() > 1 ? rt.to_string(1) : String());
}

static void string_to_string(Runtime &rt)
{
    Object *self = rt.to_object(0);
    if (self->type != PHON_CSTRING) throw rt.raise("Type error", "not a string");
    rt.push(self->as.string);
}

static void string_to_value(Runtime &rt)
{
    Object *self = rt.to_object(0);
    if (self->type != PHON_CSTRING) throw rt.raise("Type error", "not a string");
    rt.push(self->as.string);
}

static void string_char_at(Runtime &rt)
{
    auto s = checkstring(&rt, 0);
    auto pos = rt.to_integer(1);

    try
    {
	    String grapheme = s.next_grapheme(pos);
	    rt.push(std::move(grapheme));
    }
    catch (std::exception &e)
    {
        throw rt.raise("Index error", e);
    }
}

static void string_concat(Runtime &rt)
{
    auto top = rt.top_count();
    int n;

    if (top == 1)
        return;

    String result = checkstring(&rt, 0);

    for (int i = 1; i < top; ++i)
    {
        auto s = rt.to_string(i);
        result.append(s);
    }

    rt.push(std::move(result));
}

static void string_find(Runtime &rt)
{
    auto haystack = checkstring(&rt, 0);
    auto needle = rt.to_string(1);
    auto index = haystack.find(needle);
    rt.push(index);
}

static void string_rfind(Runtime &rt)
{
    auto haystack = checkstring(&rt, 0);
    auto needle = rt.to_string(1);
    auto index = haystack.rfind(needle);
    rt.push(index);
}

static void string_compare(Runtime &rt)
{
    auto a = checkstring(&rt, 0);
    auto b = rt.to_string(1);
    rt.push(a.compare(b));
}

static void string_mid(Runtime &rt)
{
    auto str = checkstring(&rt, 0);
    auto from = rt.to_integer(1);
    auto count = rt.is_defined(2) ? rt.to_integer(2) : -1;

    try
    {
        String result = str.mid(from, count);
        rt.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw rt.raise("Index error", e);
    }
}

static void string_left(Runtime &rt)
{
    auto str = checkstring(&rt, 0);
    auto count = rt.to_integer(1);

    try
    {
        auto result = str.left(count);
        rt.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw rt.raise("Index error", e);
    }
}

static void string_right(Runtime &rt)
{
    auto str = checkstring(&rt, 0);
    auto count = rt.to_integer(1);

    try
    {
        auto result = str.right(count);
        rt.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw rt.raise("Index error", e);
    }
}

static void string_to_lower(Runtime &rt)
{
    auto src = checkstring(&rt, 0);
    rt.push(src.to_lower());
}

static void string_starts_with(Runtime &rt)
{
    auto self = checkstring(&rt, 0);
    auto prefix = rt.to_string(1);
    rt.push_boolean(self.starts_with(prefix));
}

static void string_ends_with(Runtime &rt)
{
    auto self = checkstring(&rt, 0);
    auto suffix = rt.to_string(1);
    rt.push_boolean(self.ends_with(suffix));
}

static void string_contains(Runtime &rt)
{
    auto self = checkstring(&rt, 0);
    auto infix = rt.to_string(1);
    rt.push_boolean(self.contains(infix));
}

static void string_count(Runtime &rt)
{
    auto self = checkstring(&rt, 0);
    auto substring = rt.to_string(1);
    rt.push(self.count(substring));
}

static void string_to_upper(Runtime &rt)
{
    auto src = checkstring(&rt, 0);
    rt.push(src.to_upper());
}

static int istrim(int c)
{
    return c == 0x9 || c == 0xB || c == 0xC || c == 0x20 || c == 0xA0 || c == 0xFEFF ||
           c == 0xA || c == 0xD || c == 0x2028 || c == 0x2029;
}

static void string_trim(Runtime &rt)
{
    auto s = checkstring(&rt, 0);
    rt.push(s.trim());
}

static void string_ltrim(Runtime &rt)
{
    auto s = checkstring(&rt, 0);
    rt.push(s.ltrim());
}

static void string_rtrim(Runtime &rt)
{
    auto s = checkstring(&rt, 0);
    rt.push(s.rtrim());
}

static void string_from_codepoint(Runtime &rt)
{
    auto top = rt.top_count();
    String result;

    for (int i = 1; i < top; ++i)
    {
        char32_t c = rt.to_uint32(i);
        result.append(String::encode(c).data);
    }

    rt.push(std::move(result));
}

static void string_replace(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto before = rt.to_string(1);
    auto after = rt.to_string(2);
    rt.push(source.replace(before, after));
}

static void string_replace_first(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto before = rt.to_string(1);
    auto after = rt.to_string(2);
    rt.push(source.replace_first(before, after));
}

static void string_replace_last(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto before = rt.to_string(1);
    auto after = rt.to_string(2);
    rt.push(source.replace_last(before, after));
}

static void string_replace_at(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto pos = rt.to_integer(1);
    auto count = rt.to_integer(2);
    auto after = rt.to_string(3);
    rt.push(source.replace(pos, count, after));
}

static void string_remove(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto substring = rt.to_string(1);
    rt.push(source.remove(substring));
}

static void string_remove_first(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto substring = rt.to_string(1);
    rt.push(source.remove_first(substring));
}

static void string_remove_last(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto substring = rt.to_string(1);
    rt.push(source.remove_last(substring));
}

static void string_remove_at(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto pos = rt.to_integer(1);
    auto count = rt.to_integer(2);
    rt.push(source.remove(pos, count));
}

static void string_insert(Runtime &rt)
{
    auto source = checkstring(&rt, 0);
    auto pos = rt.to_integer(1);
    auto s = rt.to_string(2);
    rt.push(source.insert(pos, s));
}

static void string_reverse(Runtime &rt)
{
    auto self = checkstring(&rt, 0);
    rt.push(self.reverse());
}

static void split_with_regex(Runtime &rt)
{
    throw error("split regex not implemented");
#if 0
    js_Regexp *re;
    int limit, len, k;
    const char *p, *a, *b, *c, *e;
    Resub m;

    auto &text = checkstring(&runtime, 0);
    re = js_toregexp(&runtime, 1);
    limit = runtime.is_defined(2) ? runtime.to_integer(2) : 1 << 30;

    runtime.new_list();
    len = 0;

    e = text.end();

    /* splitting the empty string */
    if (e == text)
    {
        if (js_regexec((Reprog *) re->prog, text, &m, 0))
        {
            if (len == limit) return;
            runtime.push_literal("");
            js_setindex(&runtime, -2, 0);
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
        runtime.push_lstring(p, b - p);
        js_setindex(&runtime, -2, len++);

        for (k = 1; k < m.nsub; ++k)
        {
            if (len == limit) return;
            runtime.push_lstring(m.sub[k].sp, m.sub[k].ep - m.sub[k].sp);
            js_setindex(&runtime, -2, len++);
        }

        a = p = c;
    }

    if (len == limit) return;
    runtime.push_string(p);
    js_setindex(&runtime, -2, len);
#endif
}

static void split_with_string(Runtime &rt)
{
    auto str = checkstring(&rt, 0);
    auto sep = rt.to_string(1);
    auto pieces = str.split(sep);
    rt.new_list();

    for (int i = 1; i <= pieces.size(); i++)
    {
        rt.push(std::move(pieces[i]));
        js_setindex(&rt, -2, i-1); // TODO: changes indices to base 1 in split() and everywhere
    }
}

static void string_split(Runtime &rt)
{
    if (rt.is_null(1))
    {
        rt.new_list();
        rt.copy(0);
        js_setindex(&rt, -2, 0);
    }
    else if (rt.is_regex(1))
    {
        split_with_regex(rt);
    }
    else
    {
        split_with_string(rt);
    }
}

static void string_is_empty(Runtime &rt)
{
    auto s = checkstring(&rt, 0);
    rt.push_boolean(s.empty());
}

void Runtime::init_string()
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