//
// Created by julien on 15/01/2021.
//

#ifndef PHONOMETRICA_SETTINGS_HPP
#define PHONOMETRICA_SETTINGS_HPP

#include <phon/runtime.hpp>

namespace phonometrica {

class Settings final
{
public:

	static void initialize(Runtime *rt);

    static String settings_directory();

    static String plugin_directory();

    static String metadata_directory();

    static String user_script_directory();

    static String icon_directory();

    static String get_icon_path(std::string_view name);

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
    
private:

	static Runtime *runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SETTINGS_HPP
