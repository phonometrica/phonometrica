/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This source code is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser       *
 * General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your   *
 * option) any later version.                                                                                          *
 *                                                                                                                     *
 * This source code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the      *
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for    *
 * more details.                                                                                                       *
 *                                                                                                                     *
 * You should have received a copy of the GNU Lesser General Public License along with this source code. If not, see   *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 25/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: module to interact with the operating system.                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/object.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

namespace fs = filesystem;

static void system_user_directory(Runtime &rt)
{
    rt.push(fs::user_directory());
}

static void system_current_directory(Runtime &rt)
{
    rt.push(fs::current_directory());
}

static void system_set_current_directory(Runtime &rt)
{
    auto path = rt.to_string(-1);
    fs::set_current_directory(path);
    rt.push_null();
}

static void system_full_path(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        rt.push(fs::full_path(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_separator(Runtime &rt)
{
    rt.push(fs::separator());
}

static void system_name(Runtime &rt)
{
#if PHON_WINDOWS
    runtime.push("windows");
#elif PHON_MACOS
    runtime.push("macos");
#elif PHON_LINUX
    rt.push("linux");
#else
    runtime.push("generic");
#endif
}

static void system_join(Runtime &rt)
{
    auto top = rt.top_count();
    String result = rt.to_string(1);

    for (int i = 2; i < top; i++)
    {
        auto chunk = rt.to_string(i);
        fs::append(result, chunk);
    }
    rt.push(std::move(result));
}

static void system_write(Runtime &rt)
{
    int top = rt.top_count();

    for (int i = 1; i < top; ++i)
    {
        auto s = rt.to_string(i);
        rt.print(s);
    }
    rt.push_null();
}

static void system_temp_directory(Runtime &rt)
{
    rt.push(fs::temp_directory());
}

static void system_temp_name(Runtime &rt)
{
    rt.push(fs::temp_filename());
}

static void system_base_name(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        rt.push(fs::base_name(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_get_directory(Runtime &rt)
{
    auto path = rt.to_string(-1);
    rt.push(fs::directory_name(path));
}

static void system_create_directory(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        fs::create_directory(path);
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }

    rt.push_null();
}

static void system_remove_directory(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        fs::remove_directory(path);
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }

    rt.push_null();
}

static void system_remove_file(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        fs::remove_file(path);
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }

    rt.push_null();
}

static void system_remove(Runtime &rt)
{
    auto path = rt.to_string(-1);
    try
    {
        fs::remove(path);
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }

    rt.push_null();
}

static void system_list_directory(Runtime &rt)
{
    auto argc = rt.arg_count();
    auto path = rt.to_string(1);
    bool hidden = false;

    if (argc > 1)
    {
        hidden = rt.to_boolean(2);
    }
    try
    {
        Array<Variant> files;

        for (auto &f : fs::list_directory(path, hidden))
        {
            files.emplace_back(std::move(f));
        }
        rt.push(std::move(files));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_exists(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        rt.push_boolean(fs::exists(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_is_file(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        rt.push_boolean(fs::is_file(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_is_directory(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        rt.push_boolean(fs::is_directory(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_clear_directory(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        fs::clear_directory(path);
        rt.push_null();
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_rename(Runtime &rt)
{
    auto old_name = rt.to_string(1);
    auto new_name = rt.to_string(2);

    try
    {
        fs::rename(old_name, new_name);
        rt.push_null();
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_split_extension(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        auto pieces = fs::split_ext(path);
        Array<Variant> result;
        result.emplace_back(std::move(pieces.first));
        result.emplace_back(std::move(pieces.second));
        rt.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_strip_extension(Runtime &rt)
{
    auto path = rt.to_string(1);

    try
    {
        rt.push(fs::strip_ext(path));
    }
    catch (std::exception &e)
    {
        throw rt.raise("System error", e);
    }
}

static void system_get_extension(Runtime &rt)
{
    auto argc = rt.arg_count();
    auto path = rt.to_string(1);
    bool lower = false;

    if (argc > 1)
    {
        lower = rt.to_boolean(2);
    }

    rt.push(fs::ext(path, lower));
}

static void system_read(Runtime &rt)
{
    auto filename = rt.to_string(1);

    try
    {
        auto content = File::read_all(filename);
        rt.push(std::move(content));
    }
    catch (std::exception &e)
    {
        throw rt.raise("Intput/Output error", e);
    }
}

static std::optional<String> find_module(const String &dir, String name)
{
    if (!name.ends_with(".phon"))
        name.append(".phon");

    // Try to find a module
    auto path = fs::join(dir, name);

    if (fs::is_file(path))
        return path;

    // Try to find a package, i.e. a directory containing an "init.phon" script.
    path = fs::join(dir, fs::strip_ext(name));

    if (fs::is_directory(path))
    {
        path = fs::join(path, "init.phon");
        if (fs::is_file(path))
            return path;
    }

    return std::optional<String>();
}

static void system_find_module(Runtime &rt)
{
    auto name = rt.to_string(1);
    String cwd;
    // First look in the same directory as the current script.
    auto dir = fs::directory_name(rt.filename);
    auto path = find_module(dir, name);

    if (path)
    {
        rt.push(*path);
        return;
    }

    // Look in the current directory
    cwd = fs::current_directory();
    if (cwd != dir)
    {
        path = find_module(fs::current_directory(), name);
        if (path)
        {
            rt.push(*path);
            return;
        }
    }

    for (auto &dir : rt.import_directories)
    {
        path = find_module(dir, name);
        if (path)
        {
            rt.push(*path);
            return;
        }
    }

    throw rt.raise("Error", "Cannot find module \"%s\"", name.data());
}

static void system_genericize(Runtime &rt)
{
    auto path = rt.to_string(1);
    rt.push(fs::genericize(path));
}

static void system_nativize(Runtime &rt)
{
    auto path = rt.to_string(1);
    rt.push(fs::nativize(path));
}


void Runtime::init_system()
{
    push(new Object(*this, PHON_CSYSTEM, object_meta));
    {
        add_accessor("user_directory", system_user_directory);
        add_accessor("current_directory", system_current_directory, system_set_current_directory);
        add_accessor("temp_directory", system_temp_directory);
        add_accessor("separator", system_separator);
        add_accessor("name", system_name);
        add_method("get_full_path", system_full_path, 1);
        add_method("join_path", system_join, 0);
        add_method("write", system_write, 0);
        add_method("get_temp_name", system_temp_name, 0);
        add_method("get_base_name", system_base_name, 1);
        add_method("get_directory", system_get_directory, 1);
        add_method("create_directory", system_create_directory, 1);
        add_method("remove_directory", system_remove_directory, 1);
        add_method("remove_file", system_remove_file, 1);
        add_method("remove", system_remove, 1);
        add_method("list_directory", system_list_directory, 1);
        add_method("exists", system_exists, 1);
        add_method("is_file", system_is_file, 1);
        add_method("is_directory", system_is_directory, 1);
        add_method("clear_directory", system_clear_directory, 1);
        add_method("rename", system_rename, 2);
        add_method("split_extension", system_split_extension, 1);
        add_method("get_extension", system_get_extension, 1);
        add_method("strip_extension", system_strip_extension, 1);
        add_method("read_file", system_read, 1);
        add_method("find_module", system_find_module, 1);
        add_method("genericize", system_genericize, 1);
        add_method("nativize", system_nativize, 1);
    }
    def_global("system", PHON_DONTENUM);
}

} // namespace phonometrica


