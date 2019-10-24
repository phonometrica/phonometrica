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
	if (rt.arg_count() != 2) {
		rt.raise("Error", "Array expected two arguments, %d received", rt.arg_count());
	}
	int nrow = rt.to_integer(1);
	int ncol = rt.to_integer(2);
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
	if (rt.arg_count() != 2) {
		rt.raise("Error", "expected 2 arguments, received %d", rt.arg_count());
	}
	intptr_t i = rt.to_integer(1);
	intptr_t j = rt.to_integer(2);
	i = normalize_index(i, array.rows());
	j = normalize_index(j, array.cols());
	rt.push(array(i,j));
}

static void array_set(Runtime &rt)
{
	auto &array = rt.to_array(0);
	if (rt.arg_count() != 3) {
		rt.raise("Error", "expected 3 arguments, received %d", rt.arg_count());
	}
	intptr_t i = rt.to_integer(1);
	intptr_t j = rt.to_integer(2);
	i = normalize_index(i, array.rows());
	j = normalize_index(j, array.cols());
	double value = rt.to_number(3);
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

void Runtime::init_array()
{
	push(array_meta);
	{
		add_accessor("row_count", array_nrow);
		add_accessor("column_count", array_ncol);
		add_method("Array.meta.to_string", array_to_string, 0);
		add_method("Array.meta.get", array_get, 2);
		add_method("Array.meta.set", array_set, 3);
	}
	new_native_constructor(new_array, new_array, "Array", 1);
	def_global("Array", PHON_DONTENUM);
}

} // namespace phonometrica
