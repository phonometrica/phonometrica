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
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>
#include "object.hpp"

namespace phonometrica {

static void jsB_Function(Environment &env)
{
    int top = env.top_count();
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
            buffer.append(env.to_string(i));
        }
        buffer.append(')');
    }

    /* body */
    if (env.is_defined(top - 1)) body = env.to_string(top - 1);

    parse = parse_function(&env, "[string]", buffer.empty() ? nullptr : buffer.data(), body);
    fun = jsC_compilefunction(&env, parse);
    free_parse(&env);

    create_function(&env, fun, env.GE);
}

static void jsB_Function_prototype(Environment &env)
{
    env.push_null();
}

static void Fp_toString(Environment &env)
{
    Object *self = env.to_object(0);

    if (!env.is_callable(0))
        throw env.raise("Type error", "not a function");

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
        env.push(std::move(msg));
    }
    else if (self->type == PHON_CCFUNCTION)
    {
        String msg("function ");
        msg.append(self->as.c.name);
        msg.append("() { ... }");
        env.push(std::move(msg));
    }
    else
    {
        env.push("function () { ... }");
    }
}

static void Fp_apply(Environment &env)
{
    int i, n;

    if (!env.is_callable(0))
        throw env.raise("Type error", "not a function");

    env.copy(0);
    env.copy(1);

    if (env.is_null(2))
    {
        n = 0;
    }
    else
    {
        n = js_getlength(&env, 2);
        for (i = 0; i < n; ++i)
            js_getindex(&env, 2, i);
    }

    env.call(n);
}

static void Fp_call(Environment &env)
{
    int i, top = env.top_count();

    if (!env.is_callable(0))
        throw env.raise("Type error", "not a function");

    for (i = 0; i < top; ++i)
        env.copy(i);

    env.call(top - 2);
}

static void callbound(Environment &env)
{
    int top = env.top_count();
    int i, fun, args, n;

    fun = env.top_count();
    env.current_function();
    env.get_field(fun, "__TargetFunction__");
    env.get_field(fun, "__BoundThis__");

    args = env.top_count();
    env.get_field(fun, "__BoundArguments__");
    n = js_getlength(&env, args);
    for (i = 0; i < n; ++i)
        js_getindex(&env, args, i);
    env.remove(args);

    for (i = 1; i < top; ++i)
        env.copy(i);

    env.call(n + top - 1);
}

static void constructbound(Environment &env)
{
    int top = env.top_count();
    int i, fun, args, n;

    fun = env.top_count();
    env.current_function();
    env.get_field(fun, "__TargetFunction__");

    args = env.top_count();
    env.get_field(fun, "__BoundArguments__");
    n = js_getlength(&env, args);
    for (i = 0; i < n; ++i)
        js_getindex(&env, args, i);
    env.remove(args);

    for (i = 1; i < top; ++i)
        env.copy(i);

    env.construct(n + top - 1);
}

static void Fp_bind(Environment &env)
{
    int i, top = env.top_count();
    int n;

    if (!env.is_callable(0))
        throw env.raise("Type error", "not a function");

    n = js_getlength(&env, 0);
    if (n > top - 2)
        n -= top - 2;
    else
        n = 0;

    /* Reuse target function's prototype for HasInstance check. */
    env.get_field(0, "meta");
    env.new_native_constructor(callbound, constructbound, "[bind]", n);

    /* target function */
    env.copy(0);
    env.def_field(-2, "__TargetFunction__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    /* bound this */
    env.copy(1);
    env.def_field(-2, "__BoundThis__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);

    /* bound arguments */
    env.new_list();
    for (i = 2; i < top; ++i)
    {
        env.copy(i);
        js_setindex(&env, -2, i - 2);
    }
    env.def_field(-2, "__BoundArguments__", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
}

void Environment::init_function()
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
