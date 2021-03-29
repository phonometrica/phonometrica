#ifndef SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE
#define SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE

static const char *speech_analysis_script = R"_(
function report_intensity(time)
local sound = get_current_sound()
if sound then
local dB = get_intensity(sound, time)
print "Intensity at time ", time, " s = ", dB, " dB"
else
alert("No sound or annotation view is currently selected!")
end
end

function report_pitch(time)
local sound = get_current_sound()
if sound then
local f0 = get_pitch(sound, time)
if f0 then
print "Pitch at time ", time, " s = ", f0, " Hz"
else
print "Pitch at time ", time, " s = undefined"
end
else
alert("No sound or annotation view is currently selected!")
end
end

function report_formants(time as Number)
print "Formants at time ", time, " s:"
local sound = get_current_sound()
if sound then
local result = get_formants(sound, time)
local nformant = result.nrow

for i = 1 to nformant do
local label = "F" & i
local fmt = result[i, 1]
local bw = result[i, 2]

if fmt then
print label, " = ", fmt, " Hz\t bandwidth = ", bw, " Hz"
else
print label, " = undefined"
end
end
else
alert("No sound or annotation view is currently selected!")
end
end

function report_formants(t1 as Number, t2 as Number)
local t = t1 + (t2 - t1) / 2
report_formants(t)
end
)_";

#endif /* SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE */