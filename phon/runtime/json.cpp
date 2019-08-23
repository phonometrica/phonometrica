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
#include <phon/runtime/lex.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>
#include <phon/third_party/utf8/utf8.h>

namespace phonometrica {

static void jsonnext(Environment *J)
{
    J->lookahead = lex_json(J);
}

static int jsonaccept(Environment *J, int t)
{
    if (J->lookahead == t)
    {
        jsonnext(J);
        return 1;
    }
    return 0;
}

static void jsonexpect(Environment *J, int t)
{
    if (!jsonaccept(J, t))
        throw J->raise("Syntax error", "JSON: unexpected token: %s (expected %s)",
                       get_token_string(J->lookahead), get_token_string(t));
}

static void jsonvalue(Environment *J)
{
    int i;

    switch (J->lookahead)
    {
    case TK_STRING:
        J->push(J->text);
        jsonnext(J);
        break;

    case TK_NUMBER:
        J->push(J->number);
        jsonnext(J);
        break;

    case '{':
    {
        J->new_object();
        jsonnext(J);
        if (jsonaccept(J, '}'))
            return;
        String name;
        do
        {
            if (J->lookahead != TK_STRING)
                throw J->raise("Syntax error", "JSON: unexpected token: %s (expected string)",
                               get_token_string(J->lookahead));
            name = J->text;
            jsonnext(J);
            jsonexpect(J, ':');
            jsonvalue(J);
            J->set_field(-2, name);
        } while (jsonaccept(J, ','));
        jsonexpect(J, '}');
        break;
    }
    case '[':
        J->new_list();
        jsonnext(J);
        i = 0;
        if (jsonaccept(J, ']'))
            return;
        do
        {
            jsonvalue(J);
            js_setindex(J, -2, i++);
        } while (jsonaccept(J, ','));
        jsonexpect(J, ']');
        break;

    case TK_TRUE:
        J->push_boolean(true);
        jsonnext(J);
        break;

    case TK_FALSE:
        J->push_boolean(false);
        jsonnext(J);
        break;

    case TK_NULL:
        J->push_null();
        jsonnext(J);
        break;

    default:
        throw J->raise("Syntax error", "JSON: unexpected token: %s", get_token_string(J->lookahead));
    }
}

static void jsonrevive(Environment *J, const String &name)
{
    const char *key;
    char buf[32];

    /* revive is in 2 */
    /* holder is in -1 */

    J->get_field(-1, name); /* get value from holder */

    if (J->is_object(-1))
    {
        if (J->is_list(-1))
        {
            int i = 0;
            int n = js_getlength(J, -1);
            for (i = 0; i < n; ++i)
            {
                jsonrevive(J, int_to_str(buf, i));
                if (J->is_null(-1))
                {
                    J->pop(1);
                    J->del_field(-1, buf);
                }
                else
                {
                    J->set_field(-2, buf);
                }
            }
        }
        else
        {
            // TODO: reimplement jsonrevive()
            J->push_iterator(-1);
            std::optional<Variant> it;
            while ((it = J->next_iterator(-1)))
            {
                assert(it->is_string());
                auto s = it->as_string();
                js_rot2(J);
                jsonrevive(J, s);
                if (J->is_null(-1))
                {
                    J->pop(1);
                    J->del_field(-1, s);
                }
                else
                {
                    J->set_field(-2, s);
                }
                js_rot2(J);
            }
            J->pop(1);
        }
    }

    J->copy(2); /* reviver function */
    J->copy(-3); /* holder as this */
    J->push(name); /* name */
    J->copy(-4); /* value */
    J->call(2);
    js_rot2pop1(J); /* pop old value, leave new value on stack */
}

static void JSON_parse(Environment &env)
{
    auto source = env.to_string(1);
    init_lex(&env, "json", source);
    jsonnext(&env);

    if (env.is_callable(2))
    {
        env.new_object();
        jsonvalue(&env);
        env.def_field(-2, "", 0);
        jsonrevive(&env, "");
    }
    else
    {
        jsonvalue(&env);
    }
}

static void fmtnum(Environment *J, String &buffer, double n)
{
    if (isnan(n)) buffer.append(J->null_string);
    else if (isinf(n)) buffer.append(J->null_string);
    else if (n == 0) buffer.append('0');
    else
    {
        char buf[40];
        buffer.append(number_to_string(J, buf, n));
    }
}

static void fmtstr(Environment *J, String &buffer, const char *s)
{
    static const char *HEX = "0123456789ABCDEF";
    char32_t c;
    buffer.append('"');
    while (*s)
    {
        c = utf8::unchecked::next(s);
        switch (c)
        {
        case '"':
            buffer.append("\\\"");
            break;
        case '\\':
            buffer.append("\\\\");
            break;
        case '\b':
            buffer.append("\\b");
            break;
        case '\f':
            buffer.append("\\f");
            break;
        case '\n':
            buffer.append("\\n");
            break;
        case '\r':
            buffer.append("\\r");
            break;
        case '\t':
            buffer.append("\\t");
            break;
        default:
            if (c < ' ' || c > 127)
            {
                buffer.append("\\u");
                buffer.append(HEX[(c >> 12) & 15]);
                buffer.append(HEX[(c >> 8) & 15]);
                buffer.append(HEX[(c >> 4) & 15]);
                buffer.append(HEX[c & 15]);
            }
            else
            {
                buffer.append(c);
                break;
            }
        }
    }
    buffer.append('"');
}

static void fmtindent(Environment *J, String &buffer, std::string_view gap, int level)
{
    buffer.append('\n');
    while (level--)
        buffer.append(gap);
}

static int fmtvalue(Environment *J, String &buffer, const String &key, const char *gap, int level);

static void fmtobject(Environment *J, String &buffer, Object *obj, const char *gap, int level)
{
    int save;
    int i, n;

    n = J->top_count() - 1;
    for (i = 4; i < n; ++i)
        if (J->is_object(i))
            if (J->to_object(i) == J->to_object(-1))
                throw J->raise("Type error", "cyclic object value");

    n = 0;
    buffer.append('{');

    // Sort keys to have a consistent stringification.
    Array<String> keys;
    keys.reserve(obj->fields.size());

    for (auto &val : obj->fields)
    {
        keys.append(val.first);
    }
    std::sort(keys.begin(), keys.end());

    for (auto &s : keys)
    {
        save = (int) buffer.size();
        if (n) buffer.append(',');
        if (gap) fmtindent(J, buffer, gap, level + 1);
        fmtstr(J, buffer, s.data());
        buffer.append(':');
        if (gap)
            buffer.append(' ');
        J->get_field(-1, s);
        if (!fmtvalue(J, buffer, s, gap, level + 1))
            buffer.chop(save);
        else
            ++n;
    }
    if (gap && n) fmtindent(J, buffer, gap, level);
    buffer.append('}');
}

static void fmtlist(Environment *J, String &buffer, const char *gap, int level)
{
    intptr_t n, i;
    char buf[32];

    n = J->top_count() - 1;
    for (i = 4; i < n; ++i)
        if (J->is_object(i))
            if (J->to_object(i) == J->to_object(-1))
                throw J->raise("Type error", "cyclic object value");

    buffer.append('[');
    auto &lst = J->to_list(-1);
    n = lst.size();
    for (i = 1; i <= n; ++i)
    {
        if (i > 1) buffer.append(',');
        if (gap) fmtindent(J, buffer, gap, level + 1);
        J->push(lst[i]); // push field
        if (!fmtvalue(J, buffer, int_to_str(buf, i), gap, level + 1))
            buffer.append(J->null_string);
    }
    if (gap && n) fmtindent(J, buffer, gap, level);
    buffer.append(']');
}

static int fmtvalue(Environment *J, String &buffer, const String &key, const char *gap, int level)
{
    /* replacer is in 2 */
    /* holder is in -1 */

    // Move this out
//    J->get_field(-1, key);

    if (J->is_object(-1))
    {
        if (J->has_field(-1, "to_json"))
        {
            if (J->is_callable(-1))
            {
                J->copy(-2);
                J->push(key);
                J->call(1);
                js_rot2pop1(J);
            }
            else
            {
                J->pop(1);
            }
        }
    }

    if (J->is_callable(2))
    {
        J->copy(2); /* replacer function */
        J->copy(-3); /* holder as this */
        J->push(key); /* name */
        J->copy(-4); /* old value */
        J->call(2);
        js_rot2pop1(J); /* pop old value, leave new value on stack */
    }

    if (J->is_object(-1) && !J->is_callable(-1))
    {
        Object *obj = J->to_object(-1);
        switch (obj->type)
        {
        case PHON_CNUMBER:
            fmtnum(J, buffer, obj->as.number);
            break;
        case PHON_CSTRING:
            fmtstr(J, buffer, obj->as.string.data());
            break;
        case PHON_CBOOLEAN:
            buffer.append(obj->as.boolean ? J->true_string : J->false_string);
            break;
        case PHON_CLIST:
            fmtlist(J, buffer, gap, level);
            break;
        default:
            fmtobject(J, buffer, obj, gap, level);
            break;
        }
    }
    else if (J->is_boolean(-1))
        buffer.append(J->to_boolean(-1) ? J->true_string : J->false_string);
    else if (J->is_number(-1))
        fmtnum(J, buffer, J->to_number(-1));
    else if (J->is_string(-1))
        fmtstr(J, buffer, J->to_string(-1).data());
    else if (J->is_null(-1))
        buffer.append(J->null_string);
    else
    {
        J->pop(1);
        return 0;
    }

    J->pop(1);
    return 1;
}

static void JSON_stringify(Environment &env)
{
    char buf[12];
    const char *gap;
    intptr_t n;

    gap = nullptr;

    if (env.is_number(3))
    {
        n = env.to_integer(3);
        if (n < 0) n = 0;
        if (n > 10) n = 10;
        memset(buf, ' ', n);
        buf[n] = 0;
        if (n > 0) gap = buf;
    }
    else if (env.is_string(3))
    {
        auto s = env.to_string(3);
        n = s.size();
        if (n > 10) n = 10;
        memcpy(buf, s.data(), n);
        buf[n] = 0;
        if (n > 0) gap = buf;
    }

    env.new_object(); /* wrapper */
    env.copy(1);
    env.def_field(-2, String(), 0);
    String result;
    env.get_field(-1, String());
    if (!fmtvalue(&env, result, String(), gap, 0))
    {
        env.push_null();
    }
    else
    {
        env.push(std::move(result));
        js_rot2pop1(&env);
    }
}

void Environment::init_json()
{
    push(new Object(*this, PHON_CJSON, object_meta));
    {
        add_method("json.parse", JSON_parse, 2);
        add_method("json.stringify", JSON_stringify, 3);
    }
    def_global("json", PHON_DONTENUM);
}

} // namespace phonometrica