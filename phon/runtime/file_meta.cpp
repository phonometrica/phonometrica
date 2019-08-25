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
 * Created: 23/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: File metaobject.                                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/variant.hpp>
#include <phon/runtime/object.hpp>
#include <phon/runtime/toplevel.hpp>

namespace phonometrica {

static void new_file(Runtime &rt)
{
    String mode = rt.is_string(2) ? rt.to_string(2) : "r";
    String path = rt.to_string(1);

    // Get prototype
    rt.current_function();
    rt.get_field(-1, "meta");
    auto meta = rt.to_object(-1);

    auto obj = new Object(rt, PHON_CFILE, meta);

    try
    {
        new (&obj->as.file) File(path, mode);
    }
    catch (std::runtime_error &e)
    {
        delete obj;
        throw rt.raise("Input/Output error", e);
    }

    rt.push(obj);
}

static void file_close(Runtime &rt)
{
    auto &file = rt.to_file(0);
    file.close();
}

static void file_to_string(Runtime &rt)
{
    auto &file = rt.to_file(0);
    rt.push(file.path());
}

static void file_eof(Runtime &rt)
{
    auto &file = rt.to_file(0);
    rt.push_boolean(file.at_end());
}

static void file_rewind(Runtime &rt)
{
    auto &file = rt.to_file(0);
    file.rewind();
    rt.push_null();
}

static void file_read_line(Runtime &rt)
{
    auto &file = rt.to_file(0);
    auto ln = file.read_line();

    // Optional argument: if true, trim white space (and new line) at the end
    if (rt.to_boolean(1))
    {
        ln.rtrim();
    }
    rt.push(std::move(ln));
}

static void file_write_line(Runtime &rt)
{
    auto &file = rt.to_file(0);
    auto ln = rt.to_string(1);

    if (file.writable())
    {
        file.write_line(ln);
    }
    else
    {
        throw rt.raise("Error", "%s", "cannot write to read-only file");
    }

    rt.push_null();
}

static void file_write_lines(Runtime &rt)
{
    auto &file = rt.to_file(0);
    if (!file.writable())
    {
        throw rt.raise("Error", "%s", "cannot write to read-only file");
    }

    auto lst = rt.to_list(1);

    for (auto &v : lst)
    {
        auto s = v.to_string(rt);
        file.write(s);
    }

    rt.push_null();
}

static void file_write(Runtime &rt)
{
    auto &file = rt.to_file(0);
    auto txt = rt.to_string(1);

    if (file.writable())
    {
        file.write(txt);
    }
    else
    {
        throw rt.raise("Error", "%s", "cannot write to read-only file");
    }

    rt.push_null();
}

static void file_read_all(Runtime &rt)
{
    auto &file = rt.to_file(0);
    String text;

    while (!file.at_end())
    {
        text.append(file.read_line());
    }

    rt.push(std::move(text));
}

static void file_read_byte(Runtime &rt)
{
    auto &file = rt.to_file(0);
    rt.push(file.read_byte());
}

static void file_write_byte(Runtime &rt)
{
    auto &file = rt.to_file(0);
    auto b = rt.to_number(1);
    file.write_byte((int)b);
    rt.push_null();
}

static void file_read_lines(Runtime &rt)
{
    auto &file = rt.to_file(0);
    Array<Variant> result;

    for (auto &ln : file.read_lines())
    {
        result.emplace_back(std::move(ln));
    }

    rt.push(std::move(result));
}

void Runtime::init_file()
{
    push(file_meta);
    {
        add_method("File.meta.to_string", file_to_string, 0);
        add_method("File.meta.eof", file_eof, 0);
        add_method("File.meta.read_line", file_read_line, 0);
        add_method("File.meta.read_lines", file_read_lines, 0);
        add_method("File.meta.read_all", file_read_all, 0);
        add_method("File.meta.rewind", file_rewind, 0);
        add_method("File.meta.read_byte", file_read_byte, 0);
        add_method("File.meta.write_byte", file_write_byte, 1);
        add_method("File.meta.write", file_write, 1);
        add_method("File.meta.write_lines", file_write_lines, 1);
        add_method("File.meta.write_line", file_write_line, 1);
        add_method("File.meta.close", file_close, 0);
    }
    new_native_constructor(new_file, new_file, "File", 1);
    def_global("File", PHON_DONTENUM);
}

} // namespace phonometrica