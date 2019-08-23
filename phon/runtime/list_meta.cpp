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

#include <algorithm>
#include <random>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

static bool is_sorted(const Array<Variant> &lst)
{
    return std::is_sorted(lst.begin(), lst.end());
}

int js_getlength(Environment *J, int idx)
{
    int len;
    J->get_field(idx, "length");
    len = J->to_integer(-1);
    J->pop(1);
    return len;
}

void js_setlength(Environment *J, int idx, int len)
{
    J->push(len);
    J->set_field(idx < 0 ? idx - 1 : idx, "length");
}

int js_hasindex(Environment *J, int idx, int i)
{
    char buf[32];
    return J->has_field(idx, int_to_str(buf, i));
}

void js_getindex(Environment *J, int idx, int i)
{
    char buf[32];
    J->get_field(idx, int_to_str(buf, i));
}

void js_setindex(Environment *J, int idx, int i)
{
    char buf[32];
    J->set_field(idx, int_to_str(buf, i));
}

void js_delindex(Environment *J, int idx, int i)
{
    char buf[32];
    J->del_field(idx, int_to_str(buf, i));
}

static void list_ctor(Environment &env)
{
    int top = env.top_count();

    env.new_list();
    auto &list = env.to_list(-1);

    for (int i = 1; i < top; ++i)
    {
        list.append(env.get(i));
    }
}

static void list_concat(Environment &env)
{
    int top = env.top_count();
    Array<Variant> result = env.to_list(-1);

    for (int i = 0; i < top; ++i)
    {
        auto &lst = env.to_list(i);

        for (auto &item : lst) {
            result.append(item);
        }
    }

    env.push(std::move(result));
}

static void list_join(Environment &env)
{
    String sep;
    String result;

    auto &lst = env.to_list(0);

    if (env.is_defined(1))
    {
        sep = env.to_string(1);
    }

    for (intptr_t i = 1; i <= lst.size(); i++)
    {
        auto s = var_to_string(&env, &lst[i]);
        result.append(s);
        if (i < lst.size()) result.append(sep);
    }

    env.push(std::move(result));
}

static void list_append(Environment &env)
{
    int top = env.top_count();
    auto &lst = env.to_list(0);
    auto size = lst.size();

    for (int i = 1; i < top; ++i)
    {
        lst.append(env.get(i));
    }

    env.push(size + 1);
}

static void list_reverse(Environment &env)
{
    auto &lst = env.to_list(0);
    std::reverse(lst.begin(), lst.end());
    env.copy(0);
}

static void list_sort(Environment &env)
{
    auto &lst = env.to_list(0);
    std::sort(lst.begin(), lst.end());
    env.copy(0);
}

static void list_is_sorted(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push_boolean(std::is_sorted(lst.begin(), lst.end()));
}

static void list_shift(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push(lst.take_first());
}

static void list_shuffle(Environment &env)
{
    auto &lst = env.to_list(0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lst.begin(), lst.end(), g);
}

static void list_sample(Environment &env)
{
    auto &lst = env.to_list(0);
    auto n = env.to_integer(1);
    Array<Variant> result;
    std::random_device rd;
    std::mt19937 g(rd());
    std::sample(lst.begin(), lst.end(), std::back_inserter(result), n, g);
    env.push(std::move(result));
}

static void list_pop(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push(lst.take_last());
}

static void Ap_slice(Environment &env)
{
    int len, s, e, n;
    double sv, ev;

    env.new_list();

    len = js_getlength(&env, 0);
    sv = env.to_integer(1);
    ev = env.is_defined(2) ? env.to_integer(2) : len;

    if (sv < 0) sv = sv + len;
    if (ev < 0) ev = ev + len;

    s = sv < 0 ? 0 : sv > len ? len : sv;
    e = ev < 0 ? 0 : ev > len ? len : ev;

    for (n = 0; s < e; ++s, ++n)
        if (js_hasindex(&env, 0, s))
            js_setindex(&env, -2, n);
}

struct sortslot
{
    Variant v;
    Environment *J;
};

static int sortcmp(const void *avoid, const void *bvoid)
{
    auto aslot = reinterpret_cast<const sortslot *>(avoid);
    auto bslot = reinterpret_cast<const sortslot *>(bvoid);
    const Variant *a = &aslot->v, *b = &bslot->v;
    Environment *J = aslot->J;
    int c;

    bool unx = a->is_null();
    bool uny = b->is_null();
    if (unx) return !uny;
    if (uny) return -1;

    if (J->is_callable(1))
    {
        J->copy(1); /* copy function */
        J->push_null();
        J->push(*a);
        J->push(*b);
        J->call(2);
        c = J->to_number(-1);
        J->pop(1);
    }
    else
    {
        J->push(*a);
        J->push(*b);
        auto sx = J->to_string(-2);
        auto sy = J->to_string(-1);
        c = sx.compare(sy);
        J->pop(2);
    }
    return c;
}

static void list_remove(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &value = env.get(1);
    lst.remove(value);
    env.push_null();
}

static void list_remove_at(Environment &env)
{
    auto &lst = env.to_list(0);
    auto pos = env.to_integer(1);
    lst.remove_at(pos);
    env.push_null();
}

static void list_prepend(Environment &env)
{
    int top = env.top_count();
    auto &lst = env.to_list(0);

    for (int i = top - 1; i > 0; --i)
    {
        lst.prepend(env.get(i));
    }

    env.copy(0);
}

static void list_to_string(Environment &env)
{
    int top = env.top_count();
    env.pop(top - 1);
    env.push(",");
    list_join(env);
}

static void list_find(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &value = env.get(1);
    auto from = env.is_defined(2) ? env.to_integer(2) : 1;
    intptr_t idx;

    try
    {
        idx = lst.find(value, from);
    }
    catch (std::runtime_error &e)
    {
        throw env.raise("Index error", e);
    }
    env.push(idx);
}

static void list_rfind(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &value = env.get(1);
    auto from = env.is_defined(2) ? env.to_integer(2) : -1;
    intptr_t idx;

    try
    {
        idx = lst.rfind(value, from);
    }
    catch (std::runtime_error &e)
    {
        throw env.raise("Index error", e);
    }
    env.push(idx);
}

static void list_sorted_find(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &value = env.get(1);
    auto it = std::lower_bound(lst.begin(), lst.end(), value);
    intptr_t pos = 0; // not found
    if (it != lst.end()) pos = (it - lst.begin()) + 1;
    env.push(pos);
}

static void list_contains(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &value = env.get(1);
    env.push_boolean(lst.contains(value));
}

static void list_every(Environment &env)
{
    auto &lst = env.to_list(0);

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        env.copy(1);
        env.push_null();
        env.push(v);
        env.call(1);
        if (!env.to_boolean(-1))
            return;
        env.pop(1);
    }

    env.push_boolean(true);
}

static void list_some(Environment &env)
{
    auto &lst = env.to_list(0);

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        env.copy(1);
        env.push_null();
        env.push(v);
        env.call(1);
        if (env.to_boolean(-1))
            return;
        env.pop(1);
    }

    env.push_boolean(false);
}

static void list_apply(Environment &env)
{
    auto &lst = env.to_list(0);

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        env.copy(1);
        env.push_null();
        env.push(v);
        env.call(1);
        env.pop(1);
    }

    env.push_null();
}

static void list_map(Environment &env)
{
    auto &lst = env.to_list(0);
    Array<Variant> result;

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        env.copy(1);
        env.push_null();
        env.push(v);
        env.call(1);
        result.append(std::move(env.get(-1)));
        env.pop(1);
    }

    env.push(std::move(result));
}

static void list_filter(Environment &env)
{
    auto &lst = env.to_list(0);
    Array<Variant> result;

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        env.copy(1);
        env.push_null();
        env.push(v);
        env.call(1);
        if (env.to_boolean(-1))
        {
            result.append(v);
        }
        env.pop(1);
    }

    env.push(std::move(result));
}

static void list_reduce(Environment &env)
{
    auto &lst = env.to_list(0);

    if (lst.empty())
        throw env.raise("Type error", "no initial value");

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    Variant result = lst.front();

    for (intptr_t i = 2; i <= lst.size(); i++)
    {
        env.copy(1);
        env.push_null();
        env.push(result);
        env.push(lst[i]);
        env.call(2);
        result = std::move(env.get(-1));
        env.pop(1);
    }

    env.push(std::move(result));
}

static void list_reduce_back(Environment &env)
{
    auto &lst = env.to_list(0);

    if (lst.empty())
        throw env.raise("Type error", "no initial value");

    if (!env.is_callable(1))
        throw env.raise("Type error", "callback is not a function");

    Variant result = lst.back();

    for (intptr_t i = lst.size() - 1; i > 0; i--)
    {
        env.copy(1);
        env.push_null();
        env.push(result);
        env.push(lst[i]);
        env.call(2);
        result = std::move(env.get(-1));
        env.pop(1);
    }

    env.push(std::move(result));

}

static void list_is_list(Environment &env)
{
    if (env.is_object(1))
    {
        Object *T = env.to_object(1);
        env.push_boolean(T->type == PHON_CLIST);
    }
    else
    {
        env.push_boolean(false);
    }
}

static void list_sorted_insert(Environment &env)
{
    auto &lst = env.to_list(0);
    auto &v = env.get(1);
    auto i = std::lower_bound(lst.begin(), lst.end(), v);
    if (i == lst.end() || v < *i)
        lst.insert(i, v);
}

static void list_insert(Environment &env)
{
    auto &lst = env.to_list(0);
    auto pos = env.to_integer(1);
    auto &v = env.get(2);
    lst.insert(pos, v);
}

static void list_copy(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push(lst);
}

static void list_union(Environment &env)
{
    auto &lst1 = env.to_list(0);
    auto &lst2 = env.to_list(1);
    Array<Variant> result;
    std::set_union(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    env.push(std::move(result));
}

static void list_intersect(Environment &env)
{
    auto &lst1 = env.to_list(0);
    auto &lst2 = env.to_list(1);
    Array<Variant> result;
    std::set_intersection(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    env.push(std::move(result));
}

static void list_difference(Environment &env)
{
    auto &lst1 = env.to_list(0);
    auto &lst2 = env.to_list(1);
    Array<Variant> result;
    std::set_difference(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    env.push(std::move(result));
}

static void list_includes(Environment &env)
{
    auto &lst1 = env.to_list(0);
    auto &lst2 = env.to_list(1);
    bool result = std::includes(lst1.begin(), lst1.end(), lst2.begin(), lst2.end());
    env.push_boolean(result);
}

static void list_get_length(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push(lst.size());
}

static void list_is_empty(Environment &env)
{
    auto &lst = env.to_list(0);
    env.push(lst.empty());
}

static void list_clear(Environment &env)
{
    auto &lst = env.to_list(0);
    lst.clear();
    env.push_null();
}

void Environment::init_list()
{
    push(list_meta);
    {
        add_accessor("List.meta.length", list_get_length);
        add_method("List.meta.is_empty", list_is_empty, 0);
        add_method("List.meta.clear", list_clear, 0);
        add_method("List.meta.to_string", list_to_string, 0);
        add_method("List.meta.concat", list_concat, 0); /* 1 */
        add_method("List.meta.join", list_join, 1);
        add_method("List.meta.append", list_append, 1);
        add_method("List.meta.reverse", list_reverse, 0);
        add_method("List.meta.sort", list_sort, 0);
        add_method("List.meta.is_sorted", list_is_sorted, 0);
        add_method("List.meta.shift", list_shift, 0);
        add_method("List.meta.pop", list_pop, 0);
        add_method("List.meta.slice", Ap_slice, 2);
        add_method("List.meta.remove", list_remove, 1);
        add_method("List.meta.remove_at", list_remove_at, 1);
        add_method("List.meta.prepend", list_prepend, 1);
        add_method("List.meta.find", list_find, 1);
        add_method("List.meta.find_back", list_rfind, 1);
        add_method("List.meta.contains", list_contains, 1);
        add_method("List.meta.every", list_every, 1);
        add_method("List.meta.some", list_some, 1);
        add_method("List.meta.apply", list_apply, 1);
        add_method("List.meta.map", list_map, 1);
        add_method("List.meta.filter", list_filter, 1);
        add_method("List.meta.reduce", list_reduce, 1);
        add_method("List.meta.reduce_back", list_reduce_back, 1);
        add_method("List.meta.shuffle", list_shuffle, 0);
        add_method("List.meta.sample", list_sample, 1);
        add_method("List.meta.copy", list_copy, 0);
        add_method("List.meta.insert", list_insert, 2);
        add_method("List.meta.sorted_insert", list_sorted_insert, 1);
        add_method("List.meta.sorted_find", list_sorted_find, 1);
        add_method("List.meta.unite", list_union, 1);
        add_method("List.meta.intersect", list_intersect, 1);
        add_method("List.meta.subtract", list_difference, 1);
        add_method("List.meta.includes", list_includes, 1);
    }
    new_native_constructor(list_ctor, list_ctor, "List", 0); /* 1 */
    {
        add_method("List.is_list", list_is_list, 1);
    }
    def_global("List", PHON_DONTENUM);
}

} // namespace phonometrica