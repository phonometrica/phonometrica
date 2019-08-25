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

#define QQ(X) #X
#define Q(X) QQ(X)

namespace phonometrica {

static void Ep_toString(Runtime &rt)
{
    String message, name("Error");

    if (!rt.is_object(-1))
        throw rt.raise("Type error", "not an object");

    if (rt.has_field(0, "name"))
        name = rt.to_string(-1);
    if (rt.has_field(0, "message"))
        message = rt.to_string(-1);

    if (name.empty())
        rt.push(message);
    else if (message.empty())
        rt.push(name);
    else
    {
        name.append(": ");
        name.append(message);
        rt.push(name);
    }
}

static int jsB_ErrorX(Runtime *J, Object *prototype)
{
    int top = J->top_count();
    J->push(new Object(*J, PHON_CERROR, prototype));
    if (top > 1)
    {
        J->push(J->to_string(1));
        J->set_field(-2, "message");
    }
    if (J->get_stack_trace(1))
        J->set_field(-2, "stackTrace");
    return 1;
}

static void js_newerrorx(Runtime *J, const char *message, Object *prototype)
{
    J->push(new Object(*J, PHON_CERROR, prototype));
    J->push(message);
    J->set_field(-2, "message");
    if (J->get_stack_trace(0))
        J->set_field(-2, "stackTrace");
}


} // namespace phonometrica