#ifndef RESET_PITCH_SETTINGS_PHON_SCRIPT_INCLUDE
#define RESET_PITCH_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *reset_pitch_settings_script = R"_(phon.settings.pitch_tracking = {
	minimum_pitch: 70,
	maximum_pitch: 500,
	time_step: 0.01,
	voicing_threshold: 0.25
}
)_";

#endif /* RESET_PITCH_SETTINGS_PHON_SCRIPT_INCLUDE */