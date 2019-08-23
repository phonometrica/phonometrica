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

#ifndef PHONOMETRICA_VARIANT_HPP
#define PHONOMETRICA_VARIANT_HPP

#include <phon/string.hpp>

namespace phonometrica {

class Field;
class Object;
class Environment;
struct Variant;



//---------------------------------------------------------------------------------------------------------------------

/* run.cpp */


Variant *get_variant(Environment *J, int idx);

void var_to_primitive(Environment *J, int idx, int hint);

Object *var_to_object(Environment *J, int idx);


/* variant.cpp */


String var_to_string(Environment *J, Variant *v);

Object *var_to_object(Environment *J, Variant *v);

void var_to_primitive(Environment *J, Variant *v, int preferred);

const char *int_to_str(char *buf, int a);

double string_to_float(const char *s, char **ep);

intptr_t number_to_integer(double n);

int number_to_int32(double n);

unsigned int number_to_uint32(double n);

short number_to_int16(double n);

unsigned short number_to_uint16(double n);

String number_to_string(Environment *J, char *buf, double number);

double string_to_number(Environment *J, const String &string);


/* dump.cpp */
void dump_object(Environment *J, Object *obj);

void dump_variant(Environment *J, const Variant &v);


//---------------------------------------------------------------------------------------------------------------------

/* Hint to ToPrimitive() */
enum
{
    PHON_HINT_NONE,
    PHON_HINT_NUMBER,
    PHON_HINT_STRING
};

enum TypeTag
{
    PHON_TNULL,
    PHON_TBOOLEAN,
    PHON_TNUMBER,
    PHON_TSTRING,
    PHON_TOBJECT,
};

enum ClassTag : uint8_t
{
    PHON_COBJECT,
    PHON_CLIST,
    PHON_CFUNCTION,
    PHON_CSCRIPT,    /* function created from global/eval code */
    PHON_CCFUNCTION, /* built-in function */
    PHON_CERROR,
    PHON_CBOOLEAN,
    PHON_CNUMBER,
    PHON_CSTRING,
    PHON_CREGEX,
    PHON_CDATE,
    PHON_CFILE,
    PHON_CMATH,
    PHON_CJSON,
    PHON_CSYSTEM,
    PHON_CITERATOR,
    PHON_CUSERDATA,
};

//---------------------------------------------------------------------------------------------------------------------

// Polymorphic container used internally to store values.

struct Variant final
{
    Variant() noexcept { undef(); }

    Variant(const Variant &other);

    Variant(Variant &&other) noexcept;

    Variant(Object *value)
    {
        type = PHON_TOBJECT;
        as.object = value;
    }

    Variant(String value)
    {
        set_string(std::move(value));
    }

    Variant(double value)
    {
        set_number(value);
    }

    ~Variant() { release(); undef(); }

    // Set Variant to null (unsafe).
    void undef();

    Variant &operator=(const Variant &other);

    Variant &operator=(Variant &&other) noexcept;

    void swap(Variant &other) noexcept;

    void retain();

    void release();

    bool is_null() const { return type == PHON_TNULL; }

    bool is_boolean() const { return type == PHON_TBOOLEAN; }

    bool is_number() const { return type == PHON_TNUMBER; }

    bool is_string() const { return type == PHON_TSTRING;}

    bool is_object() const { return type == PHON_TOBJECT; }

    bool is_list() const;

    bool is_regex() const;

    bool is_file() const;

    bool is_user_data(const char *tag) const;

    bool is_user_data() const;

    bool is_error() const;

    bool is_callable() const;

    bool to_boolean() const;

    double to_number() const;

    String to_string(Environment &env) { return var_to_string(&env, this); }

    const String &as_string() const { return as.string; }

    bool operator<(const Variant &other) const;

    bool operator==(const Variant &other) const;

    bool operator!=(const Variant &other) const { return ! (*this == other); }

    void raw_copy(const Variant &src);

    void set_boolean(bool value)
    {
        type = PHON_TBOOLEAN;
        as.boolean = value;
    }

    void set_number(double value)
    {
        type = PHON_TNUMBER;
        as.number = value;
    }

    void set_string(String value)
    {
        type = PHON_TSTRING;
        new (&as.string) String(std::move(value));
    }

    union Storage
    {
        Storage() { }
        ~Storage() { }

        void swap(Storage &other);

        double number;
        bool boolean;
        String string;
        Object *object;
    } as;

    TypeTag type;
};


//---------------------------------------------------------------------------------------------------------------------

// A Value is a wrapper around a Variant to be used *outside* of the runtime. This automatically (un)references the
// variant to ensure that the GC is aware of its existence.

class Value final
{
public:

    explicit Value(Environment &env, int idx = -1);

    Value(const Value &other);

    Value(Value &&other) noexcept;

    ~Value();

    Variant &get() { return variant; }

    const Variant &get() const { return variant; }

    Variant *operator->() { return &variant; }

    const Variant *operator->() const { return &variant; }

    Variant &operator *() { return variant; }

    const Variant &operator*() const { return variant; }

private:

    Variant variant;
    String handle;
    Environment *env;
};


} // namespace phonometrica


#endif // PHONOMETRICA_VARIANT_HPP
