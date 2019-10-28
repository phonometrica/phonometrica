#ifndef SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE
#define SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE

static const char *speech_analysis_script = R"_(function report_intensity(time)
    var sound = get_current_sound()
    if sound then
        var dB = sound.get_intensity(time)
        print("Intensity at time " + time + " s = " + dB + " dB")
    else
        alert("No sound or annotation view is currently selected!")
    end
end

function report_pitch(time)
    var sound = get_current_sound()
    if sound then
        var f0 = sound.get_pitch(time)
        if f0 then
            print("Pitch at time " + time + " s = " + f0 + " Hz")
        else
            print("Pitch at time " + time + " s = undefined")
        end
    else
        alert("No sound or annotation view is currently selected!")
    end
end

function report_formants(time)
    print("Formants at time " + time + " s:")
    var sound = get_current_sound()
    if sound then
        var result = sound.get_formants(time)
        var nformant = result.row_count

        for var i = 1 to nformant do
            var label = "F" + String(i)
            var fmt = result.get(i, 1)
            var bw = result.get(i, 2)

            if fmt then
                print(label + " = " + fmt + " Hz\t bandwidth = " + bw + " Hz")
            else
                print(label + " = undefined")
            end
        end
    else
        alert("No sound or annotation view is currently selected!")
    end
end)_";

#endif /* SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE */