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

#ifndef PHONOMETRICA_RUN_HPP
#define PHONOMETRICA_RUN_HPP

namespace phonometrica {

Namespace *new_namespace(Environment *J, Object *variables, Namespace *outer);

// A namespace stores local variables for a given lexical scope. Namespaces can be nested:
// each new function defines a new namespace, although this is optimitized out if the
// locals are not referenced by a nested scope. The top-level namespace is the global namespace.
struct Namespace
{
	Namespace *outer;
	Object *variables;

	Namespace *gcnext;
	int gcmark;
};

} // namespace phonometrica

#endif // PHONOMETRICA_RUN_HPP
