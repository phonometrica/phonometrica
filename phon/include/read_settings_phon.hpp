#ifndef READ_SETTINGS_PHON_SCRIPT_INCLUDE
#define READ_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *read_settings_script = R"_(
phon.settings = {
	recent_projects: [],

	last_directory: "",

	project_ratio: 0.17,

	console_ratio: 0.8,

	info_ratio: 0.8,

	geometry: [0, 0, 0, 0],

	full_screen: true,

	hide_project: false,

	hide_console: false,

	hide_info: false,

	match_window_length: 40,

	autosave: false,

	autoload: false,

	enable_mouse_tracking: true,

	resampling_quality: 6,

	waveform: {
		magnitude: 1.0,
		scaling: "local"
	},

	pitch_tracking: {
        minimum_pitch: 70,
        maximum_pitch: 500,
		time_step: 0.01,
        voicing_threshold: 0.25
    },

	intensity: {
		minimum_intensity: 50,
		maximum_intensity: 100,
		time_step: 0.01
	},

	spectrogram: {
		window_size: 0.005,
		frequency_range: 5500,
		window_type: "Hann",
		dynamic_range: 70,
		preemphasis_threshold: 1000
	}
}

if system.name == "windows" then
	phon.settings.resources_directory = "C:\\Program Files\\Phonometrica"
elsif system.name == "macos" then
	phon.settings.resources_directory = "/Applications/Phonometrica.app/Contents/Resources"
else
	phon.settings.resources_directory = "/usr/local/share/phonometrica"
end

)_";

#endif /* READ_SETTINGS_PHON_SCRIPT_INCLUDE */