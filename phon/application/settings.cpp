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
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/runtime/file.hpp>
#include <phon/include/read_settings_phon.hpp>
#ifdef PHON_GUI
#include <phon/application/macros.hpp>
#endif

//#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/write_settings_phon.hpp>
//#include <phon/include/reset_general_settings_phon.hpp>
//#include <phon/include/reset_sound_settings_phon.hpp>
//#endif

namespace phonometrica {

Runtime *Settings::runtime = nullptr;
String Settings::std_resource_path;
#ifdef PHON_GUI
wxFont Settings::mono_font;
#endif
static String phon_key("phon"), settings_key("settings");
static String last_dir_key("last_directory");

void Settings::initialize(Runtime *rt)
{
	runtime = rt;
	using namespace filesystem;

#if PHON_WINDOWS
	std_resource_path = directory_name(rt->program_path());
#elif PHON_MACOS
	std_resource_path = directory_name(directory_name(directory_name(rt->program_path())));
#else
	std_resource_path = "/usr/local/share/phonometrica";
#endif

	// Create global functions related to settings
	auto get_settings_directory = [](Runtime &, std::span<Variant>) -> Variant {
		return Settings::settings_directory();
	};

	auto get_metadata_directory = [](Runtime &, std::span<Variant>) -> Variant {
		return Settings::metadata_directory();
	};

	auto get_plugin_directory = [](Runtime &, std::span<Variant>) -> Variant {
		return Settings::plugin_directory();
	};

	auto get_script_directory = [](Runtime &, std::span<Variant>) -> Variant {
		return Settings::user_script_directory();
	};

	auto get_config_path = [](Runtime &, std::span<Variant>) -> Variant {
		return Settings::config_path();
	};

	rt->add_global("get_settings_directory", get_settings_directory, {});
	rt->add_global("get_metadata_directory", get_metadata_directory, {});
	rt->add_global("get_plugin_directory", get_plugin_directory, {});
	rt->add_global("get_script_directory", get_script_directory, {});
	rt->add_global("get_config_path", get_config_path, {});
}

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

String Settings::get_string(const String &name)
{
	try
	{
		// Get "phon.settings.name"
		auto &phon = cast<Module>((*runtime)[phon_key]);
		auto &settings = cast<Table>(phon.get(settings_key));

		return cast<String>(settings.get(name));
	}
	catch (std::runtime_error &e)
	{
		throw error("Invalid setting \"%\": %", name, e.what());
	}
}

bool Settings::get_boolean(const String &name)
{
	try
	{
		// Get "phon.settings.name"
		auto &phon = cast<Module>((*runtime)[phon_key]);
		auto &settings = cast<Table>(phon.get(settings_key));

		return cast<bool>(settings.get(name));
	}
	catch (std::runtime_error &e)
	{
		throw error("Invalid setting \"%\": %", name, e.what());
	}
}

double Settings::get_number(const String &name)
{
	try
	{
		// Get "phon.settings.name"
		auto &phon = cast<Module>((*runtime)[phon_key]);
		auto &settings = cast<Table>(phon.get(settings_key));

		return settings.get(name).get_number();
	}
	catch (std::runtime_error &e)
	{
		throw error("Invalid setting \"%\": %", name, e.what());
	}
}

Array<Variant> &Settings::get_list(const String &name)
{
	try
	{
		// Get "phon.settings.name"
		auto &phon = cast<Module>((*runtime)[phon_key]);
		auto &settings = cast<Table>(phon.get(settings_key));

		return cast<List>(settings.get(name)).items();
	}
	catch (std::runtime_error &e)
	{
		throw error("Invalid setting \"%\": %", name, e.what());
	}
}

Hashmap<Variant, Variant> &Settings::get_table(const String &name)
{
    try
    {
        auto &phon = cast<Module>((*runtime)[phon_key]);
        auto &settings = cast<Table>(phon.get(settings_key));

        return cast<Table>(settings.get(name)).data();
    }
    catch (std::runtime_error &e)
    {
        throw error("Invalid setting \"%\": %", name, e.what());
    }
}

String Settings::get_std_script(String name)
{
	auto path = Settings::resources_directory();
	filesystem::append(path, "std");
	filesystem::nativize(name);
	name.append(".phon");
	filesystem::append(path, name);

	return path;
}

String Settings::get_last_directory()
{
	return get_string(last_dir_key);
}

void Settings::set_value(const String &key, Variant value)
{
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));
	settings[key] = std::move(value);
}

void Settings::set_value(const String &key, Array<Variant> value)
{
	set_value(key, make_handle<List>(runtime, std::move(value)));
}


void Settings::set_last_directory(const String &path)
{
	if (!path.empty()) {
		set_value(last_dir_key, filesystem::directory_name(path));
	}
}

int Settings::get_int(const String &name)
{
	return int(get_number(name));
}

bool Settings::get_boolean(const String &category, const String &name)
{
	// Get "phon.settings.category.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));
	auto &mod = cast<Table>(settings.get(category));

	return cast<bool>(mod.get(name));
}

double Settings::get_number(const String &category, const String &name)
{
	// Get "phon.settings.category.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));
	auto &mod = cast<Table>(settings.get(category));

	return mod.get(name).get_number();
}

String Settings::get_string(const String &category, const String &name)
{
	// Get "phon.settings.category.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));
	auto &mod = cast<Table>(settings.get(category));

	return cast<String>(mod.get(name));
}

void Settings::set_value(const String &category, const String &key, Variant value)
{
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));
	auto &mod = cast<Table>(settings.get(category));
	mod[key] = std::move(value);
}

String Settings::get_std_plugin_directory()
{
#if PHON_LINUX
	String name("plugins");
#else
	String name("Plugins");
#endif

	return filesystem::join(Settings::resources_directory(), name);
}

void Settings::reset_general_settings()
{
	//run_script(rt, reset_general_settings);
}

void Settings::reset_sound_settings()
{
//	run_script(rt, reset_sound_settings);
}

void Settings::read()
{
	// `read_settings_script` must always be embedded because we need the resources directory
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
	Variant result;

	try
	{
		result = runtime->do_string(content);
	}
	catch (std::exception &)
	{
		// TODO: notify user that settings are invalid and have been reinitialized.
		result = runtime->do_string(read_settings_script);
	}
	// Versions of Phonometrica prior to 0.8 created phon.settings in settings.phon.
	// We now simply store a table in this file, and create settings.phon ourselves to
	// hide it from users.
	if (result.empty())
	{
		// Sanity checks
		runtime->do_string(R"__(
			if not contains(phon, "settings") then
			    throw "Settings could not be initialized properly: check the file '" & get_config_path() & "'"
			end
		)__");
	}
	else
	{
		auto &phon = cast<Module>((*runtime)[phon_key]);
		phon["settings"] = std::move(result);
	}
}

void Settings::write()
{
#if PHON_GUI
    // FIXME: wxFont::SetNativeFontInfo() doesn't seem to work on Windows, and it crashes on Linux,
    //  so we serialize the font ourselves
    auto table = make_handle<Table>(runtime);
    auto &map = table->data();

	map["size"] = intptr_t(mono_font.GetPointSize());
	map["family"] = intptr_t(mono_font.GetFamily());
	map["style"] = intptr_t(mono_font.GetStyle());
	map["name"] = String(mono_font.GetFaceName());
	Settings::set_value("font", std::move(table));
#endif
	run_script((*runtime), write_settings);
}

String Settings::get_documentation_page(String page)
{
	filesystem::nativize(page);
	auto path = filesystem::join(resources_directory(), "html", page);
	if (!path.ends_with(".html")) {
		filesystem::append(path, "index.html");
	}
	path.prepend("file://");

	return path;
}

String Settings::resources_directory()
{
	return std_resource_path;
}

#ifdef PHON_GUI
wxFont Settings::get_mono_font()
{
	assert(mono_font.IsOk());
	return mono_font;
}

void Settings::set_mono_font(const wxFont &font)
{
	mono_font = font;
}
#endif // PHON_GUI

void Settings::post_initialize()
{
#ifdef PHON_GUI
	// Ensure we have a valid Monospace font
	try
	{
        auto &map = get_table("font");
        auto size = int(cast<intptr_t>(map["size"]));
        auto family = wxFontFamily(cast<intptr_t>(map["family"]));
        auto style = wxFontStyle(cast<intptr_t>(map["style"]));
        auto name = wxString(cast<String>(map["name"]));
		mono_font = wxFont(size, family, style, wxFONTWEIGHT_NORMAL, false, name);
	}
	catch (...)
	{
		mono_font = MONOSPACE_FONT;
	}
#endif
}

} // namespace phonometrica