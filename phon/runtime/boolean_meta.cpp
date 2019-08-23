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

static void jsB_boolean(Environment &env)
{
	env.push_boolean(env.to_boolean(1));
}

static void boolean_ctor(Environment &env)
{
	env.new_boolean(env.to_boolean(1));
}

static void boolean_to_string(Environment &env)
{
	Object *self = env.to_object(0);
	if (self->type != PHON_CBOOLEAN) throw env.raise("Type error", "not a boolean");
    env.push(self->as.boolean ? env.true_string : env.false_string);
}

static void boolean_to_value(Environment &env)
{
	Object *self = env.to_object(0);
	if (self->type != PHON_CBOOLEAN) throw env.raise("Type error", "not a boolean");
	env.push_boolean(self->as.boolean);
}

void Environment::init_boolean()
{
	boolean_meta->as.boolean = false;

    push(boolean_meta);
	{
		add_method("Boolean.meta.to_string", boolean_to_string, 0);
		add_method("Boolean.meta.to_value", boolean_to_value, 0);
	}
    new_native_constructor(jsB_boolean, boolean_ctor, "Boolean", 1);
	def_global("Boolean", PHON_DONTENUM);
}

} // namespace phonometrica
