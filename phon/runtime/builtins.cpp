/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 31/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: builtin functions.                                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/func_generic.hpp>
#include <phon/runtime/func_file.hpp>
#include <phon/runtime/func_string.hpp>
#include <phon/runtime/func_list.hpp>
#include <phon/runtime/func_table.hpp>
#include <phon/runtime/func_regex.hpp>
#include <phon/runtime/func_set.hpp>
#include <phon/runtime/func_math.hpp>
#include <phon/runtime/func_array.hpp>
#include <phon/runtime/func_module.hpp>
#include <phon/runtime/func_system.hpp>
#include <phon/runtime.hpp>

#define CLS(T) get_class<T>()
#define REF(bits) ParamBitset(bits)

namespace phonometrica {

static Variant boolean_init(Runtime &, std::span<Variant>)
{
	return false;
}

static Variant integer_init(Runtime &, std::span<Variant>)
{
	return intptr_t(0);
}

static Variant float_init(Runtime &, std::span<Variant>)
{
	return 0.0;
}


void Runtime::set_global_namespace()
{
	// Constructors for primitive types
	auto bool_class = Class::get<bool>();
	bool_class->add_initializer(boolean_init, { });
	auto int_class = Class::get<intptr_t>();
	int_class->add_initializer(integer_init, { });
	auto float_class = Class::get<double>();
	float_class->add_initializer(float_init, { });

	// Generic functions
	add_global("type", get_type, { CLS(Object) });
	add_global("len", get_length, { CLS(Object) });
	add_global("str", to_string, { CLS(Object) });
	add_global("bool", to_boolean, { CLS(Object) });
	add_global("int", to_integer, { CLS(Object) });
	add_global("float", to_float, { CLS(Object) });
	add_global("load_json", load_json, { CLS(String) });
	add_global("dump_json", dump_json, { CLS(Table) });
	add_global("import", import1, { CLS(String) });
	add_global("import", import2, { CLS(String), CLS(bool) });

	// Module type.
	auto module_class = Class::get<Module>();
	module_class->add_initializer(module_init, { CLS(String) });
	module_class->add_method(get_field_string, module_get_attr, {CLS(Module), CLS(String)});
	module_class->add_method(set_field_string, module_set_attr, {CLS(Module), CLS(String), CLS(Object)}, REF("001"));
	module_class->add_method(get_item_string, module_get_attr, {CLS(Module), CLS(String)});
	module_class->add_method(set_item_string, module_set_attr, {CLS(Module), CLS(String), CLS(Object)}, REF("001"));
	add_global("contains", module_contains, { CLS(Module), CLS(String) });

	// Math functions
	add_global("abs", math_abs, { CLS(Number) });
	add_global("abs", math_array_func<std::abs>, { CLS(Array<double>) });
	add_global("acos", math_acos, { CLS(Number) });
	add_global("acos", math_array_func<std::acos>, { CLS(Array<double>) });
	add_global("asin", math_asin, { CLS(Number) });
	add_global("asin", math_array_func<std::asin>, { CLS(Array<double>) });
	add_global("atan", math_atan, { CLS(Number) });
	add_global("atan", math_array_func<std::atan>, { CLS(Array<double>) });
	add_global("atan2", math_atan2, { CLS(Number), CLS(Number) });
	add_global("ceil", math_ceil, { CLS(Number) });
	add_global("ceil", math_array_func<std::ceil>, { CLS(Array<double>) });
	add_global("cos", math_cos, { CLS(Number) });
	add_global("cos", math_array_func<std::cos>, { CLS(Array<double>) });
	add_global("exp", math_exp, { CLS(Number) });
	add_global("exp", math_array_func<std::exp>, { CLS(Array<double>) });
	add_global("floor", math_floor, { CLS(Number) });
	add_global("floor", math_array_func<std::floor>, { CLS(Array<double>) });
	add_global("log", math_log, { CLS(Number) });
	add_global("log", math_array_func<std::log>, { CLS(Array<double>) });
	add_global("log10", math_log10, { CLS(Number) });
	add_global("log10", math_array_func<std::log10>, { CLS(Array<double>) });
	add_global("log2", math_log2, { CLS(Number) });
	add_global("log2", math_array_func<std::log2>, { CLS(Array<double>) });
	add_global("max", math_max, { CLS(Number), CLS(Number) });
	add_global("max", math_max2, { CLS(intptr_t), CLS(intptr_t) });
	add_global("min", math_min, { CLS(Number), CLS(Number) });
	add_global("min", math_min2, { CLS(intptr_t), CLS(intptr_t) });
	add_global("random", math_random, { });
	add_global("round", math_round, { CLS(Number) });
	add_global("round", math_array_func<round_float>, { CLS(Array<double>) });
	add_global("round", math_roundn, { CLS(Number), CLS(Number) });
	add_global("sin", math_sin, { CLS(Number) });
	add_global("sin", math_array_func<std::sin>, { CLS(Array<double>) });
	add_global("sqrt", math_sqrt, { CLS(Number) });
	add_global("sqrt", math_array_func<std::sqrt>, { CLS(Array<double>) });
	add_global("tan", math_tan, { CLS(Number) });
	add_global("tan", math_array_func<std::tan>, { CLS(Array<double>) });
	add_global("E", 2.7182818284590452354);
	add_global("PI", 3.1415926535897932);
	add_global("SQRT2", 1.4142135623730951);
	add_global("PHI", 1.6180339887498948);

	// String
	add_global("contains", string_contains, { CLS(String), CLS(String) });
	add_global("starts_with", string_starts_with, {CLS(String), CLS(String)});
	add_global("ends_with", string_ends_with, {CLS(String), CLS(String)});
	add_global("find", string_find1, { CLS(String), CLS(String) });
	add_global("find", string_find2, { CLS(String), CLS(String), CLS(intptr_t) });
	add_global("find_back", string_find_back1, {CLS(String), CLS(String)});
	add_global("find_back", string_find_back2, {CLS(String), CLS(String), CLS(intptr_t)});
	add_global("left", string_left, { CLS(String), CLS(intptr_t) });
	add_global("right", string_right, { CLS(String), CLS(intptr_t) });
	add_global("slice", string_slice1, {CLS(String), CLS(intptr_t)});
	add_global("slice", string_slice2, {CLS(String), CLS(intptr_t), CLS(intptr_t)});
	add_global("count", string_count,  { CLS(String), CLS(String) });
	add_global("to_upper", string_to_upper, {CLS(String)});
	add_global("to_lower", string_to_lower, {CLS(String)});
	add_global("reverse", string_reverse, { CLS(String) }, REF("1"));
	add_global("is_empty", string_is_empty, {CLS(String)});
	add_global("char", string_char, { CLS(String), CLS(intptr_t) });
	add_global("split", string_split, { CLS(String), CLS(String) });
	add_global("append", string_append, { CLS(String), CLS(String) }, REF("01"));
	add_global("prepend", string_prepend, { CLS(String), CLS(String) }, REF("01"));
	add_global("insert", string_insert, { CLS(String), CLS(intptr_t), CLS(String) }, REF("001"));
	add_global("trim", string_trim, { CLS(String) }, REF("1"));
	add_global("ltrim", string_ltrim, { CLS(String) }, REF("1"));
	add_global("rtrim", string_rtrim, { CLS(String) }, REF("1"));
	add_global("remove", string_remove, { CLS(String), CLS(String) }, REF("01"));
	add_global("remove_first", string_remove_first, {CLS(String), CLS(String)}, REF("01"));
	add_global("remove_last", string_remove_last, {CLS(String), CLS(String)}, REF("01"));
	add_global("remove_at", string_remove_at, {CLS(String), CLS(intptr_t), CLS(intptr_t)}, REF("001"));
	add_global("replace", string_replace, { CLS(String), CLS(String), CLS(String) }, REF("001"));
	add_global("replace_first", string_replace_first, {CLS(String), CLS(String), CLS(String)}, REF("001"));
	add_global("replace_last", string_replace_last, {CLS(String), CLS(String), CLS(String)}, REF("001"));
	add_global("replace_at", string_replace_at, {CLS(String), CLS(intptr_t), CLS(intptr_t), CLS(String)}, REF("0001"));
	auto string_class = Class::get<String>();
	string_class->add_initializer(string_init, { });
	string_class->add_method(get_field_string, string_get_field, {CLS(String), CLS(String)});

	// List
	add_global("contains", list_contains, { CLS(List), CLS(Object) });
	add_global("first", list_first, { CLS(List) });
	add_global("last", list_last, { CLS(List) });
	add_global("find", list_find1, { CLS(List), CLS(Object) });
	add_global("find", list_find2, { CLS(List), CLS(Object), CLS(intptr_t) });
	add_global("find_back", list_rfind_back1, {CLS(List), CLS(Object)});
	add_global("find_back", list_rfind_back2, {CLS(List), CLS(Object), CLS(intptr_t)});
	add_global("left", list_left, { CLS(List), CLS(intptr_t) });
	add_global("right", list_right, { CLS(List), CLS(intptr_t) });
	add_global("join", list_join, { CLS(List), CLS(String) });
	add_global("clear", list_clear, { CLS(List) }, REF("1"));
	add_global("append", list_append, {CLS(List), CLS(Object)}, REF("01"));
	add_global("prepend", list_prepend, {CLS(List), CLS(Object)}, REF("01"));
	add_global("is_empty", list_is_empty, {CLS(List)});
	add_global("pop", list_pop, { CLS(List) }, REF("1"));
	add_global("shift", list_shift, { CLS(List) }, REF("1"));
	add_global("sort", list_sort, { CLS(List) }, REF("1"));
	add_global("sorted_find", list_sorted_find, { CLS(List), CLS(Object) });
	add_global("sorted_insert", list_sorted_insert, { CLS(List), CLS(Object) }, REF("01"));
	add_global("is_sorted", list_is_sorted, { CLS(List) });
	add_global("reverse", list_reverse, { CLS(List) }, REF("1"));
	add_global("remove", list_remove, { CLS(List), CLS(Object) }, REF("01"));
	add_global("remove_first", list_remove_first, { CLS(List), CLS(Object) }, REF("01"));
	add_global("remove_last", list_remove_last, { CLS(List), CLS(Object) }, REF("01"));
	add_global("remove_at", list_remove_at, { CLS(List), CLS(intptr_t) }, REF("01"));
	add_global("shuffle", list_shuffle, { CLS(List) }, REF("1"));
	add_global("sample", list_sample, { CLS(List), CLS(intptr_t) });
	add_global("insert", list_insert, { CLS(List), CLS(intptr_t), CLS(Object) }, REF("001"));
	add_global("intersect", list_intersect, { CLS(List), CLS(List) });
	add_global("unite", list_unite, { CLS(List), CLS(List) });
	add_global("subtract", list_subtract, { CLS(List), CLS(List) });
	auto list_class = Class::get<List>();
	list_class->add_initializer(list_init, { });
	list_class->add_method(get_item_string, list_get_item, {CLS(List), CLS(intptr_t)});
	list_class->add_method(set_item_string, list_set_item, { CLS(List), CLS(intptr_t), CLS(Object) }, REF("001"));
	list_class->add_method(get_field_string, list_get_field, { CLS(List), CLS(String) });

	// File
	add_global("open", file_open1, { CLS(String) });
	add_global("open", file_open2, { CLS(String), CLS(String) });
	add_global("read_line", file_read_line, { CLS(File) });
	add_global("read_lines", file_read_lines, { CLS(File) });
	add_global("write_line", file_write_line, {CLS(File), CLS(String) });
	add_global("write_lines", file_write_lines, {CLS(File), CLS(List) });
	add_global("write", file_write, {CLS(File), CLS(String) });
	add_global("close", file_close, {CLS(File) });
	add_global("read", file_read, {CLS(File)});
	add_global("read_file", read_file, {CLS(String)});
	add_global("rewind", file_rewind, {CLS(File) });
	add_global("tell", file_tell, { CLS(File) });
	add_global("seek", file_seek, { CLS(File), CLS(intptr_t) });
	add_global("eof", file_eof, { CLS(File) });
	auto file_class = Class::get<File>();
	auto &v = (*globals)["open"];
	file_class->add_initializer(v.handle<Function>());
	file_class->add_method(get_field_string, file_get_field, {CLS(File), CLS(String)});

	// Table
	add_global("contains", table_contains, { CLS(Table), CLS(Object) });
	add_global("is_empty", table_is_empty, { CLS(Table) });
	add_global("clear", table_clear, { CLS(Table) }, REF("1"));
	add_global("remove", table_remove, { CLS(Table), CLS(Object) }, REF("01"));
	add_global("get", table_get1, { CLS(Table), CLS(Object) });
	add_global("get", table_get2, { CLS(Table), CLS(Object), CLS(Object) });
	auto table_class = Class::get<Table>();
	table_class->add_initializer(table_init, { });
	table_class->add_method(get_item_string, table_get_item, { CLS(Table), CLS(Object) });
	table_class->add_method(set_item_string, table_set_item, { CLS(Table), CLS(Object), CLS(Object) }, REF("001"));
	table_class->add_method(get_field_string, table_get_field, { CLS(Table), CLS(String) });

	// Array
	add_global("zeros", array_zeros1, { CLS(intptr_t) });
	add_global("zeros", array_zeros2, { CLS(intptr_t), CLS(intptr_t) });
	add_global("ones", array_ones1, { CLS(intptr_t) });
	add_global("ones", array_ones2, { CLS(intptr_t), CLS(intptr_t) });
	add_global("min", array_min, { CLS(Array<double>) });
	add_global("max", array_max, { CLS(Array<double>) });
	add_global("clear", array_clear, { CLS(Array<double>) }, REF("1"));
	auto array_class = Class::get<Array<double>>();
	auto &zeros = (*globals)["zeros"];
	array_class->add_initializer(zeros.handle<Function>());
	array_class->add_method(get_item_string, array_get_item1, { CLS(Array<double>), CLS(intptr_t) });
	array_class->add_method(get_item_string, array_get_item2, { CLS(Array<double>), CLS(intptr_t), CLS(intptr_t) });
	array_class->add_method(set_item_string, array_set_item1, { CLS(Array<double>), CLS(intptr_t), CLS(Number) }, REF("001"));
	array_class->add_method(set_item_string, array_set_item2, { CLS(Array<double>), CLS(intptr_t), CLS(intptr_t), CLS(Number) }, REF("0001"));
	array_class->add_method(get_field_string, array_get_field, { CLS(Array<double>), CLS(String) });

	// Regex
	auto regex_class = Class::get<Regex>();
	regex_class->add_initializer(regex_new1, {CLS(String)});
	regex_class->add_initializer(regex_new2, {CLS(String), CLS(String)});
	regex_class->add_method(get_field_string, regex_get_field, {CLS(Regex), CLS(String)});
	add_global("match", regex_match1, {CLS(Regex), CLS(String) });
	add_global("match", regex_match2, {CLS(Regex), CLS(String), CLS(intptr_t) });
	add_global("has_match", regex_has_match, {CLS(Regex) });
	add_global("count", regex_count, {CLS(Regex) });
	add_global("group", regex_group, {CLS(Regex), CLS(intptr_t) });
	add_global("get_start", regex_get_start, {CLS(Regex), CLS(intptr_t)});
	add_global("get_end", regex_get_end, {CLS(Regex), CLS(intptr_t)});

	// Set
	add_global("contains", set_contains, {CLS(Set), CLS(Object) });
	add_global("insert", set_insert, {CLS(Set), CLS(Object) }, REF("01"));
	add_global("remove", set_remove, {CLS(Set), CLS(Object) }, REF("01"));
	add_global("is_empty", set_is_empty, {CLS(Set) });
	add_global("clear", set_clear, {CLS(Set) }, REF("1"));
	add_global("intersect", set_intersect, {CLS(Set), CLS(Set) });
	add_global("unite", set_unite, {CLS(Set), CLS(Set) });
	add_global("subtract", set_subtract, {CLS(Set), CLS(Set) });
	auto set_class = Class::get<Set>();
	set_class->add_initializer(set_init, {});
	set_class->add_method(get_field_string, set_get_field, {CLS(Set), CLS(String) });

	// System functions
	add_global("get_user_directory", system_user_directory, {});
	add_global("get_current_directory", system_current_directory, {});
	add_global("set_current_directory", system_set_current_directory, {CLS(String) });
	add_global("get_temp_directory", system_temp_directory, {});
	add_global("get_path_separator", system_separator, {});
	add_global("get_os_name", system_name, {});
	add_global("get_full_path", system_full_path, {CLS(String) });
	add_global("join_path", system_join, {CLS(String), CLS(String) });
	add_global("get_temp_name", system_temp_name, {});
	add_global("get_base_name", system_base_name, {CLS(String) });
	add_global("get_directory", system_get_directory, {CLS(String) });
	add_global("create_directory", system_create_directory, {CLS(String) });
	add_global("remove_directory", system_remove_directory1,  {CLS(String) });
	add_global("remove_directory", system_remove_directory2,  {CLS(String), CLS(bool) });
	add_global("remove_file", system_remove_file,  {CLS(String) });
	add_global("remove_path", system_remove, {CLS(String) });
	add_global("list_directory", system_list_directory1, {CLS(String) });
	add_global("list_directory", system_list_directory2, {CLS(String), CLS(bool) });
	add_global("exists", system_exists,  {CLS(String) });
	add_global("is_document", system_is_file,  {CLS(String) });
	add_global("is_directory", system_is_directory,  {CLS(String) });
	add_global("clear_directory", system_clear_directory,  {CLS(String) });
	add_global("rename", system_rename,  {CLS(String), CLS(String) });
	add_global("split_extension", system_split_extension,  {CLS(String) });
	add_global("get_extension", system_get_extension1,  {CLS(String) });
	add_global("get_extension", system_get_extension2,  {CLS(String), CLS(bool) });
	add_global("strip_extension", system_strip_extension,  {CLS(String) });
	add_global("genericize", system_genericize,  {CLS(String) });
	add_global("nativize", system_nativize,  {CLS(String) });
}


} // namespace phonometrica

#undef CLS
#undef REF
