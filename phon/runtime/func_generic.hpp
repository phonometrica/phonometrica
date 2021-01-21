/***********************************************************************************************************************
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
 * Purpose: generic builtin functions.                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_GENERIC_HPP
#define PHONOMETRICA_FUNC_GENERIC_HPP

#include <phon/file.hpp>
#include <phon/regex.hpp>
#include <phon/runtime.hpp>
#include <phon/runtime/set.hpp>

namespace phonometrica {

static Variant get_type(Runtime &, std::span<Variant> args)
{
	return args[0].get_class()->object();
}

static Variant get_length(Runtime &, std::span<Variant> args)
{
	auto &v = args[0];

	if (check_type<String>(v)) {
		return cast<String>(v).grapheme_count();
	}
	else if (check_type<List>(v)) {
		return cast<List>(v).size();
	}
	else if (check_type<Table>(v)) {
		return cast<Table>(v).size();
	}
	else if (check_type<Array<double>>(v)) {
		return cast<Array<double>>(v).size();
	}
	else if (check_type<Set>(v)) {
		return cast<Set>(v).size();
	}
	else if (check_type<Regex>(v)) {
		return cast<Regex>(v).count();
	}
	else if (check_type<File>(v)) {
		return cast<File>(v).size();
	}

	throw error("[Type error] Cannot get length of % value", v.class_name());
}

static Variant to_string(Runtime &, std::span<Variant> args)
{
	return args[0].to_string();
}

static Variant to_boolean(Runtime &, std::span<Variant> args)
{
	return args[0].to_boolean();
}

static Variant to_integer(Runtime &, std::span<Variant> args)
{
	return args[0].to_integer();
}

static Variant to_float(Runtime &, std::span<Variant> args)
{
	return args[0].to_float();
}

static Variant load_json(Runtime &rt, std::span<Variant> args)
{
	auto &s = cast<String>(args[0]);
	return rt.do_string(s);
}

static Variant dump_json(Runtime &, std::span<Variant> args)
{
	return args[0].to_json();
}

static Variant import1(Runtime &rt, std::span<Variant> args)
{
	auto &name = cast<String>(args[0]);
	return rt.import_module(name);
}

static Variant import2(Runtime &rt, std::span<Variant> args)
{
	auto &name = cast<String>(args[0]);
	auto reload = cast<bool>(args[1]);

	return reload ? rt.reload_module(name) : rt.import_module(name);
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_GENERIC_HPP
