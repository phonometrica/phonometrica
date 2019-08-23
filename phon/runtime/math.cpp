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

#include <time.h>

namespace phonometrica {

static void Math_abs(Environment &env)
{
    env.push(fabs(env.to_number(1)));
}

static void Math_acos(Environment &env)
{
    env.push(acos(env.to_number(1)));
}

static void Math_asin(Environment &env)
{
    env.push(asin(env.to_number(1)));
}

static void Math_atan(Environment &env)
{
    env.push(atan(env.to_number(1)));
}

static void Math_atan2(Environment &env)
{
    double y = env.to_number(1);
    double x = env.to_number(2);
    env.push(atan2(y, x));
}

static void Math_ceil(Environment &env)
{
    env.push(ceil(env.to_number(1)));
}

static void Math_cos(Environment &env)
{
    env.push(cos(env.to_number(1)));
}

static void Math_exp(Environment &env)
{
    env.push(exp(env.to_number(1)));
}

static void Math_floor(Environment &env)
{
    env.push(floor(env.to_number(1)));
}

static void Math_log(Environment &env)
{
    env.push(log(env.to_number(1)));
}

static void Math_pow(Environment &env)
{
    double x = env.to_number(1);
    double y = env.to_number(2);
    if (!isfinite(y) && fabs(x) == 1)
        env.push(NAN);
    else
        env.push(pow(x, y));
}

static void Math_random(Environment &env)
{
    env.push(rand() / (RAND_MAX + 1.0));
}

static double do_round(double x)
{
    if (isnan(x)) return x;
    if (isinf(x)) return x;
    if (x == 0) return x;
    if (x > 0 && x < 0.5) return 0;
    if (x < 0 && x >= -0.5) return -0;
    return floor(x + 0.5);
}

static double do_round(double x, int n)
{
    auto p = pow(10, n);
    if (isnan(x)) return x;
    if (isinf(x)) return x;
    if (x == 0) return x;

    return round(x * p) / p;
}

static void Math_round(Environment &env)
{
    double x = env.to_number(1);
    if (env.arg_count() > 1)
    {
        auto n = (int) env.to_integer(2);
        env.push(do_round(x, n));
    }
    else
    {
        env.push(do_round(x));
    }
}

static void Math_sin(Environment &env)
{
    env.push(sin(env.to_number(1)));
}

static void Math_sqrt(Environment &env)
{
    env.push(sqrt(env.to_number(1)));
}

static void Math_tan(Environment &env)
{
    env.push(tan(env.to_number(1)));
}

static void Math_max(Environment &env)
{
    int i, n = env.top_count();
    double x = -INFINITY;
    for (i = 1; i < n; ++i)
    {
        double y = env.to_number(i);
        if (isnan(y))
        {
            x = y;
            break;
        }
        if (signbit(x) == signbit(y))
            x = x > y ? x : y;
        else if (signbit(x))
            x = y;
    }
    env.push(x);
}

static void Math_min(Environment &env)
{
    int i, n = env.top_count();
    double x = INFINITY;
    for (i = 1; i < n; ++i)
    {
        double y = env.to_number(i);
        if (isnan(y))
        {
            x = y;
            break;
        }
        if (signbit(x) == signbit(y))
            x = x < y ? x : y;
        else if (signbit(y))
            x = y;
    }
    env.push(x);
}

void Environment::init_math()
{
    srand(time(nullptr));

    push(new Object(*this, PHON_CMATH, object_meta));
    {
        add_math_constant("E", 2.7182818284590452354);
        add_math_constant("LN10", 2.302585092994046);
        add_math_constant("LN2", 0.6931471805599453);
        add_math_constant("LOG2E", 1.4426950408889634);
        add_math_constant("LOG10E", 0.4342944819032518);
        add_math_constant("PI", 3.1415926535897932);
        add_math_constant("SQRT1_2", 0.7071067811865476);
        add_math_constant("SQRT2", 1.4142135623730951);

        add_method("math.abs", Math_abs, 1);
        add_method("math.acos", Math_acos, 1);
        add_method("math.asin", Math_asin, 1);
        add_method("math.atan", Math_atan, 1);
        add_method("math.atan2", Math_atan2, 2);
        add_method("math.ceil", Math_ceil, 1);
        add_method("math.cos", Math_cos, 1);
        add_method("math.exp", Math_exp, 1);
        add_method("math.floor", Math_floor, 1);
        add_method("math.log", Math_log, 1);
        add_method("math.max", Math_max, 0); /* 2 */
        add_method("math.min", Math_min, 0); /* 2 */
        add_method("math.pow", Math_pow, 2);
        add_method("math.random", Math_random, 0);
        add_method("math.round", Math_round, 1);
        add_method("math.sin", Math_sin, 1);
        add_method("math.sqrt", Math_sqrt, 1);
        add_method("math.tan", Math_tan, 1);
    }
    def_global("math", PHON_DONTENUM);
}

} // namespace phonometrica