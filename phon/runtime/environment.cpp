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

#include <cassert>
#include <cstdarg>
#include <cerrno>
#include <phon/file.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/run.hpp>
#include <phon/runtime/toplevel.hpp>
#include <phon/utils/print.hpp>
#include <phon/utils/alloc.hpp>

namespace phonometrica {

static void *default_alloc(void *actx, void *ptr, int size)
{
    if (size == 0)
    {
        free(ptr);
        return nullptr;
    }
    return realloc(ptr, (size_t) size);
}

static void default_report(Environment *, const String &message)
{
    utils::print(stderr, message);
    utils::print(stderr, "\n");
}

static void default_print(const String &s)
{
    utils::printf(s);
}

static void default_panic(Environment *J)
{
    J->report("uncaught exception");
    /* return to javascript to abort */
}

static void js_loadstringx(Environment *J, const String &filename, const String &source, bool iseval)
{
    Ast *P;
    Function *F;

    try
    {
        P = parse(J, filename, source);
        F = jsC_compilescript(J, P, iseval ? J->strict : J->default_strict);
        free_parse(J);
        create_script(J, F, iseval ? (J->strict ? J->E : nullptr) : J->GE);
    }
    catch (std::exception &e)
    {
        free_parse(J);
        throw;
    }
}

void load_eval(Environment *J, const String &filename, const String &source)
{
    js_loadstringx(J, filename, source, 1);
}

Environment::Environment() :
    null_string("null"), undef_string("undefined"), true_string("true"), false_string("false")
{
    auto alloc = default_alloc;
    this->actx = nullptr;
    this->alloc = alloc;
    this->print = default_print;

    this->trace[0].name = "-top-";
    this->trace[0].file = "native";
    this->trace[0].line = 0;

    this->report_callback = default_report;
    this->panic = default_panic;

    this->stack = utils::allocate<Variant>(PHON_STACKSIZE);
    this->top = this->bot = this->stack;

    if (!this->stack)
    {
        alloc(nullptr, nullptr, 0);
        return;
    }

    this->gcmark = 1;
    this->nextref = 0;

    this->R = new Object(*this, PHON_COBJECT, nullptr);
    this->G = new Object(*this, PHON_COBJECT, nullptr);
    this->E = new_namespace(this, this->G, nullptr);
    this->GE = this->E;

    initialize();
}

void Environment::set_report_callback(report_callback_t report)
{
    this->report_callback = report;
}

void Environment::report(const String &message)
{
    if (report_callback)
        report_callback(this, message);
}

panic_callback_t Environment::at_panic(panic_callback_t panic)
{
    auto old = this->panic;
    this->panic = panic;

    return old;
}

int Environment::do_string(const String &source)
{

    this->load_string("[string]", source);
    push_null();
    call(0);
    pop(1);

    return 0;
}

int Environment::do_file(const String &filename)
{
    load_file(filename);
    push_null();
    call(0);
    pop(1);

    return 0;
}

int Environment::pcall(int n)
{
    // FIXME: it seems we need to save the bottom in addition to the top
    //  in order to properly restore the stack.
    auto savetop = this->top - n - 2;
    auto save_bot = bot;
    try
    {
        call(n);
        return 0;
    }
    catch (std::exception &)
    {
        /* clean up the stack to only hold the error object */
        *savetop = std::move(*(--top));
        bot = save_bot;

        while (top > savetop + 1)
        {
            top--;
            top->~Variant();
        }
//        this->top = savetop + 1;

        throw;
    }
}

int Environment::pconstruct(int n)
{
    auto savetop = this->top - n - 2;
    auto save_bot = bot;
    try
    {
        construct(n);
        return 0;
    }
    catch (std::exception &)
    {
        /* clean up the stack to only hold the error object */
        *savetop = std::move(*(--top));
        bot = save_bot;

        while (top > savetop + 1)
        {
            top--;
            top->~Variant();
        }
//        this->top = savetop + 1;
        throw;
    }
}

void Environment::pop(int n)
{
    if (unlikely(top - n < bot))
    {
        // Clean up variants on the stack before throwing.
        while (top-- != bot)
        {
            top->~Variant();
        }

        throw std::runtime_error("[Internal error] stack underflow!");
    }

    while (n-- > 0)
    {
        (--top)->~Variant();
    }
}

void Environment::load_string(const String &filename, const String &source)
{
    js_loadstringx(this, filename, source, 0);
}

void Environment::load_file(const String &filename)
{
    String content;

    try
    {
        content = File::read_all(filename);
    }
    catch (std::exception &e)
    {
        throw raise("Input/Output error", e);
    }

    load_string(filename, content);
}

void Environment::eval()
{
    if (!is_string(-1))
        return;
    load_eval(this, "(eval)", to_string(-1));
    js_rot2pop1(this);
    copy(0); /* copy 'this' */
    call(0);
}

String Environment::internalize(const String &s)
{
    auto it = strings.insert(s);
    return *it.first;
}

std::runtime_error Environment::raise(const char *error_category, std::exception &e)
{
    return raise(error_category, "%s", e.what());
}

std::runtime_error Environment::raise(const char *error_category, const char *fmt, ...)
{
    va_list ap;
    char buf[256];
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);

    if (is_stack_empty())
    {
        return error("[%] %", error_category, buf);
    }
    else
    {
        get_stack_trace(0);
        auto trace = to_string(-1);
        pop(1);

        return  error("[%] %\n%", error_category, buf, trace);
    }
}

double Environment::to_number(Variant *v)
{
    switch (v->type)
    {
    default:
    case PHON_TNULL:
        return std::nan("");
    case PHON_TBOOLEAN:
        return v->as.boolean;
    case PHON_TNUMBER:
        return v->as.number;
    case PHON_TSTRING:
        return string_to_number(this, v->as.string);
    case PHON_TOBJECT:
        var_to_primitive(this, v, PHON_HINT_NUMBER);
        return to_number(v);
    }
}

intptr_t Environment::to_integer(Variant *v)
{
    return number_to_integer(to_number(v));
}

File &Environment::to_file(int idx)
{
    Variant *v = stack_index(idx);
    if (v->is_file())
        return v->as.object->as.file;
    throw raise("Type error", "not a file");
}

Array<Variant> &Environment::to_list(int idx)
{
    Variant *v = stack_index(idx);
    if (v->is_list())
        return v->as.object->as.list;
    throw raise("Type error", "not a list");
}

void Environment::add_method(const char *name, native_callback_t cfun, int n)
{
    const char *pname = strrchr(name, '.');
    pname = pname ? pname + 1 : name;
    new_native_function(std::move(cfun), name, n);
    def_field(-2, pname, PHON_DONTENUM);
}

void Environment::add_global_function(const char *name, native_callback_t cfun, int n)
{
    new_native_function(std::move(cfun), name, n);
    def_global(name, PHON_DONTENUM);
}

void Environment::add_math_constant(const char *name, double number)
{
    push(number);
    def_field(-2, name, PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
}

void Environment::add_string_field(const char *name, const char *string)
{
    push(string);
    def_field(-2, name, PHON_DONTENUM);
}

Variant &Environment::get(int idx)
{
    return *stack_index(idx);
}

const Variant &Environment::get(int idx) const
{
    return *stack_index(idx);
}

void Environment::add_accessor(const String &name, native_callback_t getter)
{
    new_native_function(std::move(getter), name, 0);
    push_null(); // setter
    def_accessor(-3, name, PHON_READONLY | PHON_DONTENUM);
}

void Environment::add_accessor(const String &name, native_callback_t getter, native_callback_t setter)
{
    new_native_function(std::move(getter), name, 0);
    new_native_function(std::move(setter), name, 1);
    def_accessor(-3, name, PHON_DONTENUM);
}

void Environment::push(Array<Variant> lst)
{
    auto obj = new Object(*this, PHON_CLIST, list_meta);
    new (&obj->as.list) Array<Variant>(std::move(lst));
    push(obj);
}

int Environment::get_stack_trace(int skip)
{
    char buf[256];
    int n = this->tracetop - skip;
    if (n <= 0)
        return 0;
    for (; n > 0; --n)
    {
        auto &name = this->trace[n].name;
        auto &file = this->trace[n].file;
        int line = this->trace[n].line;
        if (line > 0)
        {
            if (!name.empty())
                snprintf(buf, sizeof buf, "\n\tat %s (%s:%d)", name.data(), file.data(), line);
            else
                snprintf(buf, sizeof buf, "\n\tat %s:%d", file.data(), line);
        }
        else
            snprintf(buf, sizeof buf, "\n\tat %s (%s)", name.data(), file.data());
        this->push(buf);
        if (n < this->tracetop - skip)
            js_concat(this);
    }
    return 1;
}


} // namespace phonometrica