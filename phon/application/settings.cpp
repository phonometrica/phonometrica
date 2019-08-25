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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/application/settings.hpp>
#include <phon/file.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/utils/print.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/include/read_settings_phon.hpp>
#include "settings.hpp"


#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/write_settings_phon.hpp>
#endif

namespace phonometrica {

static String phon_key("phon"), settings_key("settings");
static String last_dir_key("last_directory");

String Settings::settings_directory()
{
#if PHON_LINUX
    auto name = "phonometrica";
#else
    auto name = "Phonometrica";
#endif

    return filesystem::join(filesystem::application_directory(), name);
}

String Settings::plugin_directory()
{
	auto path = settings_directory();
	filesystem::append(path, "Plugins");

	return path;
}

String Settings::metadata_directory()
{
	auto path = settings_directory();
	filesystem::append(path, "Metadata");

	return path;
}

String Settings::user_script_directory()
{
	auto path = settings_directory();
	filesystem::append(path, "Scripts");

	return path;
}

String Settings::config_path()
{
	auto path = settings_directory();
	filesystem::append(path, "settings.phon");

	return path;
}

void Settings::read(Runtime &rt)
{
    // `read_settings_script` must always be embedded because we need to resources directory
    // to be set before we can load a script from disk.
	String content;
    auto path = config_path();

	if (filesystem::exists(path))
	{
		content = File::read_all(path);
		if (content.trim().empty())
        {
		    content = read_settings_script;
        }
	}
	else
	{
		content = read_settings_script;
	}
	rt.do_string(content);

	// Sanity check
	rt.do_string(R"__(
if (not phon.contains("settings")) then
    error("Settings could not be initialized properly: check the file '" + phon.config.path + "'")
end)__");
}

void Settings::write(Runtime &rt)
{
	run_script(rt, write_settings);
}

void Settings::initialize(Runtime &rt)
{
	static String res("resources_directory");

	auto get_settings_directory = [](Runtime &rt) {
		rt.push(Settings::settings_directory());
	};

	auto get_metadata_directory = [](Runtime &rt) {
		rt.push(Settings::metadata_directory());
	};

	auto get_plugin_directory = [](Runtime &rt) {
		rt.push(Settings::plugin_directory());
	};

	auto get_script_directory = [](Runtime &rt) {
		rt.push(Settings::user_script_directory());
	};

	auto get_config_path = [](Runtime &rt) {
		rt.push(Settings::config_path());
	};

	auto get_documentation_directory = [](Runtime &rt) {
		auto dir = get_string(rt, res);
		rt.push(filesystem::join(dir, "html"));
	};

	rt.push(new Object(rt, PHON_COBJECT, rt.object_meta));
	{
		rt.add_accessor("settings_directory", get_settings_directory);
		rt.add_accessor("metadata_directory", get_metadata_directory);
		rt.add_accessor("plugin_directory", get_plugin_directory);
		rt.add_accessor("script_directory", get_script_directory);
		rt.add_accessor("path", get_config_path);
		rt.add_accessor("documentation_directory", get_documentation_directory);
	}
	rt.set_field(-2, "config");
}

String Settings::get_string(Runtime &rt, const String &name)
{
	// Get "phon.settings.name"
	rt.get_global(phon_key);
	rt.get_field(-1, settings_key);
	rt.get_field(-1, name);
	auto value = rt.to_string(-1);
	rt.pop(3);

	return value;
}

bool Settings::get_boolean(Runtime &rt, const String &name)
{
    // Get "phon.settings.name"
    rt.get_global(phon_key);
    rt.get_field(-1, settings_key);
    rt.get_field(-1, name);
    auto value = rt.to_boolean(-1);
    rt.pop(3);

    return value;
}

double Settings::get_number(Runtime &rt, const String &name)
{
    // Get "phon.settings.name"
    rt.get_global(phon_key);
    rt.get_field(-1, settings_key);
    rt.get_field(-1, name);
    auto value = rt.to_number(-1);
    rt.pop(3);

    return value;
}

Array<Variant> &Settings::get_list(Runtime &rt, const String &name)
{
    // Get "phon.settings.name"
    rt.get_global(phon_key);
    rt.get_field(-1, settings_key);
    rt.get_field(-1, name);
    auto &value = rt.to_list(-1);
    rt.pop(3);

    return value;
}

String Settings::get_std_script(Runtime &rt, String name)
{
	auto path = Settings::get_string(rt, "resources_directory");
	filesystem::append(path, "std");
	filesystem::nativize(name);
	name.append(".phon");
	filesystem::append(path, name);

	return path;
}

String Settings::get_last_directory(Runtime &rt)
{
    return get_string(rt, last_dir_key);
}

void Settings::set_value(Runtime &rt, const String &key, String value)
{
    rt.get_global(phon_key);
	rt.get_field(-1, settings_key);
	{
		rt.push(std::move(value));
	}
	rt.set_field(-2, key);
    rt.pop(2);
}

void Settings::set_value(Runtime &rt, const String &key, Array<Variant> value)
{
    rt.get_global(phon_key);
	rt.get_field(-1, settings_key);
	{
		rt.push(std::move(value));
	}
	rt.set_field(-2, key);
    rt.pop(2);
}


void Settings::set_last_directory(Runtime &rt, const String &path)
{
	if (!path.empty()) {
		set_value(rt, last_dir_key, filesystem::directory_name(path));
	}
}

void Settings::set_value(Runtime &rt, const String &key, bool value)
{
	rt.get_global(phon_key);
	rt.get_field(-1, settings_key);
	{
		rt.push_boolean(value);
	}
	rt.set_field(-2, key);
	rt.pop(2);
}

void Settings::set_value(Runtime &rt, const String &key, double value)
{
	rt.get_global(phon_key);
	rt.get_field(-1, settings_key);
	{
		rt.push(value);
	}
	rt.set_field(-2, key);
	rt.pop(2);
}

int Settings::get_int(Runtime &rt, const String &name)
{
	return int(get_number(rt, name));
}

double Settings::get_number(Runtime &rt, const String &category, const String &name)
{
    // Get "phon.settings.category.name"
    rt.get_global(phon_key);
    rt.get_field(-1, settings_key);
    rt.get_field(-1, category);
    rt.get_field(-1, name);
    auto value = rt.to_number(-1);
    rt.pop(4);

    return value;
}

void Settings::set_value(Runtime &rt, const String &category, const String &key, double value)
{
    rt.get_global(phon_key);
    rt.get_field(-1, settings_key);
    {
        rt.get_field(-1, category);
        {
            rt.push(value);
        }
        rt.set_field(-2, key);
    }
    rt.pop(3);
}


} // namespace phonometrica