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

#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

void Environment::new_regex(const String &pattern, int flags)
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

static void regex_match(Environment &env)
{
    // TODO: add optional position for Regex.match()
    auto &re = to_regexp(&env, 0);
    auto str = env.to_string(1);
    bool result = re.match(str);
    env.push_boolean(result);
}

static void regex_group(Environment &env)
{
    auto &re = to_regexp(&env, 0);
    auto i = env.to_integer(1);
    env.push(re.capture(i));
}

static void regex_first(Environment &env)
{
    auto &re = to_regexp(&env, 0);
    auto i = env.to_integer(1);
    env.push(re.capture_start(i));
}

static void regex_last(Environment &env)
{
    auto &re = to_regexp(&env, 0);
    auto i = env.to_integer(1);
    env.push(re.capture_end(i) - 1);
}

static void jsB_new_Regex(Environment &env)
{
    String pattern;
    int flags;

    if (env.is_regex(1))
    {
        if (env.is_defined(2))
            throw env.raise("Type error", "cannot supply flags when creating one Regex from another");
        auto &old = to_regexp(&env, 1);
        pattern = old.pattern();
        flags = old.flags();
    }
    else if (env.is_null(1))
    {
        pattern = "(?:)";
        flags = 0;
    }
    else
    {
        pattern = env.to_string(1);
        flags = 0;
    }

    if (pattern.empty())
        pattern = "(?:)";

    if (env.is_defined(2))
    {
        auto str = env.to_string(2);
        auto s = str.data();
        int g = 0, i = 0, m = 0;
        while (*s)
        {
            if (*s == 'g') ++g;
            else if (*s == 'i') ++i;
            else if (*s == 'm') ++m;
            else throw env.raise("Syntax error", "invalid regular expression flag: '%c'", *s);
            ++s;
        }
        if (g > 1) throw env.raise("Syntax error", "invalid regular expression flag: 'g'");
        if (i > 1) throw env.raise("Syntax error", "invalid regular expression flag: 'i'");
        if (m > 1) throw env.raise("Syntax error", "invalid regular expression flag: 'm'");
        if (g) flags |= PHON_REGEXP_G;
        if (i) flags |= PHON_REGEXP_I;
        if (m) flags |= PHON_REGEXP_M;
    }

    env.new_regex(pattern, flags);
}

static void jsB_Regex(Environment &env)
{
    if (env.is_regex(1))
        return;
    jsB_new_Regex(env);
}

static void regex_to_string(Environment &env)
{
    auto &re = to_regexp(&env, 0);
    env.push(re.pattern());
}

static void regex_get_length(Environment &env)
{
    auto &re = to_regexp(&env, 0);
    env.push(re.count());
}

void Environment::init_regexp()
{
    push(regex_meta);
    {
        add_accessor("Regex.meta.length", regex_get_length);
        add_accessor("Regex.meta.pattern", regex_to_string);
        add_method("Regex.meta.to_string", regex_to_string, 0);
        add_method("Regex.meta.match", regex_match, 0);
        add_method("Regex.meta.group", regex_group, 1);
        add_method("Regex.meta.first", regex_first, 1);
        add_method("Regex.meta.last", regex_last, 1);

    }
    new_native_constructor(jsB_Regex, jsB_new_Regex, "Regex", 1);
    def_global("Regex", PHON_DONTENUM);
}

} // namespace phonometrica