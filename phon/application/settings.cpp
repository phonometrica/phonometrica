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
#include <phon/runtime/environment.hpp>
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

void Settings::read(Environment &env)
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
	env.do_string(content);

	// Sanity check
	env.do_string(R"__(
if (not phon.contains("settings")) then
    error("Settings could not be initialized properly: check the file '" + phon.config.path + "'")
end)__");
}

void Settings::write(Environment &env)
{
	run_script(env, write_settings);
}

void Settings::initialize(Environment &env)
{
	static String res("resources_directory");

	auto get_settings_directory = [](Environment &env) {
		env.push(Settings::settings_directory());
	};

	auto get_metadata_directory = [](Environment &env) {
		env.push(Settings::metadata_directory());
	};

	auto get_plugin_directory = [](Environment &env) {
		env.push(Settings::plugin_directory());
	};

	auto get_script_directory = [](Environment &env) {
		env.push(Settings::user_script_directory());
	};

	auto get_config_path = [](Environment &env) {
		env.push(Settings::config_path());
	};

	auto get_documentation_directory = [](Environment &env) {
		auto dir = get_string(env, res);
		env.push(filesystem::join(dir, "html"));
	};

	env.push(new Object(env, PHON_COBJECT, env.object_meta));
	{
		env.add_accessor("settings_directory", get_settings_directory);
		env.add_accessor("metadata_directory", get_metadata_directory);
		env.add_accessor("plugin_directory", get_plugin_directory);
		env.add_accessor("script_directory", get_script_directory);
		env.add_accessor("path", get_config_path);
		env.add_accessor("documentation_directory", get_documentation_directory);
	}
	env.set_field(-2, "config");
}

String Settings::get_string(Environment &env, const String &name)
{
	// Get "phon.settings.name"
	env.get_global(phon_key);
	env.get_field(-1, settings_key);
	env.get_field(-1, name);
	auto value = env.to_string(-1);
	env.pop(3);

	return value;
}

bool Settings::get_boolean(Environment &env, const String &name)
{
    // Get "phon.settings.name"
    env.get_global(phon_key);
    env.get_field(-1, settings_key);
    env.get_field(-1, name);
    auto value = env.to_boolean(-1);
    env.pop(3);

    return value;
}

double Settings::get_number(Environment &env, const String &name)
{
    // Get "phon.settings.name"
    env.get_global(phon_key);
    env.get_field(-1, settings_key);
    env.get_field(-1, name);
    auto value = env.to_number(-1);
    env.pop(3);

    return value;
}

Array<Variant> &Settings::get_list(Environment &env, const String &name)
{
    // Get "phon.settings.name"
    env.get_global(phon_key);
    env.get_field(-1, settings_key);
    env.get_field(-1, name);
    auto &value = env.to_list(-1);
    env.pop(3);

    return value;
}

String Settings::get_std_script(Environment &env, String name)
{
	auto path = Settings::get_string(env, "resources_directory");
	filesystem::append(path, "std");
	filesystem::nativize(name);
	name.append(".phon");
	filesystem::append(path, name);

	return path;
}

String Settings::get_last_directory(Environment &env)
{
    return get_string(env, last_dir_key);
}

void Settings::set_value(Environment &env, const String &key, String value)
{
    env.get_global(phon_key);
	env.get_field(-1, settings_key);
	{
		env.push(std::move(value));
	}
	env.set_field(-2, key);
    env.pop(2);
}

void Settings::set_value(Environment &env, const String &key, Array<Variant> value)
{
    env.get_global(phon_key);
	env.get_field(-1, settings_key);
	{
		env.push(std::move(value));
	}
	env.set_field(-2, key);
    env.pop(2);
}


void Settings::set_last_directory(Environment &env, const String &path)
{
	if (!path.empty()) {
		set_value(env, last_dir_key, filesystem::directory_name(path));
	}
}

void Settings::set_value(Environment &env, const String &key, bool value)
{
	env.get_global(phon_key);
	env.get_field(-1, settings_key);
	{
		env.push_boolean(value);
	}
	env.set_field(-2, key);
	env.pop(2);
}

void Settings::set_value(Environment &env, const String &key, double value)
{
	env.get_global(phon_key);
	env.get_field(-1, settings_key);
	{
		env.push(value);
	}
	env.set_field(-2, key);
	env.pop(2);
}

int Settings::get_int(Environment &env, const String &name)
{
	return int(get_number(env, name));
}

double Settings::get_number(Environment &env, const String &category, const String &name)
{
    // Get "phon.settings.category.name"
    env.get_global(phon_key);
    env.get_field(-1, settings_key);
    env.get_field(-1, category);
    env.get_field(-1, name);
    auto value = env.to_number(-1);
    env.pop(4);

    return value;
}

void Settings::set_value(Environment &env, const String &category, const String &key, double value)
{
    env.get_global(phon_key);
    env.get_field(-1, settings_key);
    {
        env.get_field(-1, category);
        {
            env.push(value);
        }
        env.set_field(-2, key);
    }
    env.pop(3);
}


} // namespace phonometrica