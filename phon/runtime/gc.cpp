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
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/run.hpp>
#include <phon/runtime/environment.hpp>
#include "object.hpp"


namespace phonometrica {

static void mark_object(Environment *J, int mark, Object *obj);


static void mark_function(Environment *J, int mark, Function *fun)
{
    fun->gcmark = mark;
    for (auto f : fun->funtab)
        if (f->gcmark != mark)
            mark_function(J, mark, f);
}

static void mark_namespace(Environment *J, int mark, Namespace *ns)
{
    do
    {
        ns->gcmark = mark;
        if (ns->variables->gcmark != mark)
            mark_object(J, mark, ns->variables);
        ns = ns->outer;
    } while (ns && ns->gcmark != mark);
}

static void mark_object(Environment *J, int mark, Object *obj)
{
    obj->gcmark = mark;

    for (auto &f : obj->fields)
    {
        Field *node = &f.second;
        if (node->value.type == PHON_TOBJECT && node->value.as.object->gcmark != mark)
            mark_object(J, mark, node->value.as.object);
        if (node->getter && node->getter->gcmark != mark)
            mark_object(J, mark, node->getter);
        if (node->setter && node->setter->gcmark != mark)
            mark_object(J, mark, node->setter);
    }
    if (obj->prototype && obj->prototype->gcmark != mark)
    {
        mark_object(J, mark, obj->prototype);
    }
    if (obj->type == PHON_CITERATOR)
    {
        mark_object(J, mark, obj->as.iter.target);
    }
    if (obj->type == PHON_CFUNCTION || obj->type == PHON_CSCRIPT)
    {
        if (obj->as.f.scope && obj->as.f.scope->gcmark != mark)
            mark_namespace(J, mark, obj->as.f.scope);
        if (obj->as.f.function && obj->as.f.function->gcmark != mark)
            mark_function(J, mark, obj->as.f.function);
    }
}

static void mark_stack(Environment *J, int mark)
{
    Variant *v = J->stack;
    auto n = J->top;
    while (n-- > J->stack)
    {
        if (v->type == PHON_TOBJECT && v->as.object->gcmark != mark)
            mark_object(J, mark, v->as.object);
        ++v;
    }
}

Environment::~Environment()
{
    Function *fun, *nextfun;
    Object *obj, *nextobj;
    Namespace *ns, *nextns;

    // Run final collection.
    collect();

    for (ns = this->gcenv; ns; ns = nextns)
        nextns = ns->gcnext, delete ns;
    for (fun = this->gcfun; fun; fun = nextfun)
        nextfun = fun->gcnext, delete fun;
    for (obj = this->gcobj; obj; obj = nextobj)
        nextobj = obj->gcnext, delete obj;

    utils::free(this->stack);
}

void Environment::collect(bool report)
{
    Function *fun, *nextfun, **prevnextfun;
    Object *obj, *nextobj, **prevnextobj;
    Namespace *ns, *nextns, **prevnextns;
    int nenv = 0, nfun = 0, nobj = 0, nstr = 0;
    int genv = 0, gfun = 0, gobj = 0, gstr = 0;
    int mark;
    int i;

    if (this->gcpause)
    {
        if (report)
            this->report("garbage collector is paused");
        return;
    }

    this->gccounter = 0;

    mark = this->gcmark = this->gcmark == 1 ? 2 : 1;

    mark_object(this, mark, this->object_meta);
    mark_object(this, mark, this->list_meta);
    mark_object(this, mark, this->function_meta);
    mark_object(this, mark, this->boolean_meta);
    mark_object(this, mark, this->number_meta);
    mark_object(this, mark, this->string_meta);
    mark_object(this, mark, this->regex_meta);
    mark_object(this, mark, this->date_meta);

    mark_object(this, mark, this->R);
    mark_object(this, mark, this->G);

    for (auto o : this->permanent_objects) {
        mark_object(this, mark, o);
    }

    mark_stack(this, mark);

    mark_namespace(this, mark, this->E);
    mark_namespace(this, mark, this->GE);
    for (i = 0; i < this->nstop; ++i)
        mark_namespace(this, mark, this->nsstack[i]);

    prevnextns = &this->gcenv;
    for (ns = this->gcenv; ns; ns = nextns)
    {
        nextns = ns->gcnext;
        if (ns->gcmark != mark)
        {
            *prevnextns = nextns;
            delete ns;
            ++genv;
        }
        else
        {
            prevnextns = &ns->gcnext;
        }
        ++nenv;
    }

    prevnextfun = &this->gcfun;
    for (fun = this->gcfun; fun; fun = nextfun)
    {
        nextfun = fun->gcnext;
        if (fun->gcmark != mark)
        {
            *prevnextfun = nextfun;
            delete fun;
            ++gfun;
        }
        else
        {
            prevnextfun = &fun->gcnext;
        }
        ++nfun;
    }

    prevnextobj = &this->gcobj;
    for (obj = this->gcobj; obj; obj = nextobj)
    {
        nextobj = obj->gcnext;
        if (obj->gcmark != mark)
        {
            *prevnextobj = nextobj;
            delete obj;
            ++gobj;
        }
        else
        {
            prevnextobj = &obj->gcnext;
        }
        ++nobj;
    }

    if (report)
    {
        char buf[256];
        snprintf(buf, sizeof buf, "garbage collected: %d/%d envs, %d/%d funs, %d/%d objs, %d/%d strs",
                 genv, nenv, gfun, nfun, gobj, nobj, gstr, nstr);
        this->report(buf);
    }

}



} // namespace phonometrica

