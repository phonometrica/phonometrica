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
#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/parse.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/utils/print.hpp>
#include <phon/utils/alloc.hpp>
#include "runtime.hpp"


namespace phonometrica {

static void jsR_run(Runtime *J, Function *F);

/* Push values on stack */

// TODO: remove stack macros (low priority)
#define TOP (J->top)
#define BOT (J->bot)

#define CHECKSTACK(n) if (TOP + n >= J->stack + PHON_STACKSIZE) throw std::runtime_error("stack overflow");

static void *default_alloc(void *actx, void *ptr, int size)
{
    if (size == 0)
    {
        free(ptr);
        return nullptr;
    }
    return realloc(ptr, (size_t) size);
}

static void default_report(Runtime *, const String &message)
{
    utils::print(stderr, message);
    utils::print(stderr, "\n");
}

static void default_print(const String &s)
{
    utils::printf(s);
}

static void default_panic(Runtime *J)
{
    J->report("uncaught exception");
    /* return to javascript to abort */
}

static void js_loadstringx(Runtime *J, const String &filename, const String &source, bool iseval)
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

void load_eval(Runtime *J, const String &filename, const String &source)
{
    js_loadstringx(J, filename, source, 1);
}

Runtime::Runtime() :
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
    this->E = new_environment(this, this->G, nullptr);
    this->GE = this->E;

    initialize();
}

void Runtime::set_report_callback(report_callback_t report)
{
    this->report_callback = report;
}

void Runtime::report(const String &message)
{
    if (report_callback)
        report_callback(this, message);
}

panic_callback_t Runtime::at_panic(panic_callback_t panic)
{
    auto old = this->panic;
    this->panic = panic;

    return old;
}

int Runtime::do_string(const String &source)
{
	if (initialize_script) initialize_script();
    this->load_string("[string]", source);
    push_null();
    call(0);
    pop(1);
    if (finalize_script) finalize_script();

    return 0;
}

int Runtime::do_file(const String &filename)
{
	if (initialize_script) initialize_script();
    load_file(filename);
    push_null();
    call(0);
    pop(1);
    if (finalize_script) finalize_script();

    return 0;
}

int Runtime::pcall(int n)
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

int Runtime::pconstruct(int n)
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

void Runtime::pop(int n)
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

void Runtime::load_string(const String &filename, const String &source)
{
    js_loadstringx(this, filename, source, 0);
}

void Runtime::load_file(const String &filename)
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

void Runtime::eval()
{
    if (!is_string(-1))
        return;
    load_eval(this, "(eval)", to_string(-1));
    js_rot2pop1(this);
    copy(0); /* copy 'this' */
    call(0);
}

String Runtime::internalize(const String &s)
{
    auto it = strings.insert(s);
    return *it.first;
}

std::runtime_error Runtime::raise(const char *error_category, std::exception &e)
{
    return raise(error_category, "%s", e.what());
}

std::runtime_error Runtime::raise(const char *error_category, const char *fmt, ...)
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

double Runtime::to_number(Variant *v)
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

intptr_t Runtime::to_integer(Variant *v)
{
    return number_to_integer(to_number(v));
}

File &Runtime::to_file(int idx)
{
    Variant *v = stack_index(idx);
    if (v->is_file())
        return v->as.object->as.file;
    throw raise("Type error", "not a file");
}

Array<Variant> &Runtime::to_list(int idx)
{
    Variant *v = stack_index(idx);
    if (v->is_list())
        return v->as.object->as.list;
    throw raise("Type error", "not a list");
}

void Runtime::add_method(const char *name, native_callback_t cfun, int n)
{
    const char *pname = strrchr(name, '.');
    pname = pname ? pname + 1 : name;
    new_native_function(std::move(cfun), name, n);
    def_field(-2, pname, PHON_DONTENUM);
}

void Runtime::add_global_function(const char *name, native_callback_t cfun, int n)
{
    new_native_function(std::move(cfun), name, n);
    def_global(name, PHON_DONTENUM);
}

void Runtime::add_math_constant(const char *name, double number)
{
    push(number);
    def_field(-2, name, PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
}

void Runtime::add_string_field(const char *name, const char *string)
{
    push(string);
    def_field(-2, name, PHON_DONTENUM);
}

Variant &Runtime::get(int idx)
{
    return *stack_index(idx);
}

const Variant &Runtime::get(int idx) const
{
    return *stack_index(idx);
}

void Runtime::add_accessor(const String &name, native_callback_t getter)
{
    new_native_function(std::move(getter), name, 0);
    push_null(); // setter
    def_accessor(-3, name, PHON_READONLY | PHON_DONTENUM);
}

void Runtime::add_accessor(const String &name, native_callback_t getter, native_callback_t setter)
{
    new_native_function(std::move(getter), name, 0);
    new_native_function(std::move(setter), name, 1);
    def_accessor(-3, name, PHON_DONTENUM);
}

void Runtime::push(Array<Variant> lst)
{
    auto obj = new Object(*this, PHON_CLIST, list_meta);
    new (&obj->as.list) Array<Variant>(std::move(lst));
    push(obj);
}

int Runtime::get_stack_trace(int skip)
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


void Runtime::push(const Variant &v)
{
	check_stack(1);
	*top = v;
	++top;
}

void Runtime::push(Variant &&v)
{
	check_stack(1);
	*top = std::move(v);
	++top;
}

void Runtime::push_null()
{
	check_stack(1);
	top->undef();
	++top;
}

void Runtime::push_undefined()
{
	push(std::nan(""));
}

void Runtime::push_boolean(bool value)
{
	check_stack(1);
	top->set_boolean(value);
	++top;
}

void Runtime::push(double v)
{
	check_stack(1);
	top->set_number(v);
	++top;
}

void Runtime::push(String &&v)
{
	check_stack(1);
	top->set_string(std::move(v));
	++top;
}

void Runtime::push(Matrix<double> m)
{
	auto obj = new Object(*this, PHON_CARRAY, array_meta);
	new (&obj->as.array) Matrix<double>(std::move(m));
	push(obj);
}

void Runtime::push(const char *v, intptr_t n)
{
	push(String(v, n));
}

void Runtime::push(const String &v)
{
	check_stack(1);
	top->set_string(v);
	++top;
}

void Runtime::push(Object *v)
{
	check_stack(1);
	new (top) Variant(v);
	++top;
}

void Runtime::push_global()
{
	push(G);
}

void Runtime::current_function()
{
	check_stack(1);
	*top = *(bot - 1);
	++top;
}


/* Read values from stack */

Variant *Runtime::stack_index(int idx)
{
	static Variant undefined;
	auto pos = idx < 0 ? top + idx : bot + idx;

	if (pos < stack || pos >= top)
		return &undefined;

	return pos;
}

const Variant *Runtime::stack_index(int idx) const
{
	return const_cast<Runtime*>(this)->stack_index(idx);
}

Variant *get_variant(Runtime *J, int idx)
{
	return J->stack_index(idx);
}

bool Runtime::is_defined(int idx) const
{
	return !is_null(idx);
}

bool Runtime::is_null(int idx) const
{
	return stack_index(idx)->is_null();
}

bool Runtime::is_boolean(int idx) const
{
	return stack_index(idx)->is_boolean();
}

bool Runtime::is_number(int idx) const
{
	return stack_index(idx)->is_number();
}

bool Runtime::is_string(int idx) const
{
	return stack_index(idx)->is_string();
}

bool Runtime::is_primitive(int idx) const
{
	return !is_object(idx);
}

bool Runtime::is_object(int idx) const
{
	return stack_index(idx)->is_object();
}

bool Runtime::is_list(int idx) const
{
	return stack_index(idx)->is_list();
}

bool Runtime::is_regex(int idx) const
{
	return stack_index(idx)->is_regex();
}

bool Runtime::is_array(int idx) const
{
	return stack_index(idx)->is_array();
}

bool Runtime::is_file(int idx) const
{
	return stack_index(idx)->is_file();
}

bool Runtime::is_user_data(int idx, const char *tag) const
{
	return stack_index(idx)->is_user_data(tag);
}

bool Runtime::is_error(int idx) const
{
	return stack_index(idx)->is_error();
}


bool Runtime::is_coercible(int idx) const
{
	auto v = stack_index(idx);
	return !v->is_null();
}

bool Runtime::is_callable(int idx) const
{
	return stack_index(idx)->is_callable();
}


static const char *js_typeof(Runtime *J, int idx)
{
	Variant *v = J->stack_index(idx);
	switch (v->type)
	{
		default:
		case PHON_TNULL:
			return "Null";
		case PHON_TBOOLEAN:
			return "Boolean";
		case PHON_TNUMBER:
			return "Number";
		case PHON_TSTRING:
			return "String";
		case PHON_TOBJECT:
			if (v->as.object->type == PHON_CFUNCTION || v->as.object->type == PHON_CCFUNCTION)
				return "Function";
			if (v->is_list())
				return "List";
			if (v->is_regex())
				return "Regex";
			if (v->is_file())
				return "File";
			if (v->is_array())
				return "Array";
			if (v->is_user_data())
				return v->as.object->as.user.tag;
			return "Object";
	}
}

bool Runtime::to_boolean(int idx) const
{
	return stack_index(idx)->to_boolean();
}

Regex &Runtime::to_regex(int idx)
{
	Variant *v = stack_index(idx);
	if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CREGEX)
		return v->as.object->as.regex;
	throw raise("Type error", "not a Regex");
}

Matrix<double> &Runtime::to_array(int idx)
{
	Variant *v = stack_index(idx);
	if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CARRAY)
		return v->as.object->as.array;
	throw raise("Type error", "not an Array");
}


double Runtime::to_number(int idx)
{
	return to_number(stack_index(idx));
}

intptr_t Runtime::to_integer(int idx)
{
	return number_to_integer(to_number(stack_index(idx)));
}

int Runtime::to_int32(int idx)
{
	return number_to_int32(to_number(stack_index(idx)));
}

unsigned int Runtime::to_uint32(int idx)
{
	return number_to_uint32(to_number(stack_index(idx)));
}

short Runtime::to_int16(int idx)
{
	return number_to_int16(to_number(stack_index(idx)));
}

unsigned short Runtime::to_uint16(int idx)
{
	return number_to_uint16(to_number(stack_index(idx)));
}

String Runtime::to_string(int idx)
{
	return var_to_string(this, stack_index(idx));
}

std::any & Runtime::to_user_data(int idx, const char *tag)
{
	Variant *v = stack_index(idx);

	if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CUSERDATA)
	{
		if (!strcmp(tag, v->as.object->as.user.tag))
			return v->as.object->as.user.data;
	}

	throw raise("Type error", "not a %s", tag);
}

std::any & Runtime::to_user_data(int idx)
{
	Variant *v = stack_index(idx);

	if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CUSERDATA)
	{
		return v->as.object->as.user.data;
	}

	throw raise("Type error", "not a user data");
}

Object *Runtime::to_object(int idx)
{
	return to_object(stack_index(idx));
}

void var_to_primitive(Runtime *J, int idx, int hint)
{
	var_to_primitive(J, J->stack_index(idx), hint);
}

static Object *jsR_tofunction(Runtime *J, int idx)
{
	Variant *v = J->stack_index(idx);
	if (v->is_null())
		return nullptr;
	if (v->type == PHON_TOBJECT)
		if (v->as.object->type == PHON_CFUNCTION || v->as.object->type == PHON_CCFUNCTION)
			return v->as.object;
	throw J->raise("Type error", "not a function");
}

/* Stack manipulation */

int Runtime::top_count() const
{
	return int(top - bot);
}

void Runtime::copy(int idx)
{
	check_stack(1);
	*top = *stack_index(idx);
	++top;
}

void Runtime::remove(int idx)
{
	auto pos = idx < 0 ? top + idx : bot + idx;

	if (pos < bot || pos >= top)
	{
		throw raise("Error", "stack error!");
	}

	for (; pos < top - 1; ++pos)
	{
		pos = pos + 1;
	}
	--top;
}

void js_dup(Runtime *J)
{
	CHECKSTACK(1);
	*TOP = *(TOP - 1);
	++TOP;
}

void js_dup2(Runtime *J)
{
	CHECKSTACK(2);
	TOP = TOP - 2;
	*(TOP + 1) = *(TOP - 1);
	TOP += 2;
}

void js_rot2(Runtime *J)
{
	/* A B -> B A */
	std::swap(*(TOP - 1), *(TOP - 2));
}

void js_rot3(Runtime *J)
{
	/* A B C -> C A B */
	Variant tmp = *(TOP - 1);    /* A B C (C) */
	*(TOP - 1) = *(TOP - 2);    /* A B B */
	*(TOP - 2) = *(TOP - 3);    /* A A B */
	*(TOP - 3) = std::move(tmp);        /* C A B */
}

void js_rot4(Runtime *J)
{
	/* A B C D -> D A B C */
	Variant tmp = *(TOP - 1);    /* A B C D (D) */
	*(TOP - 1) = *(TOP - 2);    /* A B C C */
	*(TOP - 2) = *(TOP - 3);    /* A B B C */
	*(TOP - 3) = *(TOP - 4);    /* A A B C */
	*(TOP - 4) = std::move(tmp);        /* D A B C */
}

void js_rot2pop1(Runtime *J)
{
	/* A B -> B */
	*(TOP - 2) = std::move(*(TOP - 1));
	--TOP;
}

void js_rot3pop2(Runtime *J)
{
	/* A B C -> C */
	*(TOP - 3) = std::move(*(TOP - 1));
	J->pop(2);
}

void Runtime::rot(int n)
{
	int i;
	Variant tmp = *(top - 1);

	for (i = 1; i < n; ++i)
	{
		*(top - i) = std::move(*(top - i - 1));
	}

	*(top - i) = std::move(tmp);
}


/* Field access that takes care of attributes and getters/setters */

int is_array_index(Runtime *J, const String &s, int *idx)
{
	int n = 0;
	auto p = s.data();
	while (*p)
	{
		int c = *p++;
		if (c >= '0' && c <= '9')
		{
			if (n >= INT_MAX / 10)
				return 0;
			n = n * 10 + (c - '0');
		}
		else
		{
			return 0;
		}
	}
	return *idx = n, 1;
}

bool Runtime::has_field(Object *obj, const String &name)
{
	Field *ref;
	int k;

	if (name == "length")
	{
		if (obj->type == PHON_CLIST)
		{
			push(obj->as.list.size());
		}
		else if (obj->type == PHON_CSTRING)
		{
			push(obj->as.string.grapheme_count());
		}
		else if (obj->type == PHON_CREGEX)
		{
			push(obj->as.regex.count());
		}
		else if (obj->type == PHON_CARRAY)
		{
			push(obj->as.array.size());
		}
		else if (obj->type == PHON_COBJECT)
		{
			push(obj->fields.size());
		}

		return true;
	}

	if (obj->type == PHON_CSTRING)
	{
		if (is_array_index(this, name, &k))
		{
			auto &s = obj->as.string;
			push(s.next_grapheme(k));
			return true;
		}
	}
	else if (obj->type == PHON_CREGEX)
	{
		if (name == "ignore_case")
		{
			push_boolean(obj->as.regex.flags() & Regex::Caseless);
			return true;
		}
		if (name == "multiline")
		{
			push_boolean(obj->as.regex.flags() & Regex::Multiline);
			return true;
		}
	}
	else if (obj->type == PHON_CFILE)
	{
		auto &file = obj->as.file;

		if (name == "path")
		{
			push(file.path());
			return true;
		}
		if (name == "length")
		{
			push(file.size());
			return true;
		}
	}
//    else if (obj->type == PHON_CUSERDATA)
//    {
//        if (obj->as.user.has && obj->as.user.has(this, obj->as.user.data, name))
//            return true;
//    }

	ref = obj->get_field(*this, name);
	if (ref)
	{
		if (ref->getter)
		{
			push(ref->getter);
			push(obj);
			this->call(0);
		}
		else
		{
			push(ref->value);
		}
		return true;
	}

	return false;
}


void Runtime::get_field(Object *obj, const String &name)
{
	if (!has_field(obj, name))
	{
		throw raise("Index error", "field \'%s\' does not exist", name.data());
	}
}

void Runtime::get_field(Object *obj, intptr_t pos)
{
	try
	{
		if (obj->type == PHON_CLIST)
		{
			auto &v = obj->as.list.at(pos);
			push(v);
		}
		else if (obj->type == PHON_CSTRING)
		{
			throw raise("Index error", "Cannot index String code point");
//            auto c = obj->as.string.next_codepoint(pos);
//            auto str = unicode::encode(c);
//            push(str.data);
		}
		else
		{
			throw raise("Error", "%s (type id: %d)", "cannot get numeric index for object which is not a list", obj->type);
		}
	}
	catch (std::runtime_error &e)
	{
		throw raise("Index error", e);
	}
}

void Runtime::set_field(Object *obj, intptr_t idx)
{
	if (obj->type != PHON_CLIST)
	{
		throw raise("Error", "%s (type id: %d)", "cannot set numeric index for object which is not a list", obj->type);
	}
	try
	{
		obj->as.list.at(idx) = *stack_index(-1);
	}
	catch (std::runtime_error &e)
	{
		throw raise("Index error", e);
	}
}

void Runtime::set_field(Object *obj, const String &name)
{
	Variant *value = stack_index(-1);
	Field *ref;
	int k;
	bool own;

	if (obj->type == PHON_CLIST)
	{
		if (name == "length")
		{
			double rawlen = to_number(value);
			int newlen = number_to_integer(rawlen);
			if (newlen != rawlen || newlen < 0)
				throw raise("Range error", "invalid array length");
			obj->resize_list(*this, newlen);
			return;
		}
	}
	else if (obj->type == PHON_CSTRING)
	{
		if (name == "length")
			goto readonly;
		if (is_array_index(this, name, &k))
			if (k >= 0 && k < obj->as.string.size())
				goto readonly;
	}
	else if (obj->type == PHON_CREGEX)
	{
		if (name == "pattern") goto readonly;
		if (name == "length") goto readonly;
	}
	else if (obj->type == PHON_CFILE)
	{
		if (name == "path") goto readonly;
		if (name == "length") goto readonly;
	}
//    else if (obj->type == PHON_CUSERDATA)
//    {
//        if (obj->as.user.put && obj->as.user.put(this, obj->as.user.data, name))
//            return;
//    }
	/* First try to find a setter in prototype chain */
	ref = obj->get_field(*this, name, &own);
	if (ref)
	{
		if (ref->setter)
		{
			push(ref->setter);
			push(obj);
			push(*value);
			call(1);
			pop(1);
			return;
		}
		else
		{
			if (this->strict)
				if (ref->getter)
					goto readonly;
		}
	}

	/* Field not found on this object, so create one */
	if (!ref || !own)
		ref = obj->set_field(*this, name);

	if (ref)
	{
		if (!(ref->atts & PHON_READONLY))
			ref->value = *value;
		else
			goto readonly;
	}

	return;

	readonly:
	if (this->strict)
		throw raise("Error", "cannot set read-only field '%s'", name.data());

}

void Runtime::def_field(Object *obj, const String &name, int atts, Variant *value, Object *getter, Object *setter)
{
	Field *ref;
	int k;

	if (obj->type == PHON_CLIST)
	{
		if (name == "length")
			goto readonly;
	}
	else if (obj->type == PHON_CSTRING)
	{
		if (name == "length")
			goto readonly;
		if (is_array_index(this, name, &k))
			if (k >= 0 && k < obj->as.string.size())
				goto readonly;
	}
	else if (obj->type == PHON_CREGEX)
	{
		if (name == "pattern") goto readonly;
		if (name == "global") goto readonly;
		if (name == "ignore_case") goto readonly;
		if (name == "multiline") goto readonly;
	}
	else if (obj->type == PHON_CFILE)
	{
		if (name == "path") goto readonly;
		if (name == "length") goto readonly;
	}
//    else if (obj->type == PHON_CUSERDATA)
//    {
//        if (obj->as.user.put && obj->as.user.put(this, obj->as.user.data, name))
//            return;
//    }

	ref = obj->set_field(*this, name);
	if (ref)
	{
		if (value)
		{
			if (!(ref->atts & PHON_READONLY))
				ref->value = *value;
			else if (this->strict)
				throw raise("Type error", "'%s' is read-only", name.data());
		}
		if (getter)
		{
			if (!(ref->atts & PHON_DONTCONF))
				ref->getter = getter;
			else if (this->strict)
				throw raise("Type error", "'%s' is non-configurable", name.data());
		}
		if (setter)
		{
			if (!(ref->atts & PHON_DONTCONF))
				ref->setter = setter;
			else if (this->strict)
				throw raise("Type error", "'%s' is non-configurable", name.data());
		}
		ref->atts |= atts;
	}

	return;

	readonly:
	if (this->strict)
		throw raise("Type error", "'%s' is read-only or non-configurable", name.data());
}

int Runtime::del_field(Object *obj, const String &name)
{
	Field *ref;
	int k;

	if (obj->type == PHON_CLIST)
	{
		if (name == "length")
			goto dontconf;
	}
	else if (obj->type == PHON_CSTRING)
	{
		if (name == "length")
			goto dontconf;
		if (is_array_index(this, name, &k))
			if (k >= 0 && k < obj->as.string.size())
				goto dontconf;
	}
	else if (obj->type == PHON_CREGEX)
	{
		if (name == "pattern") goto dontconf;
	}
	else if (obj->type == PHON_CFILE)
	{
		if (name == "path" || name == "length") goto dontconf;
	}
	else if (obj->type == PHON_CUSERDATA)
	{
		if (obj->as.user.destroy && obj->as.user.destroy(this, obj->as.user.data, name))
			return 1;
	}

	ref = obj->get_own_field(*this, name);
	if (ref)
	{
		if (ref->atts & PHON_DONTCONF)
			goto dontconf;
		obj->del_field(*this, name);
	}
	return 1;

	dontconf:
	if (this->strict)
		throw raise("Type error", "'%s' is non-configurable", name.data());
	return 0;
}

/* Registry, global and object field accessors */

String Runtime::ref()
{
	static String Null("_Null");
	static String True("_True");
	static String False("_False");
	Variant *v = stack_index(-1);
	String s;
	char buf[32];

	switch (v->type)
	{
		case PHON_TNULL:
			s = Null;
			break;
		case PHON_TBOOLEAN:
			s = v->as.boolean ? True : False;
			break;
		case PHON_TOBJECT:
			sprintf(buf, "%p", (void *) v->as.object);
			s = internalize(buf);
			break;
		default:
			sprintf(buf, "%d", nextref++);
			s = internalize(buf);
			break;
	}
	set_registry(s);

	return s;
}

void Runtime::unref(const String &ref)
{
	del_registry(ref);
}

void Runtime::get_registry(const String &name)
{
	get_field(R, name);
}

void Runtime::set_registry(const String &name)
{
	set_field(R, name);
	pop(1);
}

void Runtime::del_registry(const String &name)
{
	del_field(R, name);
}

void Runtime::get_global(const String &name)
{
	get_field(G, name);
}

void Runtime::set_global(const String &name)
{
	set_field(G, name);
	pop(1);
}

void Runtime::def_global(const String &name, int atts)
{
	def_field(G, name, atts, stack_index(-1), nullptr, nullptr);
	pop(1);
}

void Runtime::get_field(int idx, const String &name)
{
	get_field(to_object(idx), name);
}

void Runtime::set_field(int idx, const String &name)
{
	set_field(to_object(idx), name);
	pop(1);
}

void Runtime::def_field(int idx, const String &name, int atts)
{
	def_field(to_object(idx), name, atts, stack_index(-1), nullptr, nullptr);
	pop(1);
}

void Runtime::del_field(int idx, const String &name)
{
	del_field(to_object(idx), name);
}

void Runtime::def_accessor(int idx, const String &name, int atts)
{
	def_field(to_object(idx), name, atts, nullptr, jsR_tofunction(this, -2), jsR_tofunction(this, -1));
	pop(2);
}

bool Runtime::has_field(int idx, const String &name)
{
	return has_field(to_object(idx), name);
}

/* Iterator */

void Runtime::push_iterator(int idx)
{
	auto obj = to_object(idx);
	push(obj->new_iterator(*this));
}

std::optional<Variant> Runtime::next_iterator(int idx)
{
	return to_object(idx)->next_iterator(*this);
}

/* Environment records */

Environment *new_environment(Runtime *J, Object *variables, Environment *outer)
{
	auto E = new Environment;
	E->gcmark = 0;
	E->gcnext = J->gcenv;
	J->gcenv = E;
	++J->gccounter;

	E->outer = outer;
	E->variables = variables;
	return E;
}

static void init_variable(Runtime *J, const String &name, int idx)
{
	J->def_field(J->E->variables, name, PHON_DONTENUM | PHON_DONTCONF, J->stack_index(idx), nullptr, nullptr);
}

static void define_variable(Runtime *J, const String &name)
{
	J->def_field(J->E->variables, name, PHON_DONTENUM | PHON_DONTCONF, nullptr, nullptr, nullptr);
}

static int has_variable(Runtime *J, const String &name)
{
	Environment *E = J->E;
	do
	{
		Field *ref = E->variables->get_field(*J, name);
		if (ref)
		{
			if (ref->getter)
			{
				J->push(ref->getter);
				J->push(E->variables);
				J->call(0);
			}
			else
			{
				J->push(ref->value);
			}
			return 1;
		}
		E = E->outer;
	} while (E);
	return 0;
}

static void set_variable(Runtime *J, const String &name)
{
	Environment *E = J->E;
	do
	{
		Field *ref = E->variables->get_field(*J, name);
		if (ref)
		{
			if (ref->setter)
			{
				J->push(ref->setter);
				J->push(E->variables);
				J->copy(-3);
				J->call(1);
				J->pop(1);
				return;
			}
			if (!(ref->atts & PHON_READONLY))
				ref->value = *J->stack_index(-1);
			else if (J->strict)
				throw J->raise("Type error", "'%s' is read-only", name.data());
			return;
		}
		E = E->outer;
	} while (E);
	if (J->strict)
		throw J->raise("Reference error", "assignment to undeclared variable '%s'", name.data());
	J->set_field(J->G, name);
}

static int delete_variable(Runtime *J, const String &name)
{
	Environment *E = J->E;
	do
	{
		Field *ref = E->variables->get_own_field(*J, name);
		if (ref)
		{
			if (ref->atts & PHON_DONTCONF)
			{
				if (J->strict)
					throw J->raise("Type error", "'%s' is non-configurable", name.data());
				return 0;
			}
			E->variables->del_field(*J, name);
			return 1;
		}
		E = E->outer;
	} while (E);
	return J->del_field(J->G, name);
}

/* Function calls */

static void save_scope(Runtime *J, Environment *newE)
{
	if (J->nstop + 1 >= PHON_ENVLIMIT)
		throw std::runtime_error("stack overflow");
	J->nsstack[J->nstop++] = J->E;
	J->E = newE;
}

static void restore_scope(Runtime *J)
{
	J->E = J->nsstack[--J->nstop];
}

static void call_wfunction(Runtime *J, int n, Function *F, Environment *scope)
{
	Variant v;
	int i;

	save_scope(J, scope);

	if (n > F->numparams)
	{
		J->pop(n - F->numparams);
		n = F->numparams;
	}
	for (i = n; i < F->vartab.size(); ++i)
		J->push_null();

	jsR_run(J, F);
	v = *J->stack_index(-1);
	J->clear_stack();
	J->push(v);

	restore_scope(J);
}

static void call_function(Runtime *J, int n, Function *F, Environment *scope)
{
	Variant v;
	int i;

	scope = new_environment(J, new Object(*J, PHON_COBJECT, nullptr), scope);

	save_scope(J, scope);

	if (F->arguments)
	{
		J->new_object();
		if (!J->strict)
		{
			J->current_function();
			J->def_field(-2, "callee", PHON_DONTENUM);
		}
		J->push(n);
		J->def_field(-2, "length", PHON_DONTENUM);
		for (i = 0; i < n; ++i)
		{
			J->copy(i + 1);
			js_setindex(J, -2, i);
		}
		init_variable(J, "arguments", -1);
		J->pop(1);
	}

	for (i = 0; i < F->numparams; ++i)
	{
		if (i < n)
			init_variable(J, F->vartab[i], i + 1);
		else
		{
			J->push_null();
			init_variable(J, F->vartab[i], -1);
			J->pop(1);
		}
	}
	J->pop(n);

	jsR_run(J, F);
	v = *J->stack_index(-1);
	J->clear_stack();
	J->push(v);

	restore_scope(J);
}

static void call_script(Runtime *J, int n, Function *F, Environment *scope)
{
	Variant v;

	if (scope)
		save_scope(J, scope);

	J->pop(n);
	jsR_run(J, F);
	v = *J->stack_index(-1);
	J->clear_stack();
	J->push(v);

	if (scope)
		restore_scope(J);
}

static void call_native_function(Runtime *J, int n, int min, native_callback_t &F)
{
	int i;
	Variant v;

	for (i = n; i < min; ++i)
		J->push_null();

	F(*J);
	v = *J->stack_index(-1);
	J->clear_stack();
	J->push(v);
}

static void push_trace(Runtime *J, const String &name, const String &file, int line)
{
	if (J->tracetop + 1 == PHON_ENVLIMIT)
		throw J->raise("Runtime error", "call stack overflow");
	++J->tracetop;
	J->trace[J->tracetop].name = name;
	J->trace[J->tracetop].file = file;
	J->trace[J->tracetop].line = line;
}

/* Main interpreter loop */

void Runtime::dump_stack()
{
	printf("stack {\n");

	for (auto i = stack; i < top; ++i)
	{
		putchar(i == bot ? '>' : ' ');
		printf("% 4d: ", int(i - stack));
		dump_variant(this, *i);
		putchar('\n');
	}
	printf("}\n");
}

static void jsR_dumpenvironment(Runtime *J, Environment *E, int d)
{
	printf("scope %d ", d);
	dump_object(J, E->variables);
	if (E->outer)
		jsR_dumpenvironment(J, E->outer, d + 1);
}

void js_stacktrace(Runtime *J)
{
	int n;
	printf("stack trace:\n");
	for (n = J->tracetop; n >= 0; --n)
	{
		auto &name = J->trace[n].name;
		auto &file = J->trace[n].file;
		int line = J->trace[n].line;
		if (line > 0)
		{
			if (!name.empty())
				printf("\tat %s (%s:%d)\n", name.data(), file.data(), line);
			else
				printf("\tat %s:%d\n", file.data(), line);
		}
		else
			printf("\tat %s (%s)\n", name.data(), file.data());
	}
}

void trap(Runtime *J, int pc)
{
	if (pc > 0)
	{
		Function *F = (BOT - 1)->as.object->as.f.function;
		printf("trap at %d in function ", pc);
		jsC_dumpfunction(J, F);
	}
	J->dump_stack();
	jsR_dumpenvironment(J, J->E, 0);
	js_stacktrace(J);
}

static void jsR_run(Runtime *J, Function *F)
{
	auto &FT = F->funtab;
	auto &NT = F->numtab;
	auto &ST = F->strtab;
	instruction_t *pcstart = F->code.data();
	instruction_t *pc = F->code.data();
	enum js_OpCode opcode;
	int offset;
	int savestrict;

	const char *str;
	Object *obj;
	double x, y;
	unsigned int ux, uy;
	int ix, iy, okay;
	int b;

	savestrict = J->strict;
	J->strict = F->strict;

	while (true)
	{
		if (J->gccounter > PHON_GCLIMIT)
		{
			J->collect();
		}

		opcode = (js_OpCode) *pc++;
		switch (opcode)
		{
			case OP_POP:
				J->pop(1);
				break;
			case OP_DUP:
				js_dup(J);
				break;
			case OP_DUP2:
				js_dup2(J);
				break;
			case OP_ROT2:
				js_rot2(J);
				break;
			case OP_ROT3:
				js_rot3(J);
				break;
			case OP_ROT4:
				js_rot4(J);
				break;

			case OP_NUMBER_0:
				J->push_int(0);
				break;
			case OP_NUMBER_1:
				J->push_int(1);
				break;
			case OP_NUMBER_POS:
				J->push(*pc++);
				break;
			case OP_NUMBER_NEG:
				J->push(-(*pc++));
				break;
			case OP_NUMBER:
				J->push(NT[*pc++]);
				break;
			case OP_STRING:
				J->push(ST[*pc++]);
				break;

			case OP_CLOSURE:
				create_function(J, FT[*pc++], J->E);
				break;
			case OP_NEWOBJECT:
				J->new_object();
				break;
			case OP_NEWLIST:
			{
				SizeOpcode op;
				memcpy(op.ins, pc, sizeof(SizeOpcode));
				pc += OpSize;
				J->new_list(op.size);
				break;
			}
			case OP_NEWREGEXP:
				J->new_regex(ST[pc[0]], pc[1]);
				pc += 2;
				break;

			case OP_UNDEF:
				J->push_undefined();
				break;
			case OP_NULL:
				J->push_null();
				break;
			case OP_TRUE:
				J->push_boolean(true);
				break;
			case OP_FALSE:
				J->push_boolean(false);
				break;

			case OP_THIS:
				if (J->strict)
				{
					J->copy(0);
				}
				else
				{
					if (J->is_coercible(0))
						J->copy(0);
					else
						J->push_global();
				}
				break;

			case OP_CURRENT:
				J->current_function();
				break;

			case OP_INITLOCAL:
				*(BOT + *pc++) = *(--TOP);
				break;

			case OP_GETLOCAL:
				CHECKSTACK(1);
				*(TOP++) = *(BOT + *pc++);
				break;

			case OP_SETLOCAL:
				*(BOT + *pc++) = *(TOP - 1);
				break;

			case OP_DELLOCAL:
				++pc;
				J->push_boolean(false);
				break;

			case OP_INITVAR:
				init_variable(J, ST[*pc++], -1);
				J->pop(1);
				break;

			case OP_DEFVAR:
				define_variable(J, ST[*pc++]);
				break;

			case OP_GETVAR:
			{
				auto &str = ST[*pc++];
				if (!has_variable(J, str))
					throw J->raise("Reference error", "'%s' is not defined", str.data());
				break;
			}
			case OP_HASVAR:
				if (!has_variable(J, ST[*pc++]))
					J->push_null();
				break;

			case OP_SETVAR:
				set_variable(J, ST[*pc++]);
				break;

			case OP_DELVAR:
				b = delete_variable(J, ST[*pc++]);
				J->push_boolean(b);
				break;

			case OP_IN:
			{
				auto str = J->to_string(-2);
				if (!J->is_object(-1))
					throw J->raise("Type error", "operand to 'in' is not an object");
				auto b = J->has_field(-1, str);
				J->pop(2 + b);
				J->push_boolean(b);
				break;
			}
			case OP_INITPROP:
			{
				obj = J->to_object(-3);
				if (J->is_number(-2))
				{
					auto i = J->to_integer(-2);
					J->set_field(obj, i);
				}
				else
				{
					auto str = J->to_string(-2);
					J->set_field(obj, str);
				}
				J->pop(2);
				break;
			}
			case OP_INITGETTER:
			{
				obj = J->to_object(-3);
				auto str = J->to_string(-2);
				J->def_field(obj, str, 0, nullptr, jsR_tofunction(J, -1), nullptr);
				J->pop(2);
				break;
			}
			case OP_INITSETTER:
			{
				obj = J->to_object(-3);
				auto str = J->to_string(-2);
				J->def_field(obj, str, 0, nullptr, nullptr, jsR_tofunction(J, -1));
				J->pop(2);
				break;
			}
			case OP_GETPROP:
			{
				obj = J->to_object(-2);

				if (J->is_number(-1))
				{
					auto pos = J->to_integer(-1);
					J->get_field(obj, pos);
				}
				else
				{
					auto str = J->to_string(-1);
					J->get_field(obj, str);
				}

				js_rot3pop2(J);
				break;
			}
			case OP_GETPROP_S:
			{
				auto &str = ST[*pc++];
				obj = J->to_object(-1);
				J->get_field(obj, str);
				js_rot2pop1(J);
				break;
			}
			case OP_SETPROP:
			{
				obj = J->to_object(-3);

				if (J->is_number(-2))
				{
					auto pos = J->to_number(-2);
					J->set_field(obj, pos);
				}
				else
				{
					auto str = J->to_string(-2);
					J->set_field(obj, str);
				}
				js_rot3pop2(J);
				break;
			}
			case OP_SETPROP_S:
			{
				auto &str = ST[*pc++];
				obj = J->to_object(-2);
				J->set_field(obj, str);
				js_rot2pop1(J);
				break;
			}
			case OP_DELPROP:
			{
				auto str = J->to_string(-1);
				obj = J->to_object(-2);
				b = J->del_field(obj, str);
				J->pop(2);
				J->push_boolean(b);
				break;
			}
			case OP_DELPROP_S:
			{
				auto &str = ST[*pc++];
				obj = J->to_object(-1);
				b = J->del_field(obj, str);
				J->pop(1);
				J->push_boolean(b);
				break;
			}
			case OP_ITERATOR:
				if (J->is_coercible(-1))
				{
					auto o = J->to_object(-1);
					obj = o->new_iterator(*J);
					J->pop(1);
					J->push(obj);
				}
				break;

			case OP_NEXTITER:
				if (J->is_object(-1))
				{
					obj = J->to_object(-1);
					auto it = obj->next_iterator(*J);
					if (it)
					{
						J->push(std::move(*it));
						J->push_boolean(true);
					}
					else
					{
						J->pop(1);
						J->push_boolean(false);
					}
				}
				else
				{
					J->pop(1);
					J->push_boolean(false);
				}
				break;

				/* Function calls */

			case OP_EVAL:
				J->eval();
				break;

			case OP_CALL:
				J->call(*pc++);
				break;

			case OP_NEW:
				J->construct(*pc++);
				break;

				/* Unary operators */

			case OP_TYPEOF:
				str = js_typeof(J, -1);
				J->pop(1);
				J->push(str);
				break;

			case OP_POS:
				x = J->to_number(-1);
				J->pop(1);
				J->push(x);
				break;

			case OP_NEG:
				x = J->to_number(-1);
				J->pop(1);
				J->push(-x);
				break;

			case OP_BITNOT:
				ix = J->to_int32(-1);
				J->pop(1);
				J->push(~ix);
				break;

			case OP_LOGNOT:
				b = J->to_boolean(-1);
				J->pop(1);
				J->push_boolean(!b);
				break;

			case OP_INC:
				x = J->to_number(-1);
				J->pop(1);
				J->push(x + 1);
				break;

			case OP_DEC:
				x = J->to_number(-1);
				J->pop(1);
				J->push(x - 1);
				break;

			case OP_POSTINC:
				x = J->to_number(-1);
				J->pop(1);
				J->push(x + 1);
				J->push(x);
				break;

			case OP_POSTDEC:
				x = J->to_number(-1);
				J->pop(1);
				J->push(x - 1);
				J->push(x);
				break;

				/* Multiplicative operators */

			case OP_MUL:
				x = J->to_number(-2);
				y = J->to_number(-1);
				J->pop(2);
				J->push(x * y);
				break;

			case OP_DIV:
				x = J->to_number(-2);
				y = J->to_number(-1);
				J->pop(2);
				J->push(x / y);
				break;

			case OP_MOD:
				x = J->to_number(-2);
				y = J->to_number(-1);
				J->pop(2);
				J->push(fmod(x, y));
				break;

				/* Additive operators */

			case OP_ADD:
				js_concat(J);
				break;

			case OP_SUB:
				x = J->to_number(-2);
				y = J->to_number(-1);
				J->pop(2);
				J->push(x - y);
				break;

				/* Shift operators */

			case OP_SHL:
				ix = J->to_int32(-2);
				uy = J->to_uint32(-1);
				J->pop(2);
				J->push(ix << (uy & 0x1F));
				break;

			case OP_SHR:
				ix = J->to_int32(-2);
				uy = J->to_uint32(-1);
				J->pop(2);
				J->push(ix >> (uy & 0x1F));
				break;

			case OP_USHR:
				ux = J->to_uint32(-2);
				uy = J->to_uint32(-1);
				J->pop(2);
				J->push(ux >> (uy & 0x1F));
				break;

				/* Relational operators */

			case OP_LT:
				b = js_compare(J, &okay);
				J->pop(2);
				J->push_boolean(okay && b < 0);
				break;
			case OP_GT:
				b = js_compare(J, &okay);
				J->pop(2);
				J->push_boolean(okay && b > 0);
				break;
			case OP_LE:
				b = js_compare(J, &okay);
				J->pop(2);
				J->push_boolean(okay && b <= 0);
				break;
			case OP_GE:
				b = js_compare(J, &okay);
				J->pop(2);
				J->push_boolean(okay && b >= 0);
				break;

			case OP_INSTANCEOF:
				b = js_instanceof(J);
				J->pop(2);
				J->push_boolean(b);
				break;

				/* Equality */

			case OP_EQ:
				b = js_equal(J);
				J->pop(2);
				J->push_boolean(b);
				break;
			case OP_NE:
				b = js_equal(J);
				J->pop(2);
				J->push_boolean(!b);
				break;

			case OP_JCASE:
				offset = *pc++;
				b = js_equal(J);
				if (b)
				{
					J->pop(2);
					pc = pcstart + offset;
				}
				else
				{
					J->pop(1);
				}
				break;

				/* Binary bitwise operators */

			case OP_BITAND:
				ix = J->to_int32(-2);
				iy = J->to_int32(-1);
				J->pop(2);
				J->push(ix & iy);
				break;

			case OP_BITXOR:
				ix = J->to_int32(-2);
				iy = J->to_int32(-1);
				J->pop(2);
				J->push(ix ^ iy);
				break;

			case OP_BITOR:
				ix = J->to_int32(-2);
				iy = J->to_int32(-1);
				J->pop(2);
				J->push(ix | iy);
				break;

				/* Branching */

			case OP_DEBUGGER:
				trap(J, (int) (pc - pcstart) - 1);
				break;

			case OP_JUMP:
				pc = pcstart + *pc;
				break;

			case OP_JTRUE:
				offset = *pc++;
				b = J->to_boolean(-1);
				J->pop(1);
				if (b)
					pc = pcstart + offset;
				break;

			case OP_JFALSE:
				offset = *pc++;
				b = J->to_boolean(-1);
				J->pop(1);
				if (!b)
					pc = pcstart + offset;
				break;

			case OP_RETURN:
				J->strict = savestrict;
				return;

			case OP_LINE:
				J->trace[J->tracetop].line = *pc++;
				break;
		}
	}
}

void Runtime::call(int n)
{
	auto &v = get(-n-2);
	if (!is_callable(-n - 2))
		throw raise("Type error", "called object is not a function");

	auto obj = to_object(-n - 2);
	auto savebot = bot;
	bot = top - n - 1;

	if (obj->type == PHON_CFUNCTION)
	{
		push_trace(this, obj->as.f.function->name, obj->as.f.function->filename, obj->as.f.function->line);
		if (obj->as.f.function->lightweight)
			call_wfunction(this, n, obj->as.f.function, obj->as.f.scope);
		else
			call_function(this, n, obj->as.f.function, obj->as.f.scope);
		--tracetop;
	}
	else if (obj->type == PHON_CSCRIPT)
	{
		push_trace(this, obj->as.f.function->name, obj->as.f.function->filename, obj->as.f.function->line);
		auto s = obj->as.f.function->name.data();
		call_script(this, n, obj->as.f.function, obj->as.f.scope);
		--tracetop;
	}
	else if (obj->type == PHON_CCFUNCTION)
	{
		push_trace(this, obj->as.c.name, "native", 0);
		call_native_function(this, n, obj->as.c.length, obj->as.c.function);
		--tracetop;
	}

	bot = savebot;
}

void Runtime::construct(int n)
{
	Object *obj;
	Object *prototype;
	Object *newobj;

	if (!is_callable(-n - 1))
		throw raise("Type error", "called object is not a function");

	obj = to_object(-n - 1);

	/* built-in constructors create their own objects, give them a 'null' this */
	if (obj->type == PHON_CCFUNCTION && obj->as.c.constructor)
	{
		auto savebot = this->bot;
		push_null();
		if (n > 0) rot(n + 1);
		this->bot = this->top - n - 1;

		push_trace(this, obj->as.c.name, "native", 0);
		call_native_function(this, n, obj->as.c.length, obj->as.c.constructor);
		--tracetop;

		this->bot = savebot;

		return;
	}

	/* extract the function object's prototype field */
	get_field(-n - 1, "meta");
	if (is_object(-1))
		prototype = to_object(-1);
	else
		prototype = this->object_meta;
	pop(1);

	/* create a new object with above prototype, and shift it into the 'this' slot */
	newobj = new Object(*this, PHON_COBJECT, prototype);
	push(newobj);
	if (n > 0) rot(n + 1);

	/* call the function */
	this->call(n);

	/* if result is not an object, return the original object we created */
	if (!is_object(-1))
	{
		pop(1);
		push(newobj);
	}
}

void Runtime::check_stack(int n)
{
	if (this->top + n >= this->stack + PHON_STACKSIZE)
		throw std::runtime_error("stack overflow");
}

void Runtime::clear_stack()
{
	while (top > bot)
	{
		(--top)->~Variant();
	}
	--top;
	--bot;
//    top = --bot;
}

} // namespace phonometrica