#ifndef RESET_FORMANTS_SETTINGS_PHON_SCRIPT_INCLUDE
#define RESET_FORMANTS_SETTINGS_PHON_SCRIPT_INCLUDE

static const char *reset_formants_settings_script = R"_(phon.settings.formants = {
    number_of_formants: 4,
    window_size: 0.025,
    lpc_order: 10,
    max_frequency: 5500,
    max_bandwidth: 400
})_";

#endif /* RESET_FORMANTS_SETTINGS_PHON_SCRIPT_INCLUDE */