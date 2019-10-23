#ifndef SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE
#define SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE

static const char *speech_analysis_script = R"_(function report_intensity(time)
    var sound = get_current_sound()
    if sound then
        var dB = sound.get_intensity(time)
        print("Intensity at time " + time + "s = " + dB + "dB")
    else
        alert("No sound or annotation view is currently selected!")
    end
end
)_";

#endif /* SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE */