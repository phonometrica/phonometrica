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
 * Created: 25/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: module to interact with the operating system.                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 25/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: functions to interact with the operating system.                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

namespace fs = filesystem;

static Variant system_user_directory(Runtime &, std::span<Variant>)
{
	return fs::user_directory();
}

static Variant system_current_directory(Runtime &, std::span<Variant>)
{
	return fs::current_directory();
}

static Variant system_set_current_directory(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	fs::set_current_directory(path);

	return Variant();
}

static Variant system_full_path(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::full_path(path);
}

static Variant system_separator(Runtime &, std::span<Variant>)
{
	return fs::separator();
}

static Variant system_name(Runtime &, std::span<Variant>)
{
#if PHON_WINDOWS
	return "windows";
#elif PHON_MACOS
	return "macos";
#elif PHON_LINUX
	return "linux";
#else
	return "generic";
#endif
}

static Variant system_join(Runtime &, std::span<Variant> args)
{
	auto &s1 = cast<String>(args[0]);
	auto &s2 = cast<String>(args[1]);

	return fs::join(s1, s2);
}

static Variant system_temp_directory(Runtime &, std::span<Variant>)
{
	return fs::temp_directory();
}

static Variant system_temp_name(Runtime &, std::span<Variant>)
{
	return fs::temp_filename();
}

static Variant system_base_name(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::base_name(path);
}

static Variant system_get_directory(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::directory_name(path);
}

static Variant system_create_directory(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	fs::create_directory(path);
	
	return Variant();
}

static Variant system_remove_directory1(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
    fs::remove_directory(path, false);
	
	return Variant();
}

static Variant system_remove_directory2(Runtime &, std::span<Variant> args)
{
    auto &path = cast<String>(args[0]);
    bool recursive = cast<bool>(args[1]);
    fs::remove_directory(path, recursive);

    return Variant();
}

static Variant system_remove_file(Runtime &, std::span<Variant> args)
{
	auto path = cast<String>(args[0]);
	fs::remove_file(path);

	return Variant();
}

static Variant system_remove(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	fs::remove(path);

	return Variant();
}

static Array<Variant> list_directory(const String &path, bool hidden)
{
	Array<Variant> files;

	for (auto &f : fs::list_directory(path, hidden)) {
		files.emplace_back(std::move(f));
	}
	
	return files;	
}

static Variant system_list_directory1(Runtime &rt, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	bool hidden = false;

	return make_handle<List>(&rt, list_directory(path, hidden));
}

static Variant system_list_directory2(Runtime &rt, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	bool hidden = cast<bool>(args[1]);

	return make_handle<List>(&rt, list_directory(path, hidden));
}

static Variant system_exists(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::exists(path);
}

static Variant system_is_file(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::is_file(path);
}

static Variant system_is_directory(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::is_directory(path);
}

static Variant system_clear_directory(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	fs::clear_directory(path);

	return Variant();
}

static Variant system_rename(Runtime &, std::span<Variant> args)
{
	auto &old_name = cast<String>(args[0]);
	auto &new_name = cast<String>(args[1]);
	fs::rename(old_name, new_name);
	
	return Variant();
}

static Variant system_split_extension(Runtime &rt, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	auto pieces = fs::split_ext(path);
	Array<Variant> result;
	result.emplace_back(std::move(pieces.first));
	result.emplace_back(std::move(pieces.second));

	return make_handle<List>(&rt, std::move(result));
}

static Variant system_strip_extension(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::strip_ext(path);
}

static Variant system_get_extension1(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::ext(path, false);
}

static Variant system_get_extension2(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	bool lower = cast<bool>(args[1]);
	
	return fs::ext(path, lower);
}

static Variant system_genericize(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::genericize(path);
}

static Variant system_nativize(Runtime &, std::span<Variant> args)
{
	auto &path = cast<String>(args[0]);
	return fs::nativize(path);
}

} // namespace phonometrica


