#ifndef RESET_SPECTROGRAM_SETTINGS_PHON_SCRIPT_INCLUDE
#define RESET_SPECTROGRAM_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *reset_spectrogram_settings_script = R"_(phon.settings.spectrogram = {
		window_size: 0.005,
		frequency_range: 5500,
		window_type: "Hann",
		dynamic_range: 70,
		preemphasis_threshold: 1000
}
)_";

#endif /* RESET_SPECTROGRAM_SETTINGS_PHON_SCRIPT_INCLUDE */