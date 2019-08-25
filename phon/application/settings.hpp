/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: read/write application settings. This class only has static methods.                                      *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SETTINGS_HPP
#define PHONOMETRICA_SETTINGS_HPP

#include <phon/string.hpp>
#include <phon/runtime/variant.hpp>

namespace phonometrica {

class Runtime;


class Settings final
{
public:

    Settings() = delete;

    static void read(Runtime &rt);

    static void write(Runtime &rt);

    static String settings_directory();

    static String plugin_directory();

    static String metadata_directory();

    static String user_script_directory();

    static String config_path();

    static String get_string(Runtime &rt, const String &name);

    static bool get_boolean(Runtime &rt, const String &name);

    static double get_number(Runtime &rt, const String &name);

    static double get_number(Runtime &rt, const String &category, const String &name);

    static int get_int(Runtime &rt, const String &name);

    static Array<Variant> &get_list(Runtime &rt, const String &name);

    static void set_value(Runtime &rt, const String &key, String value);

    static void set_value(Runtime &rt, const String &key, bool value);

    static void set_value(Runtime &rt, const String &key, double value);

    static void set_value(Runtime &rt, const String &key, Array<Variant> value);

    static void set_value(Runtime &rt, const String &category, const String &key, double value);


    static void initialize(Runtime &rt);

    static String get_std_script(Runtime &rt, String name);

    static String get_last_directory(Runtime &rt);

    static void set_last_directory(Runtime &rt, const String &path);
};

} // namespace phonometrica

#endif // PHONOMETRICA_SETTINGS_HPP
