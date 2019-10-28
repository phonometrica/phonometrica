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

static intptr_t normalize_index(intptr_t i, intptr_t size)
{
	if (i > 0 && i <= size)
		return --i;
	if (i < 0 && i <= -size)
		return size + i;

	throw error("[Index error] Index % out of range in array dimension with % elements", i, size);
}

static void new_array(Runtime &rt)
{
	int argc = rt.arg_count();
	if (argc < 1 || argc > 2) {
		rt.raise("Error", "Array expected 1 or 2 arguments, %d received", argc);
	}
	intptr_t nrow = rt.to_integer(1);
	intptr_t ncol = (argc == 2) ? rt.to_integer(2) : 1;
	auto obj = new Object(rt, PHON_CARRAY, rt.array_meta);

	try
	{
		new (&obj->as.array) Matrix<double>(nrow, ncol);
	}
	catch (std::runtime_error &e)
	{
		rt.raise("Math error", "%s", e.what());
	}

	rt.push(obj);
}

static void array_nrow(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push_int(array.rows());
}

static void array_ncol(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push_int(array.cols());
}

static void array_get(Runtime &rt)
{
	auto &array = rt.to_array(0);
	int argc = rt.arg_count();
	if (argc < 1 || argc > 2) {
		rt.raise("Error", "expected 1 or 2 arguments, received %d", argc);
	}
	intptr_t i = rt.to_integer(1);
	intptr_t j = (argc == 2) ? rt.to_integer(2) : 0;
	i = normalize_index(i, array.rows());
	j = normalize_index(j, array.cols());
	rt.push(array(i,j));
}

static void array_set(Runtime &rt)
{
	auto &array = rt.to_array(0);
	int argc = rt.arg_count();
	if (argc < 2 || argc > 3) {
		rt.raise("Error", "expected 2 or 3 arguments, received %d", argc);
	}
	intptr_t i, j;
	double value;
	i = rt.to_integer(1);
	i = normalize_index(i, array.rows());

	if (argc == 2)
	{
		j = 0;
		value = rt.to_number(2);
	}
	else
	{
		j = rt.to_integer(2);
		value = rt.to_number(3);
		j = normalize_index(j, array.cols());
	}

	array(i,j) = value;
	rt.push_null();
}

static void array_to_string(Runtime &rt)
{
	auto &array = rt.to_array(0);
	String s;

	for (intptr_t i = 0; i < array.rows(); i++)
	{
		for (intptr_t j = 0; j < array.cols(); j++)
		{
			s.append(String::convert(array(i,j)));
			if (j < array.cols() - 1) {
				s.append(", ");
			}
		}

		if (i < array.rows() - 1) {
			s.append('\n');
		}
	}

	rt.push(std::move(s));
}

static void array_transpose(Runtime &rt)
{
	auto &array = rt.to_array(0);
	rt.push(array.transpose());
}

static void array_add(Runtime &rt)
{
	auto &M = rt.to_array(0);
	double x = rt.to_number(1);

	for (intptr_t i = 0; i < M.rows(); i++)
	{
		for (intptr_t j = 0; j < M.cols(); j++)
		{
			M(i, j) += x;
		}
	}

	rt.push_null();
}

static void array_sub(Runtime &rt)
{
	auto &M = rt.to_array(0);
	double x = rt.to_number(1);

	for (intptr_t i = 0; i < M.rows(); i++)
	{
		for (intptr_t j = 0; j < M.cols(); j++)
		{
			M(i, j) -= x;
		}
	}

	rt.push_null();
}


void Runtime::init_array()
{
	push(array_meta);
	{
		add_accessor("row_count", array_nrow);
		add_accessor("column_count", array_ncol);
		add_method("Array.meta.to_string", array_to_string, 0);
		add_method("Array.meta.get", array_get, 2);
		add_method("Array.meta.set", array_set, 3);
		add_method("Array.meta.transpose", array_transpose, 0);
		add_method("Array.meta.add", array_add, 1);
		add_method("Array.meta.sub", array_sub, 1);
	}
	new_native_constructor(new_array, new_array, "Array", 1);
	def_global("Array", PHON_DONTENUM);
}

} // namespace phonometrica
