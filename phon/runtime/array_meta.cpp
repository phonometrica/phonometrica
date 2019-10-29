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
 * Created: 24/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Array type, which represents a 2D numeric array (i.e. a matrix).                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

static void new_array(Runtime &rt)
{
	int argc = rt.arg_count();

	if (argc == 1)
	{
		intptr_t n = rt.to_integer(1);
		auto obj = new Object(rt, PHON_CARRAY, rt.array_meta);
		new (&obj->as.array) Array<double>(n, 0.0);
		rt.push(obj);
	}
	else if (argc == 2)
	{
		intptr_t nrow = rt.to_integer(1);
		intptr_t ncol = rt.to_integer(2);
		auto obj = new Object(rt, PHON_CARRAY, rt.array_meta);
		new (&obj->as.array) Array<double>(nrow, ncol);
		rt.push(obj);
	}
	else
	{
		throw rt.raise("Error", "Array expected 1 or 2 arguments, %d received", argc);
	}
}

static void array_nrow(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push_int(array.nrow());
}

static void array_ncol(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push_int(array.ncol());
}

static void array_dim_count(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push_int(array.ndim());
}

static void array_to_string(Runtime &rt)
{
	auto &array = rt.to_array(0);
	String s;
	intptr_t nrow = array.nrow();
	intptr_t ncol = array.ncol();

	for (intptr_t i = 1; i <= nrow; i++)
	{
		for (intptr_t j = 1; j <= ncol; j++)
		{
			s.append(String::convert(array(i,j)));
			if (j < array.ncol() || (ncol == 1 && i < nrow)) s.append(", ");
		}

		if (ncol > 1 && i < array.nrow()) s.append('\n');
	}

	rt.push(std::move(s));
}

static void array_transpose(Runtime &rt)
{
	auto &X = rt.to_array(0);

	rt.push(transpose(X));
}

static void array_add(Runtime &rt)
{
	auto &X = rt.to_array(0);
	double n = rt.to_number(1);
	rt.push(add(X, n));

}

static void array_sub(Runtime &rt)
{
	auto &X = rt.to_array(0);
	double n = rt.to_number(1);
	rt.push(sub(X, n));
}

static void array_mul(Runtime &rt)
{
	auto &X = rt.to_array(0);
	double n = rt.to_number(1);
	rt.push(mul(X, n));
}

static void array_div(Runtime &rt)
{
	auto &X = rt.to_array(0);
	double n = rt.to_number(1);
	rt.push(div(X, n));
}

void Runtime::init_array()
{
	push(array_meta);
	{
		add_accessor("row_count", array_nrow);
		add_accessor("column_count", array_ncol);
		add_accessor("dim_count", array_dim_count);
		add_method("Array.meta.to_string", array_to_string, 0);
		add_method("Array.meta.transpose", array_transpose, 0);
		add_method("Array.meta.add", array_add, 1);
		add_method("Array.meta.sub", array_sub, 1);
		add_method("Array.meta.mul", array_mul, 1);
		add_method("Array.meta.div", array_div, 1);
	}
	new_native_constructor(new_array, new_array, "Array", 1);
	def_global("Array", PHON_DONTENUM);
}

} // namespace phonometrica
