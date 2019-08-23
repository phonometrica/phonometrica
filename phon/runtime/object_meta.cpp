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
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

static void jsB_new_Object(Environment &env)
{
    if (env.is_null(1))
        env.new_object();
    else
        env.push(env.to_object(1));
}

static void jsB_Object(Environment &env)
{
    if (env.is_null(1))
        env.new_object();
    else
        env.push(env.to_object(1));
}

static void object_to_string(Environment &env)
{
    if (env.is_null(0))
        env.push("[object Undefined]");
    else if (env.is_null(0))
        env.push("[object Null]");
    else
    {
        Object *self = env.to_object(0);
        switch (self->type)
        {
        case PHON_COBJECT:
            env.push("[object Object]");
            break;
        case PHON_CLIST:
            env.push("[object List]");
            break;
        case PHON_CFUNCTION:
            env.push("[object Function]");
            break;
        case PHON_CSCRIPT:
            env.push("[object Function]");
            break;
        case PHON_CCFUNCTION:
            env.push("[object Function]");
            break;
        case PHON_CERROR:
            env.push("[object Error]");
            break;
        case PHON_CBOOLEAN:
            env.push("[object Boolean]");
            break;
        case PHON_CNUMBER:
            env.push("[object Number]");
            break;
        case PHON_CSTRING:
            env.push("[object String]");
            break;
        case PHON_CREGEX:
            env.push("[object Regex]");
            break;
        case PHON_CFILE:
            env.push("[object File]");
            break;
        case PHON_CDATE:
            env.push("[object Date]");
            break;
        case PHON_CMATH:
            env.push("[module math]");
            break;
        case PHON_CJSON:
            env.push("[module json]");
        case PHON_CSYSTEM:
            env.push("[module system]");
            break;
        case PHON_CITERATOR:
            env.push("[Iterator]");
            break;
        case PHON_CUSERDATA:
            env.push("[object ");
            env.push(self->as.user.tag);
            js_concat(&env);
            env.push("]");
            js_concat(&env);
            break;
        }
    }
}

static void object_to_value(Environment &env)
{
    env.copy(0);
}

static void object_has_field(Environment &env)
{
    Object *self = env.to_object(0);
    auto name = env.to_string(1);
    Field *ref = self->get_own_field(env, name);
    env.push_boolean(ref != nullptr);
}

static void object_is_prototype_of(Environment &env)
{
    Object *self = env.to_object(0);
    if (env.is_object(1))
    {
        Object *V = env.to_object(1);
        do
        {
            V = V->prototype;
            if (V == self)
            {
                env.push_boolean(true);
                return;
            }
        } while (V);
    }
    env.push_boolean(false);
}

static void object_field_is_enumerable(Environment &env)
{
    Object *self = env.to_object(0);
    auto name = env.to_string(1);
    Field *ref = self->get_own_field(env, name);
    env.push_boolean(ref && !(ref->atts & PHON_DONTENUM));
}

static void object_get_prototype_of(Environment &env)
{
    Object *obj;
    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");
    obj = env.to_object(1);
    if (obj->prototype)
        env.push(obj->prototype);
    else
        env.push_null();
}

static void object_get_own_field_descriptor(Environment &env)
{
    Object *obj;
    Field *ref;
    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");
    obj = env.to_object(1);
    ref = obj->get_field(env, env.to_string(2));
    if (!ref)
        env.push_null();
    else
    {
        env.new_object();
        if (!ref->getter && !ref->setter)
        {
            env.push(ref->value);
            env.set_field(-2, "value");
            env.push_boolean(!(ref->atts & PHON_READONLY));
            env.set_field(-2, "writable");
        }
        else
        {
            if (ref->getter)
                env.push(ref->getter);
            else
                env.push_null();
            env.set_field(-2, "get");
            if (ref->setter)
                env.push(ref->setter);
            else
                env.push_null();
            env.set_field(-2, "set");
        }
        env.push_boolean(!(ref->atts & PHON_DONTENUM));
        env.set_field(-2, "enumerable");
        env.push_boolean(!(ref->atts & PHON_DONTCONF));
        env.set_field(-2, "configurable");
    }
}

static void object_get_own_field_names(Environment &env)
{
    Object *obj;
    int k;

    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");
    obj = env.to_object(1);

    env.new_list();
    int i = 0;

    for (auto &f : obj->fields)
    {
        env.push(f.second.name);
        js_setindex(&env, -2, i++);
    }

    if (obj->type == PHON_CLIST)
    {
        env.push("length");
        js_setindex(&env, -2, i++);
    }

    if (obj->type == PHON_CSTRING)
    {
        env.push("length");
        js_setindex(&env, -2, i++);
        for (k = 0; k < obj->as.string.size(); ++k)
        {
            env.push(k);
            js_setindex(&env, -2, i++);
        }
    }

    if (obj->type == PHON_CREGEX)
    {
        env.push("source");
        js_setindex(&env, -2, i++);
        env.push("global");
        js_setindex(&env, -2, i++);
        env.push("ignoreCase");
        js_setindex(&env, -2, i++);
        env.push("multiline");
        js_setindex(&env, -2, i++);
        env.push("lastIndex");
        js_setindex(&env, -2, i++);
    }
}

static void ToFieldDescriptor(Environment *J, Object *obj, const String &name, Object *desc)
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

static void object_define_field(Environment &env)
{
    if (!env.is_object(1)) throw env.raise("Type error", "not an object");
    if (!env.is_object(3)) throw env.raise("Type error", "not an object");
    ToFieldDescriptor(&env, env.to_object(1), env.to_string(2), env.to_object(3));
    env.copy(1);
}

static void object_define_fields(Environment &env)
{
    Object *props;

    if (!env.is_object(1)) throw env.raise("Type error", "not an object");
    if (!env.is_object(2)) throw env.raise("Type error", "not an object");

    props = env.to_object(2);
    for (auto &f : props->fields)
    {
        auto ref = &f.second;
        if (!(ref->atts & PHON_DONTENUM))
        {
            env.push(ref->value);
            ToFieldDescriptor(&env, env.to_object(1), ref->name, env.to_object(-1));
            env.pop(1);
        }
    }

    env.copy(1);
}

static void object_create(Environment &env)
{
    Object *obj;
    Object *proto;
    Object *props;

    if (env.is_object(1))
        proto = env.to_object(1);
    else if (env.is_null(1))
        proto = nullptr;
    else
        throw env.raise("Type error", "not an object or null");

    obj = new Object(env, PHON_COBJECT, proto);
    env.push(obj);

    if (env.is_defined(2))
    {
        if (!env.is_object(2))
            throw env.raise("Type error", "not an object");
        props = env.to_object(2);

        for (auto &f : props->fields)
        {
            auto ref = &f.second;
            if (!(ref->atts & PHON_DONTENUM))
            {
                if (ref->value.type != PHON_TOBJECT)
                    throw env.raise("Type error", "not an object");
                ToFieldDescriptor(&env, obj, ref->name, ref->value.as.object);
            }
        }
    }
}

static void object_keys(Environment &env)
{
    if (!env.is_object(0))
        throw env.raise("Type error", "not an object");

    auto obj = env.to_object(0);
    Array<Variant> keys;
    keys.reserve(obj->fields.size());

    for (auto &val : obj->fields)
    {
        keys.append(val.first);
    }
    env.push(std::move(keys));
}

static void object_values(Environment &env)
{
    if (!env.is_object(0))
        throw env.raise("Type error", "not an object");

    auto obj = env.to_object(0);
    Array<Variant> values;
    values.reserve(obj->fields.size());

    for (auto &val : obj->fields)
    {
        env.get_field(obj, val.first);
        values.emplace_back(std::move(env.get(-1)));
        env.pop();
    }
    env.push(std::move(values));
}

static void object_get(Environment &env)
{
    if (!env.is_object(0))
        throw env.raise("Type error", "not an object");

    auto key = env.to_string(1);

    if (env.has_field(0, key))
    {
        return; // leave value on top of the stack
    }

    if (env.arg_count() > 1)
    {
        env.copy(-1);
    }
    else
    {
        env.push_null();
    }
}

static void object_contains(Environment &env)
{
    if (!env.is_object(0))
        throw env.raise("Type error", "not an object");
    auto obj = env.to_object(0);
    auto key = env.to_string(1);
    auto it = obj->fields.find(key);
    env.push_boolean(it != obj->fields.end());
}

static void object_prevent_extensions(Environment &env)
{
    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");
    env.to_object(1)->extensible = false;
    env.copy(1);
}

static void object_is_extensible(Environment &env)
{
    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");
    env.push_boolean(env.to_object(1)->extensible);
}

static void object_seal(Environment &env)
{
    Object *obj;

    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");

    obj = env.to_object(1);
    obj->extensible = false;

    for (auto &f : obj->fields)
    {
        f.second.atts |= PHON_DONTCONF;
    }

    env.copy(1);
}

static void object_is_sealed(Environment &env)
{
    Object *obj;

    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");

    obj = env.to_object(1);
    if (obj->extensible)
    {
        env.push_boolean(false);
        return;
    }

    for (auto &f : obj->fields)
    {
        if (!(f.second.atts & PHON_DONTCONF))
        {
            env.push_boolean(false);
            return;
        }
    }

    env.push_boolean(true);
}

static void object_freeze(Environment &env)
{
    Object *obj;

    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");

    obj = env.to_object(1);
    obj->extensible = false;

    for (auto &f : obj->fields)
    {
        f.second.atts |= PHON_READONLY | PHON_DONTCONF;
    }

    env.copy(1);
}

static void object_is_frozen(Environment &env)
{
    Object *obj;

    if (!env.is_object(1))
        throw env.raise("Type error", "not an object");

    obj = env.to_object(1);

    for (auto &f : obj->fields)
    {
        auto ref = &f.second;
        if (!(ref->atts & PHON_READONLY) || !(ref->atts & PHON_DONTCONF))
        {
            env.push_boolean(false);
            return;
        }
    }

    env.push_boolean(!obj->extensible);
}

static void object_is_empty(Environment &env)
{
    auto obj = env.to_object(0);
    env.push_boolean(obj->fields.empty());
}

void Environment::init_object()
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