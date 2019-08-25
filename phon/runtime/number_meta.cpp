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

#if defined(_MSC_VER) && (_MSC_VER < 1700) /* VS2012 has stdint.h */
typedef unsigned __int64 uint64_t;
#else

#include <stdint.h>

#endif

namespace phonometrica {


static void jsB_new_Number(Runtime &rt)
{
    rt.new_number(rt.top_count() > 1 ? rt.to_number(1) : 0);
}

static void jsB_Number(Runtime &rt)
{
    rt.push(rt.top_count() > 1 ? rt.to_number(1) : 0);
}

static void Np_to_value(Runtime &rt)
{
    Object *self = rt.to_object(0);
    if (self->type != PHON_CNUMBER) throw rt.raise("Type error", "not a number");
    rt.push(self->as.number);
}

static void Np_toString(Runtime &rt)
{
    char buf[32];
    Object *self = rt.to_object(0);
    int radix = rt.is_null(1) ? 10 : rt.to_integer(1);
    if (self->type != PHON_CNUMBER)
        throw rt.raise("Type error", "not a number");
    if (radix == 10)
    {
        rt.push(number_to_string(&rt, buf, self->as.number));
        return;
    }
    if (radix < 2 || radix > 36)
        throw rt.raise("Range error","invalid radix");

    /* lame number to string conversion for any radix from 2 to 36 */
    {
        static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        char buf[100];
        double number = self->as.number;
        int sign = self->as.number < 0;
        String buffer;
        uint64_t u, limit = ((uint64_t) 1 << 52);

        int ndigits, exp, point;

        if (number == 0)
        {
            rt.push("0");
            return;
        }
        if (std::isnan(number))
        {
            rt.push(rt.undef_string);
            return;
        }
        if (std::isinf(number))
        {
            rt.push(sign ? "-Infinity" : "Infinity");
            return;
        }

        if (sign)
            number = -number;

        /* fit as many digits as we want in an int */
        exp = 0;
        while (number * pow(radix, exp) > limit)
            --exp;
        while (number * pow(radix, exp + 1) < limit)
            ++exp;
        u = number * pow(radix, exp) + 0.5;

        /* trim trailing zeros */
        while (u > 0 && (u % radix) == 0)
        {
            u /= radix;
            --exp;
        }

        /* serialize digits */
        ndigits = 0;
        while (u > 0)
        {
            buf[ndigits++] = digits[u % radix];
            u /= radix;
        }
        point = ndigits - exp;

        if (sign)
            buffer.append('-');

        if (point <= 0)
        {
            buffer.append('0');
            buffer.append('.');
            while (point++ < 0)
                buffer.append('0');
            while (ndigits-- > 0)
                buffer.append(buf[ndigits]);
        }
        else
        {
            while (ndigits-- > 0)
            {
                buffer.append(buf[ndigits]);
                if (--point == 0 && ndigits > 0)
                    buffer.append('.');
            }
            while (point-- > 0)
                buffer.append('0');
        }

        rt.push(std::move(buffer));
    }
}

/* Customized ToString() on a number */
static void numtostr(Runtime *J, const char *fmt, int w, double n)
{
    char buf[32], *e;
    sprintf(buf, fmt, w, n);
    e = strchr(buf, 'e');
    if (e)
    {
        int exp = atoi(e + 1);
        sprintf(e, "e%+d", exp);
    }
    J->push(buf);
}

static void Np_toFixed(Runtime &rt)
{
    Object *self = rt.to_object(0);
    int width = rt.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw rt.raise("Type error", "not a number");
    if (width < 0) throw rt.raise("Range error","precision %d out of range", width);
    if (width > 20) throw rt.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (std::isnan(x) || std::isinf(x) || x <= -1e21 || x >= 1e21)
        rt.push(number_to_string(&rt, buf, x));
    else
        numtostr(&rt, "%.*f", width, x);
}

static void Np_toExponential(Runtime &rt)
{
    Object *self = rt.to_object(0);
    int width = rt.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw rt.raise("Type error", "not a number");
    if (width < 0) throw rt.raise("Range error","precision %d out of range", width);
    if (width > 20) throw rt.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (std::isnan(x) || std::isinf(x))
        rt.push(number_to_string(&rt, buf, x));
    else
        numtostr(&rt, "%.*e", width, self->as.number);
}

static void Np_toPrecision(Runtime &rt)
{
    Object *self = rt.to_object(0);
    int width = rt.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw rt.raise("Type error", "not a number");
    if (width < 1) throw rt.raise("Range error","precision %d out of range", width);
    if (width > 21) throw rt.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (std::isnan(x) || std::isinf(x))
        rt.push(number_to_string(&rt, buf, x));
    else
        numtostr(&rt, "%.*g", width, self->as.number);
}

void Runtime::init_number()
{
    number_meta->as.number = 0;

    push(number_meta);
    {
        add_method("Number.meta.to_value", Np_to_value, 0);
        add_method("Number.meta.to_string", Np_toString, 1);
        add_method("Number.meta.to_fixed", Np_toFixed, 1);
        add_method("Number.meta.to_exponential", Np_toExponential, 1);
        add_method("Number.meta.to_precision", Np_toPrecision, 1);
    }
    new_native_constructor(jsB_Number, jsB_new_Number, "Number", 0); /* 1 */
    {
        add_math_constant("MAX_VALUE", 1.7976931348623157e+308);
        add_math_constant("MIN_VALUE", 5e-324);
        add_math_constant("NaN", NAN);
        add_math_constant("NEGATIVE_INFINITY", -INFINITY);
        add_math_constant("POSITIVE_INFINITY", INFINITY);
    }
    def_global("Number", PHON_DONTENUM);
}

} // namespace phonometrica