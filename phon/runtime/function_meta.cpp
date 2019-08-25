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
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

static void jsB_Function(Runtime &rt)
{
    int top = rt.top_count();
    String body;
    Ast *parse;
    Function *fun;
    String buffer;

    /* p1, p2, ..., pn */
    if (top > 2)
    {
        for (int i = 1; i < top - 1; ++i)
        {
            if (i > 1) buffer.append(',');
            buffer.append(rt.to_string(i));
        }
        buffer.append(')');
    }

    /* body */
    if (rt.is_defined(top - 1)) body = rt.to_string(top - 1);

    parse = parse_function(&rt, "[string]", buffer.empty() ? nullptr : buffer.data(), body);
    fun = jsC_compilefunction(&rt, parse);
    free_parse(&rt);

    create_function(&rt, fun, rt.GE);
}

static void jsB_Function_prototype(Runtime &rt)
{
    rt.push_null();
}

static void Fp_toString(Runtime &rt)
{
    Object *self = rt.to_object(0);

    if (!rt.is_callable(0))
        throw rt.raise("Type error", "not a function");

    if (self->type == PHON_CFUNCTION || self->type == PHON_CSCRIPT)
    {
        Function *F = self->as.f.function;

        String msg("function ");
        msg.append(F->name);
        msg.append('(');
        for (int i = 0; i < F->numparams; ++i)
        {
            if (i > 0) msg.append(',');
            msg.append(F->vartab[i]);
        }
        msg.append(") { ... }");
        rt.push(std::move(msg));
    }
    else if (self->type == PHON_CCFUNCTION)
    {
        String msg("function ");
        msg.append(self->as.c.name);
        msg.append("() { ... }");
        rt.push(std::move(msg));
    }
    else
    {
        rt.push("function () { ... }");
    }
}

static void Fp_apply(Runtime &rt)
{
    int i, n;

    if (!rt.is_callable(0))
        throw rt.raise("Type error", "not a function");

    rt.copy(0);
    rt.copy(1);

    if (rt.is_null(2))
    {
        n = 0;
    }
    else
    {
        n = js_getlength(&rt, 2);
        for (i = 0; i < n; ++i)
            js_getindex(&rt, 2, i);
    }

    rt.call(n);
}

static void Fp_call(Runtime &rt)
{
    int i, top = rt.top_count();

    if (!rt.is_callable(0))
        throw rt.raise("Type error", "not a function");

    for (i = 0; i < top; ++i)
        rt.copy(i);

    rt.call(top - 2);
}

static void callbound(Runtime &rt)
{
    int top = rt.top_count();
    int i, fun, args, n;

    fun = rt.top_count();
    rt.current_function();
    rt.get_field(fun, "__TargetFunction__");
    rt.get_field(fun, "__BoundThis__");

    args = rt.top_count();
    rt.get_field(fun, "__BoundArguments__");
    n = js_getlength(&rt, args);
    for (i = 0; i < n; ++i)
        js_getindex(&rt, args, i);
    rt.remove(args);

    for (i = 1; i < top; ++i)
        rt.copy(i);

    rt.call(n + top - 1);
}

static void constructbound(Runtime &rt)
{
    int top = rt.top_count();
    int i, fun, args, n;

    fun = rt.top_count();
    rt.current_function();
    rt.get_field(fun, "__TargetFunction__");

    args = rt.top_count();
    rt.get_field(fun, "__BoundArguments__");
    n = js_getlength(&rt, args);
    for (i = 0; i < n; ++i)
        js_getindex(&rt, args, i);
    rt.remove(args);

    for (i = 1; i < top; ++i)
        rt.copy(i);

    rt.construct(n + top - 1);
}

static void Fp_bind(Runtime &rt)
{
    int i, top = rt.top_count();
    int n;

    if (!rt.is_callable(0))
        throw rt.raise("Type error", "not a function");

    n = js_getlength(&rt, 0);
    if (n > top - 2)
        n -= top - 2;
    else
        n = 0;

    /* Reuse target function's prototype for HasInstance check. */
    rt.get_field(0, "meta");
    rt.new_native_constructor(callbound, constructbound, "[bind]", n);

    /* target function */
    rt.copy(0);
    rt.def_field(-2, "__TargetFunction__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    /* bound this */
    rt.copy(1);
    rt.def_field(-2, "__BoundThis__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    /* bound arguments */
    rt.new_list();
    for (i = 2; i < top; ++i)
    {
        rt.copy(i);
        js_setindex(&rt, -2, i - 2);
    }
    rt.def_field(-2, "__BoundArguments__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
}

void Runtime::init_function()
{
    function_meta->as.c.name = "Function.prototype";
    function_meta->as.c.function = jsB_Function_prototype;
    function_meta->as.c.constructor = nullptr;
    function_meta->as.c.length = 0;

    push(function_meta);
    {
        add_method("Function.meta.to_string", Fp_toString, 2);
        add_method("Function.meta.apply", Fp_apply, 2);
        add_method("Function.meta.call", Fp_call, 1);
        add_method("Function.meta.bind", Fp_bind, 1);
    }
    new_native_constructor(jsB_Function, jsB_Function, "Function", 1);
    def_global("Function", PHON_DONTENUM);
}

} // namespace phonometrica
