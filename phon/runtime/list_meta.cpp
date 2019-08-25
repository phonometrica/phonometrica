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
#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

static bool is_sorted(const Array<Variant> &lst)
{
    return std::is_sorted(lst.begin(), lst.end());
}

int js_getlength(Runtime *J, int idx)
{
    int len;
    J->get_field(idx, "length");
    len = J->to_integer(-1);
    J->pop(1);
    return len;
}

void js_setlength(Runtime *J, int idx, int len)
{
    J->push(len);
    J->set_field(idx < 0 ? idx - 1 : idx, "length");
}

int js_hasindex(Runtime *J, int idx, int i)
{
    char buf[32];
    return J->has_field(idx, int_to_str(buf, i));
}

void js_getindex(Runtime *J, int idx, int i)
{
    char buf[32];
    J->get_field(idx, int_to_str(buf, i));
}

void js_setindex(Runtime *J, int idx, int i)
{
    char buf[32];
    J->set_field(idx, int_to_str(buf, i));
}

void js_delindex(Runtime *J, int idx, int i)
{
    char buf[32];
    J->del_field(idx, int_to_str(buf, i));
}

static void list_ctor(Runtime &rt)
{
    int top = rt.top_count();

    rt.new_list();
    auto &list = rt.to_list(-1);

    for (int i = 1; i < top; ++i)
    {
        list.append(rt.get(i));
    }
}

static void list_concat(Runtime &rt)
{
    int top = rt.top_count();
    Array<Variant> result = rt.to_list(-1);

    for (int i = 0; i < top; ++i)
    {
        auto &lst = rt.to_list(i);

        for (auto &item : lst) {
            result.append(item);
        }
    }

    rt.push(std::move(result));
}

static void list_join(Runtime &rt)
{
    String sep;
    String result;

    auto &lst = rt.to_list(0);

    if (rt.is_defined(1))
    {
        sep = rt.to_string(1);
    }

    for (intptr_t i = 1; i <= lst.size(); i++)
    {
        auto s = var_to_string(&rt, &lst[i]);
        result.append(s);
        if (i < lst.size()) result.append(sep);
    }

    rt.push(std::move(result));
}

static void list_append(Runtime &rt)
{
    int top = rt.top_count();
    auto &lst = rt.to_list(0);
    auto size = lst.size();

    for (int i = 1; i < top; ++i)
    {
        lst.append(rt.get(i));
    }

    rt.push(size + 1);
}

static void list_reverse(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    std::reverse(lst.begin(), lst.end());
    rt.copy(0);
}

static void list_sort(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    std::sort(lst.begin(), lst.end());
    rt.copy(0);
}

static void list_is_sorted(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push_boolean(std::is_sorted(lst.begin(), lst.end()));
}

static void list_shift(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push(lst.take_first());
}

static void list_shuffle(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lst.begin(), lst.end(), g);
}

static void list_sample(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto n = rt.to_integer(1);
    Array<Variant> result;
    std::random_device rd;
    std::mt19937 g(rd());
    std::sample(lst.begin(), lst.end(), std::back_inserter(result), n, g);
    rt.push(std::move(result));
}

static void list_pop(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push(lst.take_last());
}

static void Ap_slice(Runtime &rt)
{
    int len, s, e, n;
    double sv, ev;

    rt.new_list();

    len = js_getlength(&rt, 0);
    sv = rt.to_integer(1);
    ev = rt.is_defined(2) ? rt.to_integer(2) : len;

    if (sv < 0) sv = sv + len;
    if (ev < 0) ev = ev + len;

    s = sv < 0 ? 0 : sv > len ? len : sv;
    e = ev < 0 ? 0 : ev > len ? len : ev;

    for (n = 0; s < e; ++s, ++n)
        if (js_hasindex(&rt, 0, s))
            js_setindex(&rt, -2, n);
}

struct sortslot
{
    Variant v;
    Runtime *J;
};

static int sortcmp(const void *avoid, const void *bvoid)
{
    auto aslot = reinterpret_cast<const sortslot *>(avoid);
    auto bslot = reinterpret_cast<const sortslot *>(bvoid);
    const Variant *a = &aslot->v, *b = &bslot->v;
    Runtime *J = aslot->J;
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

static void list_remove(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &value = rt.get(1);
    lst.remove(value);
    rt.push_null();
}

static void list_remove_at(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto pos = rt.to_integer(1);
    lst.remove_at(pos);
    rt.push_null();
}

static void list_prepend(Runtime &rt)
{
    int top = rt.top_count();
    auto &lst = rt.to_list(0);

    for (int i = top - 1; i > 0; --i)
    {
        lst.prepend(rt.get(i));
    }

    rt.copy(0);
}

static void list_to_string(Runtime &rt)
{
    int top = rt.top_count();
    rt.pop(top - 1);
    rt.push(",");
    list_join(rt);
}

static void list_find(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &value = rt.get(1);
    auto from = rt.is_defined(2) ? rt.to_integer(2) : 1;
    intptr_t idx;

    try
    {
        idx = lst.find(value, from);
    }
    catch (std::runtime_error &e)
    {
        throw rt.raise("Index error", e);
    }
    rt.push(idx);
}

static void list_rfind(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &value = rt.get(1);
    auto from = rt.is_defined(2) ? rt.to_integer(2) : -1;
    intptr_t idx;

    try
    {
        idx = lst.rfind(value, from);
    }
    catch (std::runtime_error &e)
    {
        throw rt.raise("Index error", e);
    }
    rt.push(idx);
}

static void list_sorted_find(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &value = rt.get(1);
    auto it = std::lower_bound(lst.begin(), lst.end(), value);
    intptr_t pos = 0; // not found
    if (it != lst.end()) pos = (it - lst.begin()) + 1;
    rt.push(pos);
}

static void list_contains(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &value = rt.get(1);
    rt.push_boolean(lst.contains(value));
}

static void list_every(Runtime &rt)
{
    auto &lst = rt.to_list(0);

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(v);
        rt.call(1);
        if (!rt.to_boolean(-1))
            return;
        rt.pop(1);
    }

    rt.push_boolean(true);
}

static void list_some(Runtime &rt)
{
    auto &lst = rt.to_list(0);

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(v);
        rt.call(1);
        if (rt.to_boolean(-1))
            return;
        rt.pop(1);
    }

    rt.push_boolean(false);
}

static void list_apply(Runtime &rt)
{
    auto &lst = rt.to_list(0);

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(v);
        rt.call(1);
        rt.pop(1);
    }

    rt.push_null();
}

static void list_map(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    Array<Variant> result;

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(v);
        rt.call(1);
        result.append(std::move(rt.get(-1)));
        rt.pop(1);
    }

    rt.push(std::move(result));
}

static void list_filter(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    Array<Variant> result;

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    for (auto &v : lst)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(v);
        rt.call(1);
        if (rt.to_boolean(-1))
        {
            result.append(v);
        }
        rt.pop(1);
    }

    rt.push(std::move(result));
}

static void list_reduce(Runtime &rt)
{
    auto &lst = rt.to_list(0);

    if (lst.empty())
        throw rt.raise("Type error", "no initial value");

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    Variant result = lst.front();

    for (intptr_t i = 2; i <= lst.size(); i++)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(result);
        rt.push(lst[i]);
        rt.call(2);
        result = std::move(rt.get(-1));
        rt.pop(1);
    }

    rt.push(std::move(result));
}

static void list_reduce_back(Runtime &rt)
{
    auto &lst = rt.to_list(0);

    if (lst.empty())
        throw rt.raise("Type error", "no initial value");

    if (!rt.is_callable(1))
        throw rt.raise("Type error", "callback is not a function");

    Variant result = lst.back();

    for (intptr_t i = lst.size() - 1; i > 0; i--)
    {
        rt.copy(1);
        rt.push_null();
        rt.push(result);
        rt.push(lst[i]);
        rt.call(2);
        result = std::move(rt.get(-1));
        rt.pop(1);
    }

    rt.push(std::move(result));

}

static void list_is_list(Runtime &rt)
{
    if (rt.is_object(1))
    {
        Object *T = rt.to_object(1);
        rt.push_boolean(T->type == PHON_CLIST);
    }
    else
    {
        rt.push_boolean(false);
    }
}

static void list_sorted_insert(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto &v = rt.get(1);
    auto i = std::lower_bound(lst.begin(), lst.end(), v);
    if (i == lst.end() || v < *i)
        lst.insert(i, v);
}

static void list_insert(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    auto pos = rt.to_integer(1);
    auto &v = rt.get(2);
    lst.insert(pos, v);
}

static void list_copy(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push(lst);
}

static void list_union(Runtime &rt)
{
    auto &lst1 = rt.to_list(0);
    auto &lst2 = rt.to_list(1);
    Array<Variant> result;
    std::set_union(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    rt.push(std::move(result));
}

static void list_intersect(Runtime &rt)
{
    auto &lst1 = rt.to_list(0);
    auto &lst2 = rt.to_list(1);
    Array<Variant> result;
    std::set_intersection(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    rt.push(std::move(result));
}

static void list_difference(Runtime &rt)
{
    auto &lst1 = rt.to_list(0);
    auto &lst2 = rt.to_list(1);
    Array<Variant> result;
    std::set_difference(lst1.begin(), lst1.end(), lst2.begin(), lst2.end(), std::back_inserter(result));
    rt.push(std::move(result));
}

static void list_includes(Runtime &rt)
{
    auto &lst1 = rt.to_list(0);
    auto &lst2 = rt.to_list(1);
    bool result = std::includes(lst1.begin(), lst1.end(), lst2.begin(), lst2.end());
    rt.push_boolean(result);
}

static void list_get_length(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push(lst.size());
}

static void list_is_empty(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    rt.push(lst.empty());
}

static void list_clear(Runtime &rt)
{
    auto &lst = rt.to_list(0);
    lst.clear();
    rt.push_null();
}

void Runtime::init_list()
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