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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: read/write application settings. This class only has static methods.                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SETTINGS_HPP
#define PHONOMETRICA_SETTINGS_HPP

#ifdef PHON_GUI
#include <wx/font.h>
#endif
#include <phon/runtime.hpp>

namespace phonometrica {

class Settings final
{
public:

	static void initialize(Runtime *rt);

	static void post_initialize();

    static String settings_directory();

    static String plugin_directory();

    static String metadata_directory();

    static String user_script_directory();

    static String resources_directory();

    static String config_path();

	static String get_string(const String &name);

	static bool get_boolean(const String &name);

	static bool get_boolean(const String &category, const String &name);

	static double get_number(const String &name);

	static double get_number(const String &category, const String &name);

	static String get_string(const String &category, const String &name);

	static int get_int(const String &name);

	static Array<Variant> &get_list(const String &name);

	static void set_value(const String &key, Variant value);

	static void set_value(const String &key, Array<Variant> value);

	static void set_value(const String &category, const String &key, Variant value);

	static String get_std_plugin_directory();

    static String get_std_script(String name);

    static String get_last_directory();

    static void set_last_directory(const String &path);

    static void reset_general_settings();

    static void reset_sound_settings();

    static void read();

    static void write();

    static String get_documentation_page(String page);

#ifdef PHON_GUI
    static wxFont get_mono_font();

    static void set_mono_font(const wxFont &font);
#endif

private:

	static Runtime *runtime;

	static String std_resource_path;

#ifdef PHON_GUI
	static wxFont mono_font;
#endif
};

} // namespace phonometrica

#endif // PHONOMETRICA_SETTINGS_HPP
