#ifndef READ_SETTINGS_PHON_SCRIPT_INCLUDE
#define READ_SETTINGS_PHON_SCRIPT_INCLUDE

const char *read_settings_script = R"_(
phon.settings = {
	recent_projects: [],

	last_directory: "",
	
	use_antialiasing: true,

	project_ratio: 0.17,

	console_ratio: 0.8,

	info_ratio: 0.8,

	geometry: [0, 0, 0, 0],

	full_screen: true,

	hide_project: false,

	hide_console: false,

	hide_info: false,

	match_window_length: 30,

	autosave: false,

	autoload: false,

	enable_mouse_tracking: true,

	resampling_quality: 5,

	pitch_tracking: {
        minimum_pitch: 70,

        maximum_pitch: 500,

        time_step: 0.01,
        
        voicing_threshold: 0.25
    }
}

if system.name == "windows" then
	phon.settings.praat_path = "C:\\Program Files\\Praat.exe"
	phon.settings.resources_directory = "C:\\Program Files\\Phonometrica"

elsif system.name == "macos" then
	phon.settings.praat_path = "/Applications/Praat.app/MacOS/Praat"
	phon.settings.resources_directory = "/Applications/Phonometrica.app/Contents/Resources"
else
	phon.settings.praat_path = "/usr/bin/praat"
	phon.settings.resources_directory = "/usr/local/share/phonometrica"
end

)_";

#endif /* READ_SETTINGS_PHON_SCRIPT_INCLUDE */