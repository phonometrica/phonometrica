#ifndef RESET_GENERAL_SETTINGS_PHON_SCRIPT_INCLUDE
#define RESET_GENERAL_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *reset_general_settings_script = R"_(phon.settings.match_window_length = 40
phon.settings.autosave = false
phon.settings.autoload = false

if system.name == "windows" then
	phon.settings.resources_directory = "C:\\Program Files\\Phonometrica"
elsif system.name == "macos" then
	phon.settings.resources_directory = "/Applications/Phonometrica.app/Contents/Resources"
else
	phon.settings.resources_directory = "/usr/local/share/phonometrica"
end

)_";

#endif /* RESET_GENERAL_SETTINGS_PHON_SCRIPT_INCLUDE */