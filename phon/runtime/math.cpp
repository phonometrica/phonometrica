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

static void math_abs(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return abs(n); }));
	else
	    rt.push(fabs(rt.to_number(1)));
}

static void math_acos(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return acos(n); }));
	else
	    rt.push(acos(rt.to_number(1)));
}

static void math_asin(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return asin(n); }));
	else
	    rt.push(asin(rt.to_number(1)));
}

static void math_atan(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return atan(n); }));
	else
	    rt.push(atan(rt.to_number(1)));
}

static void math_atan2(Runtime &rt)
{
    double y = rt.to_number(1);
    double x = rt.to_number(2);
    rt.push(atan2(y, x));
}

static void math_ceil(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return ceil(n); }));
	else
	    rt.push(ceil(rt.to_number(1)));
}

static void math_cos(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return cos(n); }));
	else
	    rt.push(cos(rt.to_number(1)));
}

static void math_exp(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return exp(n); }));
	else
	    rt.push(exp(rt.to_number(1)));
}

static void math_floor(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return floor(n); }));
	else
	    rt.push(floor(rt.to_number(1)));
}

static void math_log(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return log(n); }));
	else
	    rt.push(log(rt.to_number(1)));
}

static void math_log10(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return log10(n); }));
	else
		rt.push(log10(rt.to_number(1)));
}

static void math_log2(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return log2(n); }));
	else
		rt.push(log2(rt.to_number(1)));
}

static void math_pow(Runtime &rt)
{
    double x = rt.to_number(1);
    double y = rt.to_number(2);
    if (!std::isfinite(y) && fabs(x) == 1)
        rt.push(NAN);
    else
        rt.push(pow(x, y));
}

static void math_random(Runtime &rt)
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

static double do_round(long double x, int n)
{
    auto p = pow(10, n);
    if (std::isnan(x)) return x;
    if (std::isinf(x)) return x;
    if (x == 0) return x;

    return round(x * p) / p;
}

static void math_round(Runtime &rt)
{
	if (rt.is_array(1))
	{
		auto &X = rt.to_array(1);	
		if (rt.arg_count() > 1)
		{
			auto n = (int) rt.to_integer(2);
			auto f = [=](double x) { return do_round(x, n); };
			rt.push(apply(X, f));
		}
		else
		{
			auto f = [=](double x) { return do_round(x); };		
			rt.push(apply(X, f));
		}
	}
	else
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
}

static void math_sin(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return sin(n); }));
	else
	    rt.push(sin(rt.to_number(1)));
}

static void math_sqrt(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return sqrt(n); }));
	else
	    rt.push(sqrt(rt.to_number(1)));
}

static void math_tan(Runtime &rt)
{
	if (rt.is_array(1))
		rt.push(apply(rt.to_array(1), [](double n) { return tan(n); }));
	else
	    rt.push(tan(rt.to_number(1)));
}

static void math_max(Runtime &rt)
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

static void math_min(Runtime &rt)
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

	add_global_function("abs", math_abs, 1);
	add_global_function("acos", math_acos, 1);
	add_global_function("asin", math_asin, 1);
	add_global_function("atan", math_atan, 1);
	add_global_function("atan2", math_atan2, 2);
	add_global_function("ceil", math_ceil, 1);
	add_global_function("cos", math_cos, 1);
	add_global_function("exp", math_exp, 1);
	add_global_function("floor", math_floor, 1);
	add_global_function("log", math_log, 1);
	add_global_function("log10", math_log10, 1);
	add_global_function("log2", math_log2, 1);
	add_global_function("max", math_max, 0); /* 2 */
	add_global_function("min", math_min, 0); /* 2 */
	add_global_function("pow", math_pow, 2);
	add_global_function("random", math_random, 0);
	add_global_function("round", math_round, 1);
	add_global_function("sin", math_sin, 1);
	add_global_function("sqrt", math_sqrt, 1);
	add_global_function("tan", math_tan, 1);

    push(new Object(*this, PHON_CMATH, object_meta));
    {
        add_math_constant("E", 2.7182818284590452354);
        add_math_constant("PI", 3.1415926535897932);
        add_math_constant("SQRT2", 1.4142135623730951);
        add_math_constant("PHI", 1.6180339887498948);
    }
    def_global("__math", PHON_DONTENUM);

	do_string(R"__(this
		E = __math.E
		PI = __math.PI
		SQRT2 = __math.SQRT2
        PHI = __math.PHI
    )__");
}

} // namespace phonometrica