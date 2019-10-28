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

#include <time.h>

namespace phonometrica {

static void Math_abs(Runtime &rt)
{
    rt.push(fabs(rt.to_number(1)));
}

static void Math_acos(Runtime &rt)
{
    rt.push(acos(rt.to_number(1)));
}

static void Math_asin(Runtime &rt)
{
    rt.push(asin(rt.to_number(1)));
}

static void Math_atan(Runtime &rt)
{
    rt.push(atan(rt.to_number(1)));
}

static void Math_atan2(Runtime &rt)
{
    double y = rt.to_number(1);
    double x = rt.to_number(2);
    rt.push(atan2(y, x));
}

static void Math_ceil(Runtime &rt)
{
    rt.push(ceil(rt.to_number(1)));
}

static void Math_cos(Runtime &rt)
{
    rt.push(cos(rt.to_number(1)));
}

static void Math_exp(Runtime &rt)
{
    rt.push(exp(rt.to_number(1)));
}

static void Math_floor(Runtime &rt)
{
    rt.push(floor(rt.to_number(1)));
}

static void Math_log(Runtime &rt)
{
    rt.push(log(rt.to_number(1)));
}

static void Math_log10(Runtime &rt)
{
	rt.push(log10(rt.to_number(1)));
}

static void Math_log2(Runtime &rt)
{
	rt.push(log2(rt.to_number(1)));
}

static void Math_pow(Runtime &rt)
{
    double x = rt.to_number(1);
    double y = rt.to_number(2);
    if (!std::isfinite(y) && fabs(x) == 1)
        rt.push(NAN);
    else
        rt.push(pow(x, y));
}

static void Math_random(Runtime &rt)
{
    rt.push(rand() / (RAND_MAX + 1.0));
}

static double do_round(double x)
{
    if (std::isnan(x)) return x;
    if (std::isinf(x)) return x;
    if (x == 0) return x;
    if (x > 0 && x < 0.5) return 0;
    if (x < 0 && x >= -0.5) return -0;
    return floor(x + 0.5);
}

static double do_round(double x, int n)
{
    auto p = pow(10, n);
    if (std::isnan(x)) return x;
    if (std::isinf(x)) return x;
    if (x == 0) return x;

    return round(x * p) / p;
}

static void Math_round(Runtime &rt)
{
    double x = rt.to_number(1);
    if (rt.arg_count() > 1)
    {
        auto n = (int) rt.to_integer(2);
        rt.push(do_round(x, n));
    }
    else
    {
        rt.push(do_round(x));
    }
}

static void Math_sin(Runtime &rt)
{
    rt.push(sin(rt.to_number(1)));
}

static void Math_sqrt(Runtime &rt)
{
    rt.push(sqrt(rt.to_number(1)));
}

static void Math_tan(Runtime &rt)
{
    rt.push(tan(rt.to_number(1)));
}

static void Math_max(Runtime &rt)
{
    int i, n = rt.top_count();
    double x = -INFINITY;
    for (i = 1; i < n; ++i)
    {
        double y = rt.to_number(i);
        if (std::isnan(y))
        {
            x = y;
            break;
        }
        if (std::signbit(x) == std::signbit(y))
            x = x > y ? x : y;
        else if (std::signbit(x))
            x = y;
    }
    rt.push(x);
}

static void Math_min(Runtime &rt)
{
    int i, n = rt.top_count();
    double x = INFINITY;
    for (i = 1; i < n; ++i)
    {
        double y = rt.to_number(i);
        if (std::isnan(y))
        {
            x = y;
            break;
        }
        if (std::signbit(x) == std::signbit(y))
            x = x < y ? x : y;
        else if (std::signbit(y))
            x = y;
    }
    rt.push(x);
}

void Runtime::init_math()
{
    srand(time(nullptr));

	add_global_function("abs", Math_abs, 1);
	add_global_function("acos", Math_acos, 1);
	add_global_function("asin", Math_asin, 1);
	add_global_function("atan", Math_atan, 1);
	add_global_function("atan2", Math_atan2, 2);
	add_global_function("ceil", Math_ceil, 1);
	add_global_function("cos", Math_cos, 1);
	add_global_function("exp", Math_exp, 1);
	add_global_function("floor", Math_floor, 1);
	add_global_function("log", Math_log, 1);
	add_global_function("log10", Math_log10, 1);
	add_global_function("log2", Math_log2, 1);
	add_global_function("max", Math_max, 0); /* 2 */
	add_global_function("min", Math_min, 0); /* 2 */
	add_global_function("pow", Math_pow, 2);
	add_global_function("random", Math_random, 0);
	add_global_function("round", Math_round, 1);
	add_global_function("sin", Math_sin, 1);
	add_global_function("sqrt", Math_sqrt, 1);
	add_global_function("tan", Math_tan, 1);

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
    }
    def_global("math", PHON_DONTENUM);
}

} // namespace phonometrica