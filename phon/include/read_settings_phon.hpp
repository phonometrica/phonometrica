#ifndef READ_SETTINGS_PHON_SCRIPT_INCLUDE
#define READ_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *read_settings_script = R"_(
phon.settings = {
"recent_projects": [],

"last_directory": "",

"project_ratio": 0.17,

"console_ratio": 0.8,

"info_ratio": 0.8,

"geometry": [0, 0, 0, 0],

"full_screen": true,

"hide_project": false,

"hide_console": false,

"hide_info": false,

"autosave": false,

"autoload": false,

"autohints": true,

"enable_mouse_tracking": true,

"concordance": {
"discard_empty": true,
"context_length": 40
},

"sound_plots": {
"waveform": true,
"spectrogram": true,
"formants": false,
"pitch": true,
"intensity": true
},

"waveform": {
"magnitude": 1.0,
"scaling": "local"
},

"pitch_tracking": {
"minimum_pitch": 70,
"maximum_pitch": 500,
"time_step": 0.01,
"voicing_threshold": 0.25
},

"intensity": {
"minimum_intensity": 50,
"maximum_intensity": 100,
"time_step": 0.01
},

"spectrogram": {
"window_size": 0.005,
"frequency_range": 5500,
"window_type": "Gaussian",
"dynamic_range": 70,
"preemphasis_threshold": 1000
},

"formants": {
"number_of_formants": 4,
"window_size": 0.025,
"lpc_order": 10,
"max_frequency": 5500,
"max_bandwidth": 400
}
}

)_";

#endif /* READ_SETTINGS_PHON_SCRIPT_INCLUDE */