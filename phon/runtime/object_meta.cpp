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
#include <phon/runtime/object.hpp>

namespace phonometrica {

static void jsB_new_Object(Runtime &rt)
{
    if (rt.is_null(1))
        rt.new_object();
    else
        rt.push(rt.to_object(1));
}

static void jsB_Object(Runtime &rt)
{
    if (rt.is_null(1))
        rt.new_object();
    else
        rt.push(rt.to_object(1));
}

static void object_to_string(Runtime &rt)
{
    if (rt.is_null(0))
        rt.push("[object Undefined]");
    else if (rt.is_null(0))
        rt.push("[object Null]");
    else
    {
        Object *self = rt.to_object(0);
        switch (self->type)
        {
        case PHON_COBJECT:
            rt.push("[object Object]");
            break;
        case PHON_CLIST:
            rt.push("[object List]");
            break;
        case PHON_CFUNCTION:
            rt.push("[object Function]");
            break;
        case PHON_CSCRIPT:
            rt.push("[object Function]");
            break;
        case PHON_CCFUNCTION:
            rt.push("[object Function]");
            break;
        case PHON_CERROR:
            rt.push("[object Error]");
            break;
        case PHON_CBOOLEAN:
            rt.push("[object Boolean]");
            break;
        case PHON_CNUMBER:
            rt.push("[object Number]");
            break;
        case PHON_CSTRING:
            rt.push("[object String]");
            break;
        case PHON_CREGEX:
            rt.push("[object Regex]");
            break;
        case PHON_CFILE:
            rt.push("[object File]");
            break;
        case PHON_CDATE:
            rt.push("[object Date]");
            break;
        case PHON_CMATH:
            rt.push("[module math]");
            break;
        case PHON_CJSON:
            rt.push("[module json]");
        case PHON_CSYSTEM:
            rt.push("[module system]");
            break;
        case PHON_CITERATOR:
            rt.push("[Iterator]");
            break;
        case PHON_CUSERDATA:
            rt.push("[object ");
            rt.push(self->as.user.tag);
            js_concat(&rt);
            rt.push("]");
            js_concat(&rt);
            break;
        }
    }
}

static void object_to_value(Runtime &rt)
{
    rt.copy(0);
}

static void object_has_field(Runtime &rt)
{
    Object *self = rt.to_object(0);
    auto name = rt.to_string(1);
    Field *ref = self->get_own_field(rt, name);
    rt.push_boolean(ref != nullptr);
}

static void object_is_prototype_of(Runtime &rt)
{
    Object *self = rt.to_object(0);
    if (rt.is_object(1))
    {
        Object *V = rt.to_object(1);
        do
        {
            V = V->prototype;
            if (V == self)
            {
                rt.push_boolean(true);
                return;
            }
        } while (V);
    }
    rt.push_boolean(false);
}

static void object_field_is_enumerable(Runtime &rt)
{
    Object *self = rt.to_object(0);
    auto name = rt.to_string(1);
    Field *ref = self->get_own_field(rt, name);
    rt.push_boolean(ref && !(ref->atts & PHON_DONTENUM));
}

static void object_get_prototype_of(Runtime &rt)
{
    Object *obj;
    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");
    obj = rt.to_object(1);
    if (obj->prototype)
        rt.push(obj->prototype);
    else
        rt.push_null();
}

static void object_get_own_field_descriptor(Runtime &rt)
{
    Object *obj;
    Field *ref;
    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");
    obj = rt.to_object(1);
    ref = obj->get_field(rt, rt.to_string(2));
    if (!ref)
        rt.push_null();
    else
    {
        rt.new_object();
        if (!ref->getter && !ref->setter)
        {
            rt.push(ref->value);
            rt.set_field(-2, "value");
            rt.push_boolean(!(ref->atts & PHON_READONLY));
            rt.set_field(-2, "writable");
        }
        else
        {
            if (ref->getter)
                rt.push(ref->getter);
            else
                rt.push_null();
            rt.set_field(-2, "get");
            if (ref->setter)
                rt.push(ref->setter);
            else
                rt.push_null();
            rt.set_field(-2, "set");
        }
        rt.push_boolean(!(ref->atts & PHON_DONTENUM));
        rt.set_field(-2, "enumerable");
        rt.push_boolean(!(ref->atts & PHON_DONTCONF));
        rt.set_field(-2, "configurable");
    }
}

static void object_get_own_field_names(Runtime &rt)
{
    Object *obj;
    int k;

    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");
    obj = rt.to_object(1);

    rt.new_list();
    int i = 0;

    for (auto &f : obj->fields)
    {
        rt.push(f.second.name);
        js_setindex(&rt, -2, i++);
    }

    if (obj->type == PHON_CLIST)
    {
        rt.push("length");
        js_setindex(&rt, -2, i++);
    }

    if (obj->type == PHON_CSTRING)
    {
        rt.push("length");
        js_setindex(&rt, -2, i++);
        for (k = 0; k < obj->as.string.size(); ++k)
        {
            rt.push(k);
            js_setindex(&rt, -2, i++);
        }
    }

    if (obj->type == PHON_CREGEX)
    {
        rt.push("source");
        js_setindex(&rt, -2, i++);
        rt.push("global");
        js_setindex(&rt, -2, i++);
        rt.push("ignoreCase");
        js_setindex(&rt, -2, i++);
        rt.push("multiline");
        js_setindex(&rt, -2, i++);
        rt.push("lastIndex");
        js_setindex(&rt, -2, i++);
    }
}

static void ToFieldDescriptor(Runtime *J, Object *obj, const String &name, Object *desc)
{
    int haswritable = 0;
    int hasvalue = 0;
    int enumerable = 0;
    int configurable = 0;
    int writable = 0;
    int atts = 0;

    J->push(obj);
    J->push(desc);

    if (J->has_field(-1, "writable"))
    {
        haswritable = 1;
        writable = J->to_boolean(-1);
        J->pop(1);
    }
    if (J->has_field(-1, "enumerable"))
    {
        enumerable = J->to_boolean(-1);
        J->pop(1);
    }
    if (J->has_field(-1, "configurable"))
    {
        configurable = J->to_boolean(-1);
        J->pop(1);
    }
    if (J->has_field(-1, "value"))
    {
        hasvalue = 1;
        J->set_field(-3, name);
    }

    if (!writable) atts |= PHON_READONLY;
    if (!enumerable) atts |= PHON_DONTENUM;
    if (!configurable) atts |= PHON_DONTCONF;

    if (J->has_field(-1, "get"))
    {
        if (haswritable || hasvalue)
            throw J->raise("Type error", "value/writable and get/set attributes are exclusive");
    }
    else
    {
        J->push_null();
    }

    if (J->has_field(-2, "set"))
    {
        if (haswritable || hasvalue)
            throw J->raise("Type error", "value/writable and get/set attributes are exclusive");
    }
    else
    {
        J->push_null();
    }

    J->def_accessor(-4, name, atts);
    J->pop(2);
}

static void object_define_field(Runtime &rt)
{
    if (!rt.is_object(1)) throw rt.raise("Type error", "not an object");
    if (!rt.is_object(3)) throw rt.raise("Type error", "not an object");
    ToFieldDescriptor(&rt, rt.to_object(1), rt.to_string(2), rt.to_object(3));
    rt.copy(1);
}

static void object_define_fields(Runtime &rt)
{
    Object *props;

    if (!rt.is_object(1)) throw rt.raise("Type error", "not an object");
    if (!rt.is_object(2)) throw rt.raise("Type error", "not an object");

    props = rt.to_object(2);
    for (auto &f : props->fields)
    {
        auto ref = &f.second;
        if (!(ref->atts & PHON_DONTENUM))
        {
            rt.push(ref->value);
            ToFieldDescriptor(&rt, rt.to_object(1), ref->name, rt.to_object(-1));
            rt.pop(1);
        }
    }

    rt.copy(1);
}

static void object_create(Runtime &rt)
{
    Object *obj;
    Object *proto;
    Object *props;

    if (rt.is_object(1))
        proto = rt.to_object(1);
    else if (rt.is_null(1))
        proto = nullptr;
    else
        throw rt.raise("Type error", "not an object or null");

    obj = new Object(rt, PHON_COBJECT, proto);
    rt.push(obj);

    if (rt.is_defined(2))
    {
        if (!rt.is_object(2))
            throw rt.raise("Type error", "not an object");
        props = rt.to_object(2);

        for (auto &f : props->fields)
        {
            auto ref = &f.second;
            if (!(ref->atts & PHON_DONTENUM))
            {
                if (ref->value.type != PHON_TOBJECT)
                    throw rt.raise("Type error", "not an object");
                ToFieldDescriptor(&rt, obj, ref->name, ref->value.as.object);
            }
        }
    }
}

static void object_keys(Runtime &rt)
{
    if (!rt.is_object(0))
        throw rt.raise("Type error", "not an object");

    auto obj = rt.to_object(0);
    Array<Variant> keys;
    keys.reserve(obj->fields.size());

    for (auto &val : obj->fields)
    {
        keys.append(val.first);
    }
    rt.push(std::move(keys));
}

static void object_values(Runtime &rt)
{
    if (!rt.is_object(0))
        throw rt.raise("Type error", "not an object");

    auto obj = rt.to_object(0);
    Array<Variant> values;
    values.reserve(obj->fields.size());

    for (auto &val : obj->fields)
    {
        rt.get_field(obj, val.first);
        values.emplace_back(std::move(rt.get(-1)));
        rt.pop();
    }
    rt.push(std::move(values));
}

static void object_get(Runtime &rt)
{
    if (!rt.is_object(0))
        throw rt.raise("Type error", "not an object");

    auto key = rt.to_string(1);

    if (rt.has_field(0, key))
    {
        return; // leave value on top of the stack
    }

    if (rt.arg_count() > 1)
    {
        rt.copy(-1);
    }
    else
    {
        rt.push_null();
    }
}

static void object_contains(Runtime &rt)
{
    if (!rt.is_object(0))
        throw rt.raise("Type error", "not an object");
    auto obj = rt.to_object(0);
    auto key = rt.to_string(1);
    auto it = obj->fields.find(key);
    rt.push_boolean(it != obj->fields.end());
}

static void object_prevent_extensions(Runtime &rt)
{
    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");
    rt.to_object(1)->extensible = false;
    rt.copy(1);
}

static void object_is_extensible(Runtime &rt)
{
    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");
    rt.push_boolean(rt.to_object(1)->extensible);
}

static void object_seal(Runtime &rt)
{
    Object *obj;

    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");

    obj = rt.to_object(1);
    obj->extensible = false;

    for (auto &f : obj->fields)
    {
        f.second.atts |= PHON_DONTCONF;
    }

    rt.copy(1);
}

static void object_is_sealed(Runtime &rt)
{
    Object *obj;

    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");

    obj = rt.to_object(1);
    if (obj->extensible)
    {
        rt.push_boolean(false);
        return;
    }

    for (auto &f : obj->fields)
    {
        if (!(f.second.atts & PHON_DONTCONF))
        {
            rt.push_boolean(false);
            return;
        }
    }

    rt.push_boolean(true);
}

static void object_freeze(Runtime &rt)
{
    Object *obj;

    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");

    obj = rt.to_object(1);
    obj->extensible = false;

    for (auto &f : obj->fields)
    {
        f.second.atts |= PHON_READONLY | PHON_DONTCONF;
    }

    rt.copy(1);
}

static void object_is_frozen(Runtime &rt)
{
    Object *obj;

    if (!rt.is_object(1))
        throw rt.raise("Type error", "not an object");

    obj = rt.to_object(1);

    for (auto &f : obj->fields)
    {
        auto ref = &f.second;
        if (!(ref->atts & PHON_READONLY) || !(ref->atts & PHON_DONTCONF))
        {
            rt.push_boolean(false);
            return;
        }
    }

    rt.push_boolean(!obj->extensible);
}

static void object_is_empty(Runtime &rt)
{
    auto obj = rt.to_object(0);
    rt.push_boolean(obj->fields.empty());
}

void Runtime::init_object()
{
    push(object_meta);
    {
        // Object.meta.length is handled in Environment::has_field()
        add_method("Object.meta.is_empty", object_is_empty, 0);
        add_method("Object.meta.to_string", object_to_string, 0);
        add_method("Object.meta.to_value", object_to_value, 0);
        add_method("Object.meta.has_field", object_has_field, 1);
        add_method("Object.meta.is_prototype_of", object_is_prototype_of, 1);
        add_method("Object.meta.field_is_enumerable", object_field_is_enumerable, 1);
        add_method("Object.meta.keys", object_keys, 0);
        add_method("Object.meta.values", object_values, 0);
        add_method("Object.meta.get", object_get, 0);
        add_method("Object.meta.contains", object_contains, 1);
    }
    new_native_constructor(jsB_Object, jsB_new_Object, "Object", 1);
    {
        /* ES5 */
        add_method("Object.get_prototype_of", object_get_prototype_of, 1);
        add_method("Object.get_own_field_descriptor", object_get_own_field_descriptor, 2);
        add_method("Object.get_own_field_names", object_get_own_field_names, 1);
        add_method("Object.create", object_create, 2);
        add_method("Object.define_field", object_define_field, 3);
        add_method("Object.define_fields", object_define_fields, 2);
        add_method("Object.seal", object_seal, 1);
        add_method("Object.freeze", object_freeze, 1);
        add_method("Object.prevent_extensions", object_prevent_extensions, 1);
        add_method("Object.is_sealed", object_is_sealed, 1);
        add_method("Object.is_frozen", object_is_frozen, 1);
        add_method("Object.is_extensible", object_is_extensible, 1);

    }
    def_global("Object", PHON_DONTENUM);
}

} // namespace phonometrica