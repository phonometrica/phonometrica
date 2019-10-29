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

#include <cstring>
#include <limits>
#include <phon/runtime/toplevel.hpp>
#include <phon/runtime/lex.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include "runtime.hpp"
#include "variant.hpp"
#include "object.hpp"


namespace phonometrica {

static inline
bool equal(double x, double y)
{
	// Comparison method by Christer Ericson. See http://doubletimecollisiondetection.net/blog/?p=89
	double scale = std::max<double>(1.0, std::max<double>(std::fabs(x), std::fabs(y)));
	return std::fabs(x - y) <= std::numeric_limits<double>::epsilon() * scale;
}


intptr_t number_to_integer(double n)
{
    if (n == 0) return 0;
    if (std::isnan(n)) return 0;
    n = (n < 0) ? -floor(-n) : floor(n);
    if constexpr (sizeof(intptr_t) < sizeof(double))
    {
        if (n < std::numeric_limits<intptr_t>::min()) return std::numeric_limits<intptr_t>::min();
        if (n > std::numeric_limits<intptr_t>::max()) return std::numeric_limits<intptr_t>::max();
    }
    return (intptr_t) n;
}

int number_to_int32(double n)
{
    double two32 = 4294967296.0;
    double two31 = 2147483648.0;

    if (!std::isfinite(n) || n == 0)
        return 0;

    n = fmod(n, two32);
    n = n >= 0 ? floor(n) : ceil(n) + two32;
    if (n >= two31)
        return n - two32;
    else
        return n;
}

unsigned int number_to_uint32(double n)
{
    return (unsigned int) number_to_int32(n);
}

short number_to_int16(double n)
{
    return number_to_int32(n);
}

unsigned short number_to_uint16(double n)
{
    return number_to_int32(n);
}

/* obj.toString() */
static int jsV_toString(Runtime *J, Object *obj)
{
    J->push(obj);
    J->get_field(-1, "to_string");
    if (J->is_callable(-1))
    {
        js_rot2(J);
        J->call(0);
        if (J->is_primitive(-1))
            return 1;
        J->pop(1);
        return 0;
    }
    J->pop(2);
    return 0;
}

/* obj.to_value() */
static int jsV_to_value(Runtime *J, Object *obj)
{
    J->push(obj);
    J->get_field(-1, "to_value");
    if (J->is_callable(-1))
    {
        js_rot2(J);
        J->call(0);
        if (J->is_primitive(-1))
            return 1;
        J->pop(1);
        return 0;
    }
    J->pop(2);
    return 0;
}

/* ToPrimitive() on a value */
void var_to_primitive(Runtime *J, Variant *v, int preferred)
{
    Object *obj;

    if (v->type != PHON_TOBJECT)
        return;

    obj = v->as.object;

    if (preferred == PHON_HINT_NONE)
        preferred = obj->type == PHON_CDATE ? PHON_HINT_STRING : PHON_HINT_NUMBER;

    if (preferred == PHON_HINT_STRING)
    {
        if (jsV_toString(J, obj) || jsV_to_value(J, obj))
        {
            *v = *get_variant(J, -1);
            J->pop(1);
            return;
        }
    }
    else
    {
        if (jsV_to_value(J, obj) || jsV_toString(J, obj))
        {
            *v = *get_variant(J, -1);
            J->pop(1);
            return;
        }
    }

    if (J->strict)
        throw J->raise("Type error", "cannot convert object to primitive");

    v->set_string("[object]");
    return;
}

/* to_boolean() on a value */
bool Variant::to_boolean() const
{
    switch (type)
    {
    case PHON_TNULL:
        return false;
    case PHON_TBOOLEAN:
        return as.boolean;
    case PHON_TNUMBER:
        return as.number != 0 && !std::isnan(as.number);
    case PHON_TSTRING: // the empty string is also true
    default:
        return true;
    }
}

const char *int_to_str(char *out, int v)
{
    char buf[32], *s = out;
    unsigned int a;
    int i = 0;
    if (v < 0)
    {
        a = -v;
        *s++ = '-';
    }
    else
    {
        a = v;
    }
    while (a)
    {
        buf[i++] = (a % 10) + '0';
        a /= 10;
    }
    if (i == 0)
        buf[i++] = '0';
    while (i > 0)
        *s++ = buf[--i];
    *s = 0;
    return out;
}

double string_to_float(const char *s, char **ep)
{
    char *end;
    double n;
    const char *e = s;
    int isflt = 0;
    if (*e == '+' || *e == '-') ++e;
    while (*e >= '0' && *e <= '9') ++e;
    if (*e == '.')
    {
        ++e;
        isflt = 1;
    }
    while (*e >= '0' && *e <= '9') ++e;
    if (*e == 'e' || *e == 'E')
    {
        ++e;
        if (*e == '+' || *e == '-') ++e;
        while (*e >= '0' && *e <= '9') ++e;
        isflt = 1;
    }
    if (isflt || e - s > 9)
        n = str_to_double(s, &end);
    else
        n = strtol(s, &end, 10);
    if (end == e)
    {
        *ep = (char *) e;
        return n;
    }
    *ep = (char *) s;
    return 0;
}

/* ToString() on a number */
String number_to_string(Runtime *J, char *buf, double f)
{
    char digits[32], *p = buf, *s = digits;
    int exp, ndigits, point;

    if (f == 0) return "0";
    if (std::isnan(f)) return J->undef_string;
    if (std::isinf(f)) return f < 0 ? "-Infinity" : "Infinity";

    /* Fast case for integers. This only works assuming all integers can be
     * exactly represented by a float. This is true for 32-bit integers and
     * 64-bit floats. */
    if (f >= INT_MIN && f <= INT_MAX)
    {
        int i = (int) f;
        if ((double) i == f)
            return int_to_str(buf, i);
    }

    ndigits = grisu2(f, digits, &exp);
    point = ndigits + exp;

    if (std::signbit(f))
        *p++ = '-';

    if (point < -5 || point > 21)
    {
        *p++ = *s++;
        if (ndigits > 1)
        {
            int n = ndigits - 1;
            *p++ = '.';
            while (n--)
                *p++ = *s++;
        }
        fmt_exp(p, point - 1);
    }

    else if (point <= 0)
    {
        *p++ = '0';
        *p++ = '.';
        while (point++ < 0)
            *p++ = '0';
        while (ndigits-- > 0)
            *p++ = *s++;
        *p = 0;
    }

    else
    {
        while (ndigits-- > 0)
        {
            *p++ = *s++;
            if (--point == 0 && ndigits > 0)
                *p++ = '.';
        }
        while (point-- > 0)
            *p++ = '0';
        *p = 0;
    }

    return buf;
}

/* to_string() on a value */
String var_to_string(Runtime *J, Variant *v)
{
    switch (v->type)
    {
    default:
    case PHON_TNULL:
        return J->null_string;
    case PHON_TBOOLEAN:
        return v->as.boolean ? J->true_string : J->false_string;
    case PHON_TSTRING:
        return v->as.string;
    case PHON_TNUMBER:
    {
        char buf[32];
        return number_to_string(J, buf, v->as.number);
    }
    case PHON_TOBJECT:
        var_to_primitive(J, v, PHON_HINT_STRING);
        return var_to_string(J, v);
    }
}

/* Objects */

static Object *jsV_newboolean(Runtime *J, bool v)
{
    Object *obj = new Object(*J, PHON_CBOOLEAN, J->boolean_meta);
    obj->as.boolean = v;
    return obj;
}

static Object *jsV_newnumber(Runtime *J, double v)
{
    Object *obj = new Object(*J, PHON_CNUMBER, J->number_meta);
    obj->as.number = v;
    return obj;
}

static Object *jsV_newstring(Runtime *J, const String &v)
{
    Object *obj = new Object(*J, PHON_CSTRING, J->string_meta);
    new(&obj->as.string) String(std::move(J->internalize(v)));

    return obj;
}

/* ToObject() on a value */
Object *Runtime::to_object(Variant *v)
{
    switch (v->type)
    {
    default:
    case PHON_TNULL:
        throw raise("Type error", "cannot convert null to object");
    case PHON_TBOOLEAN:
        return jsV_newboolean(this, v->as.boolean);
    case PHON_TNUMBER:
        return jsV_newnumber(this, v->as.number);
    case PHON_TSTRING:
        return jsV_newstring(this, v->as.string);
    case PHON_TOBJECT:
        return v->as.object;
    }
}

void Runtime::new_objectx()
{
    Object *prototype = to_object(-1);
    pop(1);
    push(new Object(*this, PHON_COBJECT, prototype));
}

void Runtime::new_object()
{
    push(new Object(*this, PHON_COBJECT, object_meta));
}

void Runtime::new_list(intptr_t size)
{
    auto obj = new Object(*this, PHON_CLIST, list_meta);
    new (&obj->as.list) Array<Variant>(size, Variant());
    push(obj);
}

void Runtime::new_array(intptr_t size)
{
    auto obj = new Object(*this, PHON_CARRAY, array_meta);
    new (&obj->as.array) Array<double>(size, 0.0);
    push(obj);
}

void Runtime::new_array(intptr_t nrow, intptr_t ncol)
{
	auto obj = new Object(*this, PHON_CARRAY, array_meta);
	new (&obj->as.array) Array<double>(nrow, ncol, 0.0);
	push(obj);
}

void Runtime::new_boolean(bool v)
{
    push(jsV_newboolean(this, v));
}

void Runtime::new_number(double v)
{
    push(jsV_newnumber(this, v));
}

void Runtime::new_string(const String &v)
{
    push(jsV_newstring(this, v));
}

void create_function(Runtime *J, Function *fun, Environment *scope)
{
    Object *obj = new Object(*J, PHON_CFUNCTION, J->function_meta);
    obj->as.f.function = fun;
    obj->as.f.scope = scope;
    J->push(obj);
    {
        J->push(fun->numparams);
        J->def_field(-2, "length", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
        J->new_object();
        {
            J->copy(-2);
            J->def_field(-2, "constructor", PHON_DONTENUM);
        }
        J->def_field(-2, "meta", PHON_DONTCONF);
    }
}

void create_script(Runtime *J, Function *fun, Environment *scope)
{
    Object *obj = new Object(*J, PHON_CSCRIPT, nullptr);
    obj->as.f.function = fun;
    obj->as.f.scope = scope;
    J->push(obj);
}

void Runtime::new_native_function(native_callback_t fun, const String &name, int length)
{
    Object *obj = new Object(*this, PHON_CCFUNCTION, function_meta);
    new (&obj->as.c.name) String(name);
    new (&obj->as.c.function) native_callback_t(std::move(fun));
    obj->as.c.length = length;
    push(obj);
    {
        push(length);
        def_field(-2, "length", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
        new_object();
        {
            copy(-2);
            def_field(-2, "constructor", PHON_DONTENUM);
        }
        def_field(-2, "meta", PHON_DONTCONF);
    }
}

/* prototype -- constructor */
void Runtime::new_native_constructor(native_callback_t fun, native_callback_t con, const String &name, int length)
{
    Object *obj = new Object(*this, PHON_CCFUNCTION, function_meta);
    new (&obj->as.c.name) String(name);
    new (&obj->as.c.function) native_callback_t(std::move(fun));
    new (&obj->as.c.constructor) native_callback_t(std::move(con));
    obj->as.c.length = length;
    push(obj); /* proto obj */
    {
        push(length);
        def_field(-2, "length", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
        js_rot2(this); /* obj proto */
        copy(-2); /* obj proto obj */
        def_field(-2, "constructor", PHON_DONTENUM);
        def_field(-2, "meta", PHON_READONLY | PHON_DONTENUM | PHON_DONTCONF);
    }
}

void Runtime::new_user_data(const char *tag, std::any data, has_field_callback_t has, put_callback_t put,
                            delete_callback_t destroy)
{
    Object *prototype = nullptr;
    Object *obj;

    if (is_object(-1))
        prototype = to_object(-1);
    pop(1);

    obj = new Object(*this, PHON_CUSERDATA, prototype);
    obj->as.user.tag = tag;
    new(&obj->as.user.data) std::any(std::move(data));
    obj->as.user.has = has;
    obj->as.user.put = put;
    obj->as.user.destroy = destroy;
    push(obj);
}

void Runtime::new_user_data(const char *tag, std::any data)
{
    new_user_data(tag, std::move(data), nullptr, nullptr, nullptr);
}

void Runtime::new_user_data(Object *meta, const char *tag, std::any data)
{
    push(meta);
    new_user_data(tag, std::move(data));
}


/* Non-trivial operations on values. These are implemented using the stack. */

int js_instanceof(Runtime *J)
{
    Object *O, *V;

    if (!J->is_callable(-1))
        throw J->raise("Type error", "instanceof: invalid operand");

    if (!J->is_object(-2))
        return 0;

    J->get_field(-1, "meta");
    if (!J->is_object(-1))
        throw J->raise("Type error", "instanceof: 'prototype' field is not an object");
    O = J->to_object(-1);
    J->pop(1);

    V = J->to_object(-2);
    while (V)
    {
        V = V->prototype;
        if (O == V)
            return 1;
    }

    return 0;
}

void js_concat(Runtime *J)
{
	auto str = J->to_string(-2);
	str.append(J->to_string(-1));
	J->pop(2);
	J->push(std::move(str));
#if 0
    var_to_primitive(J, -2, PHON_HINT_NONE);
    var_to_primitive(J, -1, PHON_HINT_NONE);

    if (J->is_string(-2) || J->is_string(-1))
    {
        auto str = J->to_string(-2);
        str.append(J->to_string(-1));
        J->pop(2);
        J->push(std::move(str));
    }
    else
    {
        double x = J->to_number(-2);
        double y = J->to_number(-1);
        J->pop(2);
        J->push(x + y);
    }
#endif
}

int js_compare(Runtime *J, int *okay)
{
    var_to_primitive(J, -2, PHON_HINT_NUMBER);
    var_to_primitive(J, -1, PHON_HINT_NUMBER);

    *okay = 1;
    if (J->is_string(-2) && J->is_string(-1))
    {
        return J->to_string(-2).compare(J->to_string(-1));
    }
    else
    {
        double x = J->to_number(-2);
        double y = J->to_number(-1);
        if (std::isnan(x) || std::isnan(y))
            *okay = 0;
        return x < y ? -1 : x > y ? 1 : 0;
    }
}

int js_equal(Runtime *J)
{
    Variant *x = get_variant(J, -2);
    Variant *y = get_variant(J, -1);

    return *x == *y;
}

Variant::Variant(const Variant &other)
{
    this->raw_copy(other);
    this->retain();
}

Variant::Variant(Variant &&other) noexcept
{
    this->raw_copy(other);
    other.undef();
}

void Variant::undef()
{
    type = PHON_TNULL;
}

Variant &Variant::operator=(const Variant &other)
{
    if (this != &other)
    {
        this->release();
        this->raw_copy(other);
        this->retain();
    }

    return *this;
}

Variant &Variant::operator=(Variant &&other) noexcept
{
    this->release();
    this->raw_copy(other);
    other.undef();

    return *this;
}

void Variant::swap(Variant &other) noexcept
{
    this->as.swap(other.as);
    std::swap(this->type, other.type);
}

void Variant::retain()
{
    if (is_string())
        as.string.impl->retain();
//    else if (is_object())
//        as.object->retain();
}

void Variant::release()
{
    if (is_string())
        as.string.~String();
//    else if (is_object())
//        as.object->release();
}

void Variant::raw_copy(const Variant &src)
{
    std::memcpy(this, &src, sizeof(Variant));
}

bool Variant::is_list() const
{
    return is_object() && as.object->type == PHON_CLIST;
}

bool Variant::is_regex() const
{
    return is_object() && as.object->type == PHON_CREGEX;
}

bool Variant::is_user_data() const
{
    return is_object() && as.object->type == PHON_CUSERDATA;
}

bool Variant::is_user_data(const char *tag) const
{
    if (is_user_data())
    {
        return !strcmp(tag, as.object->as.user.tag);
    }

    return false;
}

bool Variant::is_error() const
{
    return is_object() && as.object->type == PHON_CERROR;
}

bool Variant::is_callable() const
{
    if (is_object())
    {
        auto t = as.object->type;
        return t == PHON_CFUNCTION || t == PHON_CSCRIPT || t == PHON_CCFUNCTION;
    }

    return false;
}

bool Variant::is_file() const
{
    return is_object() && as.object->type == PHON_CFILE;
}

bool Variant::operator<(const Variant &other) const
{
    if (this->type != other.type) {
        throw error("cannot compare values with different types");
    }

    switch (type)
    {
    case PHON_TBOOLEAN:
        return as.boolean < other.as.boolean;
    case PHON_TNUMBER:
        return as.number < other.as.number;
    case PHON_TSTRING:
        return as.string < other.as.string;
    case PHON_TNULL:
        throw error("null values cannot be ordered");
    default:
        // Use address
        return as.object < other.as.object;
    }
}

bool Variant::operator==(const Variant &other) const
{
    if (this->type != other.type) {
        return false;
    }

    switch (type)
    {
    case PHON_TBOOLEAN:
        return as.boolean == other.as.boolean;
    case PHON_TNUMBER:
    {
        auto n1 = this->as.number;
        auto n2 = other.as.number;
        // (undefined == undefined) is true
        return (std::isnan(n1) && std::isnan(n2)) || equal(n1, n2);
    }
        return as.number == other.as.number;
    case PHON_TSTRING:
        return as.string == other.as.string;
    case PHON_TNULL:
        return true;
    default:
        if (as.object->type == PHON_CLIST)
            return as.object->as.list == other.as.object->as.list;
        return as.object == other.as.object;
    }
}

double Variant::to_number() const
{
    if (is_number())
        return as.number;

    throw std::runtime_error("[Type error] value is not a number");
}

bool Variant::is_array() const
{
	return is_object() && as.object->type == PHON_CARRAY;
}

void Variant::Storage::swap(Variant::Storage &other)
{
    Storage tmp;
    std::memcpy(&tmp, this, sizeof(Storage));
    std::memcpy(this, &other, sizeof(Storage));
    std::memcpy(&other, &tmp, sizeof(Storage));
}


//---------------------------------------------------------------------------------------------------------------------

Value::Value(Runtime &rt, int idx) :
    variant(rt.get(idx))
{
    // Don't register primitive types.
    if (variant.is_object())
    {
        rt.copy(idx);
        this->handle = rt.ref();
        this->rt = &rt;
    }
    else
    {
        this->rt = nullptr;
    }
}

Value::Value(Value &&other) noexcept :
    variant(std::move(other.variant)), handle(std::move(other.handle)), rt(other.rt)
{
    other.rt = nullptr;
}

Value::Value(const Value &other) :
    variant(other.variant), handle(other.handle), rt(other.rt)
{

}

Value::~Value()
{
    if (rt)
    {
        rt->unref(handle);
    }
}

} // namespace phonometrica