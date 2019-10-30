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

#ifndef PHONOMETRICA_OBJECT_HPP
#define PHONOMETRICA_OBJECT_HPP

#include <cstdint>
#include <unordered_map>
#include <phon/regex.hpp>
#include <phon/file.hpp>
#include <phon/runtime/common.hpp>
#include <phon/runtime/variant.hpp>
#include <phon/utils/matrix.hpp>

namespace phonometrica {

class Runtime;
struct Function;
struct Environment;

class Field final
{
public:

    Field() = default;

    ~Field() = default;

    String name;
    int atts = 0;
    Variant value;
    Object *getter = nullptr;
    Object *setter = nullptr;
};


using FieldMap = std::unordered_map<String, Field>;

//---------------------------------------------------------------------------------------------------------------------

class Object final /* : public Countable<Object, int32_t> */
{
public:

    Object(Runtime &rt, ClassTag type, Object *prototype);

    ~Object();

    Field *get_own_field(Runtime &, const String &name);

    Field *get_field(Runtime &, const String &name, bool *own);

    Field *get_field(Runtime &, const String &name);

    Field *set_field(Runtime &rt, const String &name);

    void del_field(Runtime &rt, const String &name);

    Object *new_iterator(Runtime &rt);

    std::optional<Variant> next_iterator(Runtime &rt);

    void resize_list(Runtime &rt, int new_size);

    String to_string();

public:

    ClassTag type;
    bool extensible;
    uint8_t gcmark;
    uint32_t version = 0;
    FieldMap fields;
    Object *gcnext; // *gcprev;
    //Environment *runtime; // environment the object was created from (for destruction)
    Object *prototype;

    union Storage
    {
        Storage() { }
        ~Storage() { }

        bool boolean;
        double number;
        String string;
        Array<double> array;
        struct
        {
            Function *function;
            Environment *scope;
        } f;
        struct
        {
            String name;
            native_callback_t function;
            native_callback_t constructor;
            int length;
        } c;
        Regex regex;
        File file;
        Array<Variant> list;
        struct
        {
            Object *target;
            intptr_t index;
            std::any data;   // type erased C++ iterator
        } iter;
        struct
        {
            const char *tag;
            std::any data;
            has_field_callback_t has;
            put_callback_t put;
            delete_callback_t destroy;
            finalize_callback_t finalize;
        } user;
    } as;
};


} // namespace phonometrica

#endif // PHONOMETRICA_OBJECT_HPP
