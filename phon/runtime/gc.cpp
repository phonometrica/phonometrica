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
#include <phon/runtime/compile.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

static void mark_object(Runtime *J, int mark, Object *obj);


static void mark_function(Runtime *J, int mark, Function *fun)
{
    fun->gcmark = mark;
    for (auto f : fun->funtab)
        if (f->gcmark != mark)
            mark_function(J, mark, f);
}

static void mark_environment(Runtime *J, int mark, Environment *env)
{
    do
    {
	    env->gcmark = mark;
        if (env->variables->gcmark != mark)
            mark_object(J, mark, env->variables);
	    env = env->outer;
    } while (env && env->gcmark != mark);
}

static void mark_object(Runtime *J, int mark, Object *obj)
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
	        mark_environment(J, mark, obj->as.f.scope);
        if (obj->as.f.function && obj->as.f.function->gcmark != mark)
            mark_function(J, mark, obj->as.f.function);
    }
}

static void mark_stack(Runtime *J, int mark)
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

Runtime::~Runtime()
{
    Function *fun, *nextfun;
    Object *obj, *nextobj;
    Environment *env, *nextenv;

    // Run final collection.
    collect();

    for (env = this->gcenv; env; env = nextenv)
	    nextenv = env->gcnext, delete env;
    for (fun = this->gcfun; fun; fun = nextfun)
        nextfun = fun->gcnext, delete fun;
    for (obj = this->gcobj; obj; obj = nextobj)
        nextobj = obj->gcnext, delete obj;

    utils::free(this->stack);
}

void Runtime::collect(bool report)
{
    Function *fun, *nextfun, **prevnextfun;
    Object *obj, *nextobj, **prevnextobj;
    Environment *env, *nextenv, **prevnextenv;
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

	mark_environment(this, mark, this->E);
	mark_environment(this, mark, this->GE);
    for (i = 0; i < this->nstop; ++i)
	    mark_environment(this, mark, this->nsstack[i]);

	prevnextenv = &this->gcenv;
    for (env = this->gcenv; env; env = nextenv)
    {
	    nextenv = env->gcnext;
        if (env->gcmark != mark)
        {
            *prevnextenv = nextenv;
            delete env;
            ++genv;
        }
        else
        {
	        prevnextenv = &env->gcnext;
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

