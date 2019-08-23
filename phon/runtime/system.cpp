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
#include <phon/runtime/environment.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

namespace fs = filesystem;

static void system_user_directory(Environment &env)
{
    env.push(fs::user_directory());
}

static void system_current_directory(Environment &env)
{
    env.push(fs::current_directory());
}

static void system_set_current_directory(Environment &env)
{
    auto path = env.to_string(-1);
    fs::set_current_directory(path);
    env.push_null();
}

static void system_full_path(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        env.push(fs::full_path(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_separator(Environment &env)
{
    env.push(fs::separator());
}

static void system_name(Environment &env)
{
#if PHON_WINDOWS
    env.push("windows");
#elif PHON_MACOS
    env.push("macos");
#elif PHON_LINUX
    env.push("linux");
#else
    env.push("generic");
#endif
}

static void system_join(Environment &env)
{
    auto top = env.top_count();
    String result = env.to_string(1);

    for (int i = 2; i < top; i++)
    {
        auto chunk = env.to_string(i);
        fs::append(result, chunk);
    }
    env.push(std::move(result));
}

static void system_write(Environment &env)
{
    int top = env.top_count();

    for (int i = 1; i < top; ++i)
    {
        auto s = env.to_string(i);
        env.print(s);
    }
    env.push_null();
}

static void system_temp_directory(Environment &env)
{
    env.push(fs::temp_directory());
}

static void system_temp_name(Environment &env)
{
    env.push(fs::temp_filename());
}

static void system_base_name(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        env.push(fs::base_name(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_get_directory(Environment &env)
{
    auto path = env.to_string(-1);
    env.push(fs::directory_name(path));
}

static void system_create_directory(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        fs::create_directory(path);
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }

    env.push_null();
}

static void system_remove_directory(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        fs::remove_directory(path);
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }

    env.push_null();
}

static void system_remove_file(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        fs::remove_file(path);
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }

    env.push_null();
}

static void system_remove(Environment &env)
{
    auto path = env.to_string(-1);
    try
    {
        fs::remove(path);
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }

    env.push_null();
}

static void system_list_directory(Environment &env)
{
    auto argc = env.arg_count();
    auto path = env.to_string(1);
    bool hidden = false;

    if (argc > 1)
    {
        hidden = env.to_boolean(2);
    }
    try
    {
        Array<Variant> files;

        for (auto &f : fs::list_directory(path, hidden))
        {
            files.emplace_back(std::move(f));
        }
        env.push(std::move(files));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_exists(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        env.push_boolean(fs::exists(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_is_file(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        env.push_boolean(fs::is_file(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_is_directory(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        env.push_boolean(fs::is_directory(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_clear_directory(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        fs::clear_directory(path);
        env.push_null();
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_rename(Environment &env)
{
    auto old_name = env.to_string(1);
    auto new_name = env.to_string(2);

    try
    {
        fs::rename(old_name, new_name);
        env.push_null();
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_split_extension(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        auto pieces = fs::split_ext(path);
        Array<Variant> result;
        result.emplace_back(std::move(pieces.first));
        result.emplace_back(std::move(pieces.second));
        env.push(std::move(result));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_strip_extension(Environment &env)
{
    auto path = env.to_string(1);

    try
    {
        env.push(fs::strip_ext(path));
    }
    catch (std::exception &e)
    {
        throw env.raise("System error", e);
    }
}

static void system_get_extension(Environment &env)
{
    auto argc = env.arg_count();
    auto path = env.to_string(1);
    bool lower = false;

    if (argc > 1)
    {
        lower = env.to_boolean(2);
    }

    env.push(fs::ext(path, lower));
}

static void system_read(Environment &env)
{
    auto filename = env.to_string(1);

    try
    {
        auto content = File::read_all(filename);
        env.push(std::move(content));
    }
    catch (std::exception &e)
    {
        throw env.raise("Intput/Output error", e);
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

static void system_find_module(Environment &env)
{
    auto name = env.to_string(1);
    String cwd;
    // First look in the same directory as the current script.
    auto dir = fs::directory_name(env.filename);
    auto path = find_module(dir, name);

    if (path)
    {
        env.push(*path);
        return;
    }

    // Look in the current directory
    cwd = fs::current_directory();
    if (cwd != dir)
    {
        path = find_module(fs::current_directory(), name);
        if (path)
        {
            env.push(*path);
            return;
        }
    }

    for (auto &dir : env.import_directories)
    {
        path = find_module(dir, name);
        if (path)
        {
            env.push(*path);
            return;
        }
    }

    throw env.raise("Error", "Cannot find module \"%s\"", name.data());
}

static void system_genericize(Environment &env)
{
    auto path = env.to_string(1);
    env.push(fs::genericize(path));
}

static void system_nativize(Environment &env)
{
    auto path = env.to_string(1);
    env.push(fs::nativize(path));
}


void Environment::init_system()
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


