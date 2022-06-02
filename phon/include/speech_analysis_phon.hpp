#ifndef SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE
#define SPEECH_ANALYSIS_PHON_SCRIPT_INCLUDE

static const char *speech_analysis_script = R"_(
function report_intensity(time)
local sound = get_current_sound()

if sound then
print "Intensity at time ", time, " s:"
local channels = get_visible_channels()

foreach channel in channels do
local dB = get_intensity(sound, channel, time)

if channel == 0 then
print "Average over all channels: ", dB, " dB"
else
print "Channel ", channel, ": ", dB, " dB"
end
end
else
alert("No sound or annotation view is currently selected!")
end
end

function report_pitch(time)
local sound = get_current_sound()

if sound then
print "Pitch at time ", time, " s:"
local channels = get_visible_channels()

foreach channel in channels do
local f0 = get_pitch(sound, time)

if f0 then
f0 = f0 & "Hz"
else
f0 = "undefined"
end

if channel then
print "Average over all channels: ", f0
else
print "Channel ", channel, ": ", f0
end
end
else
alert("No sound or annotation view is currently selected!")
end
end

function report_formants(time as Number)
local sound = get_current_sound()

if sound then
print "Formants at time ", time, " s:"
local channels = get_visible_channels()

foreach channel in channels do
if channel == 0 then
print "Average over all channels:"
else
print "Channel ", channel, ":"
end
local result = get_formants(sound, channel, time)
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