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
 * Purpose: File builtin functions.                                                                                    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FUNC_FILE_HPP
#define PHONOMETRICA_FUNC_FILE_HPP

#include <phon/file.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

static Variant file_get_field(Runtime &rt, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	auto &key = cast<String>(args[1]);
	if (key == rt.length_string) {
		return f.size();
	}
	else if (key == "path") {
		return f.path();
	}

	throw error("[Index error] String type has no member named \"%\"", key);
}

static Variant file_open1(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return make_handle<File>(path);
}

static Variant file_open2(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	auto &mode = cast<String>(args[1]);

	return make_handle<File>(path, mode.data());
}

static Variant file_read_line(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	return f.read_line();
}

static Variant file_write_line(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	auto &text = cast<String>(args[1]);
	f.write_line(text);

	return Variant();
}

static Variant file_write_lines(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	auto &lines = cast<List>(args[1]).items();
	for (auto &line : lines) {
		f.write_line(line.to_string());
	}

	return Variant();
}

static Variant file_write(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	auto &text = cast<String>(args[1]);
	f.write(text);

	return Variant();
}

static Variant file_close(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	f.close();

	return Variant();
}

static Variant file_rewind(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	f.rewind();

	return Variant();
}

static Variant file_read(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	String line, text;

	while (!f.at_end())
	{
		text.append(f.read_line());
	}

	return text;
}

static Variant read_file(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return File::read_all(path);
}

static Variant file_read_lines(Runtime &rt, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	Array<Variant> result;

	for (auto &ln : f.read_lines()) {
		result.append(std::move(ln));
	}

	return make_handle<List>(&rt, std::move(result));
}

static Variant file_seek(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	auto pos = cast<intptr_t>(args[1]);
	f.seek(pos);

	return Variant();
}

static Variant file_tell(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	return f.tell();
}

static Variant file_eof(Runtime &, std::span<Variant> args)
{
	auto &f = cast<File>(args[0]);
	return f.at_end();
}

} // namespace phonometrica

#endif // PHONOMETRICA_FUNC_FILE_HPP
