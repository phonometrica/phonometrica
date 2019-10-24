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
#include <phon/runtime/object.hpp>

namespace phonometrica {

void Runtime::new_regex(const String &pattern, int flags)
{
    auto obj = new Object(*this, PHON_CREGEX, regex_meta);

    try
    {
        new (&obj->as.regex) Regex(pattern, flags);
    }
    catch (std::runtime_error &e)
    {
        throw raise("Syntax error", "regular expression: %s", e.what());
    }

//    opts = 0;
//    if (flags & PHON_REGEXP_I) opts |= REG_ICASE;
//    if (flags & PHON_REGEXP_M) opts |= REG_NEWLINE;

    push(obj);
}

static void regex_match(Runtime &rt)
{
    // TODO: add optional position for Regex.match()
    auto &re = rt.to_regex(0);
    auto str = rt.to_string(1);
    bool result = re.match(str);
    rt.push_boolean(result);
}

static void regex_group(Runtime &rt)
{
    auto &re = rt.to_regex(0);
    auto i = rt.to_integer(1);
	if (re.has_match())
	{
		rt.push(re.capture(i));
	}
	else
	{
		throw error("[Regex error]: Cannot get capture % because regex has no match", i);
	}
}

static void regex_has_match(Runtime& rt)
{
	auto &re = rt.to_regex(0);
	rt.push_boolean(re.has_match());
}

static void regex_first(Runtime &rt)
{
    auto &re = rt.to_regex(0);
    auto i = rt.to_integer(1);
    rt.push(re.capture_start(i));
}

static void regex_last(Runtime &rt)
{
    auto &re = rt.to_regex(0);
    auto i = rt.to_integer(1);
    rt.push(re.capture_end(i) - 1);
}

static void jsB_new_Regex(Runtime &rt)
{
    String pattern;
    int flags;

    if (rt.is_regex(1))
    {
        if (rt.is_defined(2))
            throw rt.raise("Type error", "cannot supply flags when creating one Regex from another");
        auto &old = rt.to_regex(1);
        pattern = old.pattern();
        flags = old.flags();
    }
    else if (rt.is_null(1))
    {
        pattern = "(?:)";
        flags = 0;
    }
    else
    {
        pattern = rt.to_string(1);
        flags = 0;
    }

    if (pattern.empty())
        pattern = "(?:)";

    if (rt.is_defined(2))
    {
        auto str = rt.to_string(2);
        auto s = str.data();
        int g = 0, i = 0, m = 0;
        while (*s)
        {
            if (*s == 'g') ++g;
            else if (*s == 'i') ++i;
            else if (*s == 'm') ++m;
            else throw rt.raise("Syntax error", "invalid regular expression flag: '%c'", *s);
            ++s;
        }
        if (g > 1) throw rt.raise("Syntax error", "invalid regular expression flag: 'g'");
        if (i > 1) throw rt.raise("Syntax error", "invalid regular expression flag: 'i'");
        if (m > 1) throw rt.raise("Syntax error", "invalid regular expression flag: 'm'");
        if (g) flags |= PHON_REGEXP_G;
        if (i) flags |= PHON_REGEXP_I;
        if (m) flags |= PHON_REGEXP_M;
    }

    rt.new_regex(pattern, flags);
}

static void jsB_Regex(Runtime &rt)
{
    if (rt.is_regex(1))
        return;
    jsB_new_Regex(rt);
}

static void regex_to_string(Runtime &rt)
{
    auto &re = rt.to_regex(0);
    rt.push(re.pattern());
}

void Runtime::init_regexp()
{
    push(regex_meta);
    {
        add_accessor("pattern", regex_to_string);
        add_method("Regex.meta.to_string", regex_to_string, 0);
        add_method("Regex.meta.match", regex_match, 0);
		add_method("Regex.meta.has_match", regex_has_match, 0);
        add_method("Regex.meta.group", regex_group, 1);
        add_method("Regex.meta.first", regex_first, 1);
        add_method("Regex.meta.last", regex_last, 1);

    }
    new_native_constructor(jsB_Regex, jsB_new_Regex, "Regex", 1);
    def_global("Regex", PHON_DONTENUM);
}

} // namespace phonometrica