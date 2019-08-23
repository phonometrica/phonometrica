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

#if defined(_MSC_VER) && (_MSC_VER < 1700) /* VS2012 has stdint.h */
typedef unsigned __int64 uint64_t;
#else

#include <stdint.h>

#endif

namespace phonometrica {


static void jsB_new_Number(Environment &env)
{
    env.new_number(env.top_count() > 1 ? env.to_number(1) : 0);
}

static void jsB_Number(Environment &env)
{
    env.push(env.top_count() > 1 ? env.to_number(1) : 0);
}

static void Np_to_value(Environment &env)
{
    Object *self = env.to_object(0);
    if (self->type != PHON_CNUMBER) throw env.raise("Type error", "not a number");
    env.push(self->as.number);
}

static void Np_toString(Environment &env)
{
    char buf[32];
    Object *self = env.to_object(0);
    int radix = env.is_null(1) ? 10 : env.to_integer(1);
    if (self->type != PHON_CNUMBER)
        throw env.raise("Type error", "not a number");
    if (radix == 10)
    {
        env.push(number_to_string(&env, buf, self->as.number));
        return;
    }
    if (radix < 2 || radix > 36)
        throw env.raise("Range error","invalid radix");

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
            env.push("0");
            return;
        }
        if (isnan(number))
        {
            env.push(env.undef_string);
            return;
        }
        if (isinf(number))
        {
            env.push(sign ? "-Infinity" : "Infinity");
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

        env.push(std::move(buffer));
    }
}

/* Customized ToString() on a number */
static void numtostr(Environment *J, const char *fmt, int w, double n)
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

static void Np_toFixed(Environment &env)
{
    Object *self = env.to_object(0);
    int width = env.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw env.raise("Type error", "not a number");
    if (width < 0) throw env.raise("Range error","precision %d out of range", width);
    if (width > 20) throw env.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (isnan(x) || isinf(x) || x <= -1e21 || x >= 1e21)
        env.push(number_to_string(&env, buf, x));
    else
        numtostr(&env, "%.*f", width, x);
}

static void Np_toExponential(Environment &env)
{
    Object *self = env.to_object(0);
    int width = env.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw env.raise("Type error", "not a number");
    if (width < 0) throw env.raise("Range error","precision %d out of range", width);
    if (width > 20) throw env.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (isnan(x) || isinf(x))
        env.push(number_to_string(&env, buf, x));
    else
        numtostr(&env, "%.*e", width, self->as.number);
}

static void Np_toPrecision(Environment &env)
{
    Object *self = env.to_object(0);
    int width = env.to_integer(1);
    char buf[32];
    double x;
    if (self->type != PHON_CNUMBER) throw env.raise("Type error", "not a number");
    if (width < 1) throw env.raise("Range error","precision %d out of range", width);
    if (width > 21) throw env.raise("Range error","precision %d out of range", width);
    x = self->as.number;
    if (isnan(x) || isinf(x))
        env.push(number_to_string(&env, buf, x));
    else
        numtostr(&env, "%.*g", width, self->as.number);
}

void Environment::init_number()
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