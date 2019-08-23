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

#include <iostream>
#include <string>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/run.hpp>
#include <phon/runtime/environment.hpp>
#include "environment.hpp"
#include "object.hpp"


namespace phonometrica {

static void jsR_run(Environment *J, Function *F);

/* Push values on stack */

// TODO: remove stack macros (low priority)
#define TOP (J->top)
#define BOT (J->bot)

#define CHECKSTACK(n) if (TOP + n >= J->stack + PHON_STACKSIZE) throw std::runtime_error("stack overflow");

void Environment::push(const Variant &v)
{
    check_stack(1);
    *top = v;
    ++top;
}

void Environment::push(Variant &&v)
{
    check_stack(1);
    *top = std::move(v);
    ++top;
}

void Environment::push_null()
{
    check_stack(1);
    top->undef();
    ++top;
}

void Environment::push_undefined()
{
    push(std::nan(""));
}

void Environment::push_boolean(bool value)
{
    check_stack(1);
    top->set_boolean(value);
    ++top;
}

void Environment::push(double v)
{
    check_stack(1);
    top->set_number(v);
    ++top;
}

void Environment::push(String &&v)
{
    check_stack(1);
    top->set_string(std::move(v));
    ++top;
}

void Environment::push(const char *v, intptr_t n)
{
    push(String(v, n));
}

void Environment::push(const String &v)
{
    check_stack(1);
    top->set_string(v);
    ++top;
}

void Environment::push(Object *v)
{
    check_stack(1);
    new (top) Variant(v);
    ++top;
}

void Environment::push_global()
{
    push(G);
}

void Environment::current_function()
{
    check_stack(1);
    *top = *(bot - 1);
    ++top;
}


/* Read values from stack */

Variant *Environment::stack_index(int idx)
{
    static Variant undefined;
    auto pos = idx < 0 ? top + idx : bot + idx;

    if (pos < stack || pos >= top)
        return &undefined;

    return pos;
}

const Variant *Environment::stack_index(int idx) const
{
    return const_cast<Environment*>(this)->stack_index(idx);
}

Variant *get_variant(Environment *J, int idx)
{
    return J->stack_index(idx);
}

bool Environment::is_defined(int idx) const
{
    return !is_null(idx);
}

bool Environment::is_null(int idx) const
{
    return stack_index(idx)->is_null();
}

bool Environment::is_boolean(int idx) const
{
    return stack_index(idx)->is_boolean();
}

bool Environment::is_number(int idx) const
{
    return stack_index(idx)->is_number();
}

bool Environment::is_string(int idx) const
{
    return stack_index(idx)->is_string();
}

bool Environment::is_primitive(int idx) const
{
    return !is_object(idx);
}

bool Environment::is_object(int idx) const
{
    return stack_index(idx)->is_object();
}

bool Environment::is_list(int idx) const
{
    return stack_index(idx)->is_list();
}

bool Environment::is_regex(int idx) const
{
    return stack_index(idx)->is_regex();
}

bool Environment::is_file(int idx) const
{
    return stack_index(idx)->is_file();
}


bool Environment::is_user_data(int idx, const char *tag) const
{
    return stack_index(idx)->is_user_data(tag);
}

bool Environment::is_error(int idx) const
{
    return stack_index(idx)->is_error();
}


bool Environment::is_coercible(int idx) const
{
    auto v = stack_index(idx);
    return !v->is_null();
}

bool Environment::is_callable(int idx) const
{
    return stack_index(idx)->is_callable();
}


static const char *js_typeof(Environment *J, int idx)
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
        if (v->is_user_data())
            return v->as.object->as.user.tag;
        return "Object";
    }
}

bool Environment::to_boolean(int idx) const
{
    return stack_index(idx)->to_boolean();
}

double Environment::to_number(int idx)
{
    return to_number(stack_index(idx));
}

intptr_t Environment::to_integer(int idx)
{
    return number_to_integer(to_number(stack_index(idx)));
}

int Environment::to_int32(int idx)
{
    return number_to_int32(to_number(stack_index(idx)));
}

unsigned int Environment::to_uint32(int idx)
{
    return number_to_uint32(to_number(stack_index(idx)));
}

short Environment::to_int16(int idx)
{
    return number_to_int16(to_number(stack_index(idx)));
}

unsigned short Environment::to_uint16(int idx)
{
    return number_to_uint16(to_number(stack_index(idx)));
}

String Environment::to_string(int idx)
{
    return var_to_string(this, stack_index(idx));
}

std::any & Environment::to_user_data(int idx, const char *tag)
{
    Variant *v = stack_index(idx);

    if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CUSERDATA)
    {
        if (!strcmp(tag, v->as.object->as.user.tag))
            return v->as.object->as.user.data;
    }

    throw raise("Type error", "not a %s", tag);
}

std::any & Environment::to_user_data(int idx)
{
    Variant *v = stack_index(idx);

    if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CUSERDATA)
    {
        return v->as.object->as.user.data;
    }

    throw raise("Type error", "not a user data");
}

Object *Environment::to_object(int idx)
{
    return to_object(stack_index(idx));
}

void var_to_primitive(Environment *J, int idx, int hint)
{
    var_to_primitive(J, J->stack_index(idx), hint);
}

Regex & to_regexp(Environment *J, int idx)
{
    Variant *v = J->stack_index(idx);
    if (v->type == PHON_TOBJECT && v->as.object->type == PHON_CREGEX)
        return v->as.object->as.regex;
    throw J->raise("Type error", "not a regex");
}

static Object *jsR_tofunction(Environment *J, int idx)
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

int Environment::top_count() const
{
    return int(top - bot);
}

void Environment::copy(int idx)
{
    check_stack(1);
    *top = *stack_index(idx);
    ++top;
}

void Environment::remove(int idx)
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

void js_dup(Environment *J)
{
    CHECKSTACK(1);
    *TOP = *(TOP - 1);
    ++TOP;
}

void js_dup2(Environment *J)
{
    CHECKSTACK(2);
    TOP = TOP - 2;
    *(TOP + 1) = *(TOP - 1);
    TOP += 2;
}

void js_rot2(Environment *J)
{
    /* A B -> B A */
    std::swap(*(TOP - 1), *(TOP - 2));
}

void js_rot3(Environment *J)
{
    /* A B C -> C A B */
    Variant tmp = *(TOP - 1);    /* A B C (C) */
    *(TOP - 1) = *(TOP - 2);    /* A B B */
    *(TOP - 2) = *(TOP - 3);    /* A A B */
    *(TOP - 3) = std::move(tmp);        /* C A B */
}

void js_rot4(Environment *J)
{
    /* A B C D -> D A B C */
    Variant tmp = *(TOP - 1);    /* A B C D (D) */
    *(TOP - 1) = *(TOP - 2);    /* A B C C */
    *(TOP - 2) = *(TOP - 3);    /* A B B C */
    *(TOP - 3) = *(TOP - 4);    /* A A B C */
    *(TOP - 4) = std::move(tmp);        /* D A B C */
}

void js_rot2pop1(Environment *J)
{
    /* A B -> B */
    *(TOP - 2) = std::move(*(TOP - 1));
    --TOP;
}

void js_rot3pop2(Environment *J)
{
    /* A B C -> C */
    *(TOP - 3) = std::move(*(TOP - 1));
    J->pop(2);
}

void Environment::rot(int n)
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

int is_array_index(Environment *J, const String &s, int *idx)
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

bool Environment::has_field(Object *obj, const String &name)
{
    Field *ref;
    int k;

    if (name == "length")
    {
        if (obj->type == PHON_CLIST)
        {
            push(obj->as.list.size());
            return true;
        }
        if (obj->type == PHON_CSTRING)
        {
            push(obj->as.string.size());
            return true;
        }
        if (obj->type == PHON_COBJECT)
        {
            push(obj->fields.size());
            return true;
        }
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
        if (name == "pattern")
        {
            push(obj->as.regex.pattern());
            return true;
        }
        if (name == "ignore_case")
        {
            push_boolean(obj->as.regex.flags() & Regex::ICase);
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


void Environment::get_field(Object *obj, const String &name)
{
    if (!has_field(obj, name))
    {
        throw raise("Index error", "field \'%s\' does not exist", name.data());
    }
}

void Environment::get_field(Object *obj, intptr_t pos)
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

void Environment::set_field(Object *obj, intptr_t idx)
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

void Environment::set_field(Object *obj, const String &name)
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

void Environment::def_field(Object *obj, const String &name, int atts, Variant *value, Object *getter, Object *setter)
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

int Environment::del_field(Object *obj, const String &name)
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

String Environment::ref()
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

void Environment::unref(const String &ref)
{
    del_registry(ref);
}

void Environment::get_registry(const String &name)
{
    get_field(R, name);
}

void Environment::set_registry(const String &name)
{
    set_field(R, name);
    pop(1);
}

void Environment::del_registry(const String &name)
{
    del_field(R, name);
}

void Environment::get_global(const String &name)
{
    get_field(G, name);
}

void Environment::set_global(const String &name)
{
    set_field(G, name);
    pop(1);
}

void Environment::def_global(const String &name, int atts)
{
    def_field(G, name, atts, stack_index(-1), nullptr, nullptr);
    pop(1);
}

void Environment::get_field(int idx, const String &name)
{
    get_field(to_object(idx), name);
}

void Environment::set_field(int idx, const String &name)
{
    set_field(to_object(idx), name);
    pop(1);
}

void Environment::def_field(int idx, const String &name, int atts)
{
    def_field(to_object(idx), name, atts, stack_index(-1), nullptr, nullptr);
    pop(1);
}

void Environment::del_field(int idx, const String &name)
{
    del_field(to_object(idx), name);
}

void Environment::def_accessor(int idx, const String &name, int atts)
{
    def_field(to_object(idx), name, atts, nullptr, jsR_tofunction(this, -2), jsR_tofunction(this, -1));
    pop(2);
}

bool Environment::has_field(int idx, const String &name)
{
    return has_field(to_object(idx), name);
}

/* Iterator */

void Environment::push_iterator(int idx)
{
    auto obj = to_object(idx);
    push(obj->new_iterator(*this));
}

std::optional<Variant> Environment::next_iterator(int idx)
{
    return to_object(idx)->next_iterator(*this);
}

/* Namespace records */

Namespace *new_namespace(Environment *J, Object *variables, Namespace *outer)
{
    auto E = new Namespace;
    E->gcmark = 0;
    E->gcnext = J->gcenv;
    J->gcenv = E;
    ++J->gccounter;

    E->outer = outer;
    E->variables = variables;
    return E;
}

static void init_variable(Environment *J, const String &name, int idx)
{
    J->def_field(J->E->variables, name, PHON_DONTENUM | PHON_DONTCONF, J->stack_index(idx), nullptr, nullptr);
}

static void define_variable(Environment *J, const String &name)
{
    J->def_field(J->E->variables, name, PHON_DONTENUM | PHON_DONTCONF, nullptr, nullptr, nullptr);
}

static int has_variable(Environment *J, const String &name)
{
    Namespace *E = J->E;
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

static void set_variable(Environment *J, const String &name)
{
    Namespace *E = J->E;
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

static int delete_variable(Environment *J, const String &name)
{
    Namespace *E = J->E;
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

static void save_scope(Environment *J, Namespace *newE)
{
    if (J->nstop + 1 >= PHON_ENVLIMIT)
        throw std::runtime_error("stack overflow");
    J->nsstack[J->nstop++] = J->E;
    J->E = newE;
}

static void restore_scope(Environment *J)
{
    J->E = J->nsstack[--J->nstop];
}

static void call_wfunction(Environment *J, int n, Function *F, Namespace *scope)
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

static void call_function(Environment *J, int n, Function *F, Namespace *scope)
{
    Variant v;
    int i;

    scope = new_namespace(J, new Object(*J, PHON_COBJECT, nullptr), scope);

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

static void call_script(Environment *J, int n, Function *F, Namespace *scope)
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

static void call_native_function(Environment *J, int n, int min, native_callback_t &F)
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

static void push_trace(Environment *J, const String &name, const String &file, int line)
{
    if (J->tracetop + 1 == PHON_ENVLIMIT)
        throw J->raise("Runtime error", "call stack overflow");
    ++J->tracetop;
    J->trace[J->tracetop].name = name;
    J->trace[J->tracetop].file = file;
    J->trace[J->tracetop].line = line;
}

/* Main interpreter loop */

void Environment::dump_stack()
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

static void jsR_dumpenvironment(Environment *J, Namespace *E, int d)
{
    printf("scope %d ", d);
    dump_object(J, E->variables);
    if (E->outer)
        jsR_dumpenvironment(J, E->outer, d + 1);
}

void js_stacktrace(Environment *J)
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

void trap(Environment *J, int pc)
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

static void jsR_run(Environment *J, Function *F)
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

void Environment::call(int n)
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

void Environment::construct(int n)
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

void Environment::check_stack(int n)
{
    if (this->top + n >= this->stack + PHON_STACKSIZE)
        throw std::runtime_error("stack overflow");
}

void Environment::clear_stack()
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