/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 28/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: statistical functions for the scripting engine.                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/toplevel.hpp>
#include <phon/analysis/statistics.hpp>

namespace phonometrica {

static stats::Alternative alternative(const String &s)
{
	if (s == "two-tailed")
		return stats::Alternative::TwoTailed;
	else if (s == "greater")
		return stats::Alternative::Greater;
	else if (s == "less")
		return stats::Alternative::Less;

	throw error("Invalid alternative in F-test: %", s);
}

static void stat_sum(Runtime &rt)
{
	auto &x = rt.to_array(1);
	if (rt.arg_count() > 1)
	{
		auto dim = rt.to_integer(2);
		if (x.ndim() == 1 && dim == 1)
			rt.push(stats::sum(x));
		else
			rt.push(stats::sum(x, dim));
	}
	else
	{
		rt.push(stats::sum(x));
	}
}

static void stat_mean(Runtime &rt)
{
	auto &x = rt.to_array(1);
	if (rt.arg_count() > 1)
	{
		auto dim = rt.to_integer(2);
		if (x.ndim() == 1 && dim == 1)
			rt.push(stats::mean(x));
		else
			rt.push(stats::mean(x, dim));
	}
	else
	{
		rt.push(stats::mean(x));
	}
}

static void stat_var(Runtime &rt)
{
	auto &x = rt.to_array(1);
	if (rt.arg_count() > 1)
	{
		auto dim = rt.to_integer(2);
		if (x.ndim() == 1 && dim == 1)
			rt.push(stats::sample_variance(x));
		else
			rt.push(stats::sample_variance(x, dim));
	}
	else
	{
		rt.push(stats::sample_variance(x));
	}
}

static void stat_std(Runtime &rt)
{
	auto &x = rt.to_array(1);
	if (rt.arg_count() > 1)
	{
		auto dim = rt.to_integer(2);
		if (x.ndim() == 1 && dim == 1)
			rt.push(stats::stdev(x));
		else
			rt.push(stats::stdev(x, dim));
	}
	else
	{
		rt.push(stats::stdev(x));
	}
}

static void stat_chi2(Runtime &rt)
{
	auto &x = rt.to_array(1);
	auto result = stats::chi2_test(x);
	rt.new_object();
	rt.add_numeric_field("chi2", std::get<0>(result));
	rt.add_numeric_field("df", std::get<1>(result));
	rt.add_numeric_field("p", std::get<2>(result));
}

static void stat_ftest(Runtime &rt)
{
	auto &x = rt.to_array(1);
	auto &y = rt.to_array(2);

	if (x.ndim() != 1 || y.ndim() != 1) {
		throw error("F-test expected two one-dimensional arrays");
	}
	String s = rt.arg_count() > 2 ? rt.to_string(3) : "two-tailed";
	stats::Alternative alt = alternative(s);

	auto result = stats::f_test(x, y, alt);
	rt.new_object();
	rt.add_numeric_field("f", std::get<0>(result));
	rt.add_numeric_field("df1", std::get<1>(result));
	rt.add_numeric_field("df2", std::get<2>(result));
	rt.add_numeric_field(("p"), std::get<3>(result));
}

static void stat_ttest(Runtime &rt)
{
	auto &x = rt.to_array(1);
	auto &y = rt.to_array(2);

	if (x.ndim() != 1 || y.ndim() != 1) {
		throw error("t-test expected two one-dimensional arrays");
	}
	bool var_equal = false;
	if (rt.arg_count() > 2)
	{
		if (rt.is_boolean(3))
		{
			var_equal = rt.to_boolean(3);
		}
		else
		{
			throw error("Third argument in t-test should be a Boolean value (var_equal)");
		}
	}
	String s = rt.arg_count() > 3 ? rt.to_string(4) : "two-tailed";
	stats::Alternative alt = alternative(s);

	auto result = var_equal ? stats::student_ttest2(x, y, alt) : stats::welch_ttest2(x, y, alt);
	rt.new_object();
	const char *method = var_equal ? "student" : "welch";
	rt.add_string_field("method", method);
	rt.add_numeric_field("t", std::get<0>(result));
	rt.add_numeric_field("df", std::get<1>(result));
	rt.add_numeric_field(("p"), std::get<2>(result));
}

static void stat_ttest1(Runtime &rt)
{
	auto &x = rt.to_array(1);
	auto mu = rt.to_number(2);
	String s = rt.arg_count() > 2 ? rt.to_string(3) : "two-tailed";
	stats::Alternative alt = alternative(s);
	auto result = stats::student_ttest1(x, mu, alt);
	rt.new_object();
	rt.add_numeric_field("t", std::get<0>(result));
	rt.add_numeric_field("df", std::get<1>(result));
	rt.add_numeric_field(("p"), std::get<2>(result));
}

//TODO: check here for reports: https://valelab4.ucsf.edu/svn/3rdpartypublic/boost/libs/math/doc/sf_and_dist/html/math_toolkit/dist/stat_tut/weg/f_eg.html
void Runtime::init_stats()
{
	add_global_function("sum", stat_sum, 1);
	add_global_function("mean", stat_mean, 1);
	add_global_function("variance", stat_var, 1);
	add_global_function("std", stat_std, 1);
	add_global_function("chi2_test", stat_chi2, 1);
	add_global_function("f_test", stat_ftest, 2);
	add_global_function("t_test", stat_ttest, 2);
	add_global_function("t_test1", stat_ttest1, 2);
}

} // namespace phonometrica