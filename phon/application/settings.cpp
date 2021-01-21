//
// Created by julien on 15/01/2021.
//

#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/runtime/file.hpp>
#include <phon/include/read_settings_phon.hpp>

//#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/write_settings_phon.hpp>
//#include <phon/include/reset_general_settings_phon.hpp>
//#include <phon/include/reset_sound_settings_phon.hpp>
//#endif

namespace phonometrica {

Runtime *Settings::runtime = nullptr;
static String phon_key("phon"), settings_key("settings");
static String last_dir_key("last_directory");

void Settings::initialize(Runtime *rt)
{
	runtime = rt;
	static String res("resources_directory");

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

	auto get_documentation_directory = [](Runtime &, std::span<Variant>) -> Variant {
		auto dir = get_string(res);
		return filesystem::join(dir, "html");
	};

	rt->add_global("get_settings_directory", get_settings_directory, {});
	rt->add_global("get_metadata_directory", get_metadata_directory, {});
	rt->add_global("get_plugin_directory", get_plugin_directory, {});
	rt->add_global("get_script_directory", get_script_directory, {});
	rt->add_global("get_config_path", get_config_path, {});
	rt->add_global("get_documentation_directory", get_documentation_directory, {});
}

String Settings::settings_directory()
{
#if PHON_WINDOWS
	auto name = "Phonometrica";
#elif PHON_MACOS
	auto name = ".phonometrica";
#else
	auto name = "phonometrica";
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
	// Get "phon.settings.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));

	return cast<String>(settings.get(name));
}

bool Settings::get_boolean(const String &name)
{
	// Get "phon.settings.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));

	return cast<bool>(settings.get(name));
}

double Settings::get_number(const String &name)
{
	// Get "phon.settings.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));

	return settings.get(name).get_number();
}

Array<Variant> &Settings::get_list(const String &name)
{
	// Get "phon.settings.name"
	auto &phon = cast<Module>((*runtime)[phon_key]);
	auto &settings = cast<Table>(phon.get(settings_key));

	return cast<List>(settings.get(name)).items();
}

String Settings::get_std_script(String name)
{
	auto path = Settings::get_string("resources_directory");
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

	return filesystem::join(Settings::get_string("resources_directory"), name);
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
	run_script((*runtime), write_settings);
}

String Settings::icon_directory()
{
	return filesystem::join(Settings::get_string("resources_directory"), "icons");
}

String Settings::get_icon_path(std::string_view name)
{
	return filesystem::join(icon_directory(), name);
}


} // namespace phonometrica