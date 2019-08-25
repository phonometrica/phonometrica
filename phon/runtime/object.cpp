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

#include <phon/runtime/object.hpp>
#include <phon/runtime/runtime.hpp>
#include "object.hpp"


namespace phonometrica {


static Field *jsV_getenumfield(Runtime *J, Object *obj, const String &name)
{
    do
    {
        Field *ref = obj->get_own_field(*J, name);
        if (ref && !(ref->atts & PHON_DONTENUM))
            return ref;
        obj = obj->prototype;
    } while (obj);
    return nullptr;
}

Object::Object(Runtime &rt, ClassTag type, Object *prototype)
{
    memset(&this->as, 0, sizeof(Storage));
    this->gcmark = 0;
    // Attach to GC chain.
    this->gcnext = rt.gcobj;
    //if (rt.gcobj) rt.gcobj->gcprev = this;
    rt.gcobj = this;
    ++rt.gccounter;
    //this->rt = &rt;

    this->type = type;
    this->prototype = prototype;
    this->extensible = true;
}

Object::~Object()
{
    if (this->type == PHON_CLIST)
        this->as.list.~Array<Variant>();
    else if (this->type == PHON_CREGEX)
        this->as.regex.~Regex();
    else if (this->type == PHON_CFILE)
        this->as.file.~File();
    else if (this->type == PHON_CITERATOR)
        this->as.iter.data.~any();
    else if (this->type == PHON_CUSERDATA)
        this->as.user.data.~any();
    else if (this->type == PHON_CCFUNCTION)
    {
        this->as.c.constructor.~native_callback_t();
        this->as.c.function.~native_callback_t();
        this->as.c.name.~String();
    }

    // Detach object from the GC chain.
//    if (this == rt->gcobj) {
//        rt->gcobj = this->gcnext;
//    }
//
//    if (this->gcprev) {
//        this->gcprev->gcnext = this->gcnext;
//    }
//    if (this->gcnext) {
//        this->gcnext->gcprev = this->gcprev;
//    }
}

Field *Object::get_own_field(Runtime &, const String &name)
{
    auto it = fields.find(name);
    return (it == fields.end()) ? nullptr : &it->second;
}

Field *Object::get_field(Runtime &, const String &name, bool *own)
{
    *own = true;
    auto obj = this;

    do
    {
        auto it = obj->fields.find(name);
        if (it != obj->fields.end())
            return &it->second;
        obj = obj->prototype;
        *own = false;
    }
    while (obj);

    return nullptr;
}

Field *Object::get_field(Runtime &, const String &name)
{
    auto obj = this;

    do
    {
        auto it = obj->fields.find(name);
        if (it != obj->fields.end())
            return &it->second;
        obj = obj->prototype;
    }
    while (obj);

    return nullptr;
}

Field *Object::set_field(Runtime &rt, const String &name)
{
    if (!this->extensible)
    {
        auto result = this->get_own_field(rt, name);
        if (rt.strict && !result)
            throw rt.raise("Type error", "object is non-extensible");
        return result;
    }

    auto r = this->fields.emplace(name, Field{});
    if (r.second) this->version++;

    return &r.first->second;
}

void Object::del_field(Runtime &rt, const String &name)
{
    fields.erase(name);
    version++;
}

Object *Object::new_iterator(Runtime &rt)
{
    if (type == PHON_CSTRING)
    {
        throw rt.raise("Error", "%s", "cannot iterate string");
    }

    auto io = new Object(rt, PHON_CITERATOR, nullptr);
    io->as.iter.target = this;

    if (type == PHON_CLIST || type == PHON_CSTRING)
    {
        new (&io->as.iter.data) std::any();
        io->as.iter.index = 1;

        return io;
    }
    io->as.iter.index = 0;
    io->version = this->version;
    new (&io->as.iter.data) std::any(this->fields.begin());

    return io;
}

std::optional<Variant> Object::next_iterator(Runtime &rt)
{
    assert(as.iter.target->type != PHON_CSTRING);

    if (this->type != PHON_CITERATOR)
        throw rt.raise("Type error", "not an iterator");

    if (as.iter.target->type == PHON_CLIST)
    {
        auto &lst = as.iter.target->as.list;
        auto i = as.iter.index;

        if (i > 0 && i <= lst.size())
        {
            // Move to the next position.
            as.iter.index = i + 1;

            return lst[i];
        }
    }
    else
    {
        if (this->version != as.iter.target->version)
        {
            throw rt.raise("Error", "%s", "object iterator has been invalidated by a mutating operation");
        }
        auto &it = std::any_cast<FieldMap::iterator&>(as.iter.data);

        if (it != fields.end())
        {
            auto &key = it->first;
            it++;

            return key;
        }
    }

    return std::optional<Variant>(); // done
}

void Object::resize_list(Runtime &rt, int new_size)
{
    as.list.resize(new_size);
}


} // namespace phonometrica