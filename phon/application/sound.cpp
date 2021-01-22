/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <set>
#if PHON_WINDOWS
#include <windows.h>
#endif
#include <sndfile.h>
//#include <iostream>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/sound.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/resampler.hpp>
#include <phon/analysis/signal_processing.hpp>
#include <phon/analysis/speech_utils.hpp>
#include <phon/third_party/swipe/swipe.h>
#include <phon/utils/matrix.hpp>

#if PHON_MACOS
const int BUFFER_SIZE = 4096;
#else
const int BUFFER_SIZE = 2048;
#endif

namespace phonometrica {

Array<String> Sound::the_supported_sound_formats;
Array<String> Sound::the_common_sound_formats;
Object *Sound::metaobject = nullptr;

Sound::Sound(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{

}

void Sound::set_sound_formats()
{
	SF_FORMAT_INFO format_info;
	std::set<String> temp;
	int count;

	sf_command(nullptr, SFC_GET_SIMPLE_FORMAT_COUNT, &count, sizeof(int));

	for (int k = 0; k < count; k++) {
		format_info.format = k;
		sf_command(nullptr, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof(format_info));
		temp.emplace(format_info.extension);
	}

	for (auto &format : temp) {
		the_supported_sound_formats.emplace_back(format);
	}

	// it seems that "oga" is the default extension for OGG files in libsndfile
	if (the_supported_sound_formats.contains("oga") && ! the_supported_sound_formats.contains("ogg")) {
		the_supported_sound_formats.emplace_back("ogg");
	}

	the_common_sound_formats.emplace_back("wav");
	the_common_sound_formats.emplace_back("flac");
	the_common_sound_formats.emplace_back("aiff");
}

const Array<String> &Sound::supported_sound_formats()
{
	return the_supported_sound_formats;
}

const Array<String> &Sound::common_sound_formats()
{
	return the_common_sound_formats;
}

bool Sound::is_sound() const
{
	return true;
}

const char *Sound::class_name() const
{
	return "Sound";
}

void Sound::load()
{
    if (!m_data)
    {
		m_data = std::make_shared<AudioData>(handle());
    }
}

void Sound::write()
{

}

bool Sound::supports_format(const String &format)
{
	// Strip leading dot.
	std::string_view fmt(format.data() + 1, format.size() - 1);

	for (auto &f : the_supported_sound_formats) {
		if (f == fmt) {
			return true;
		}
	}

	return false;
}

Array<String> Sound::supported_sound_format_names()
{
	SF_FORMAT_INFO info;
	int count;
	Array<String> formats;

	sf_command(nullptr, SFC_GET_SIMPLE_FORMAT_COUNT, &count, sizeof(int));

	for (int k = 0; k < count; k++)
	{
		info.format = k;
		sf_command(nullptr, SFC_GET_SIMPLE_FORMAT, &info, sizeof(info));
		formats.append(info.name);
	}

	return formats;
}

String Sound::rtaudio_version()
{
	return String(RtAudio::getVersion());
}

String Sound::libsndfile_version()
{
	String sf_version;
	char buffer[128] ;

	sf_command(nullptr, SFC_GET_LIB_VERSION, buffer, sizeof(buffer)) ;
	sf_version = buffer;

	return sf_version.split("-")[2];
}

double Sound::duration() const
{
    auto h = handle();

    return double(h.frames()) / h.samplerate();
}

int Sound::sample_rate() const
{
	return handle().samplerate();
}

intptr_t Sound::nframes() const
{
	return handle().frames();
}

std::shared_ptr<AudioData> Sound::data()
{
	load();
	return m_data;
}

SndfileHandle Sound::handle() const
{
	if (m_data) return m_data->handle();

#if PHON_WINDOWS
	auto wpath = m_path.to_wide();
        return SndfileHandle(wpath.data());
#else
	return SndfileHandle(m_path.data());
#endif
}

int Sound::nchannel() const
{
    return handle().channels();
}

std::shared_ptr<AudioData> Sound::light_data() const
{
	// We don't to load the audio data, so we use the current data if it's already loaded, otherwise we create
	// an AudioData which doesn't load the sound file.
	if (m_data)
	{
		return m_data;
	}

	return std::make_shared<AudioData>(handle(), false);
}

void Sound::convert(const String &path, int sample_rate, Sound::Format fmt)
{
	const int BUFFER_SIZE = 1024;
	double buffer[BUFFER_SIZE];
	int flags = static_cast<int>(fmt) | SF_FORMAT_PCM_32;

#if PHON_WINDOWS
	auto wpath = path.to_wide();
	SndfileHandle outfile(wpath.data(), SFM_WRITE, flags, 1, sample_rate);
#else
	SndfileHandle outfile(path.data(), SFM_WRITE, flags, 1, sample_rate);
#endif
	auto input = this->light_data();
	Resampler resampler(input->sample_rate(), sample_rate, BUFFER_SIZE);
	input->seek(0);
	auto size = input->size();
	double *out = nullptr;
	intptr_t ol = double(size) * sample_rate / input->sample_rate();

	while (ol > 0)
	{
		auto count = input->read(buffer, BUFFER_SIZE);
		if (count == 0) {
			count = BUFFER_SIZE;
			memset(buffer, 0, sizeof(double) * BUFFER_SIZE);
		}
		auto len = resampler.process(buffer, count, out);
		if (len > ol) len = ol;
		outfile.write(out, len);
		ol -= len;
	}

	input->seek(0);
}

int Sound::get_intensity_window_size() const
{
	// Praat's settings
	double min_pitch = 100;
	double effective_duration = 3.2 / min_pitch;

	return int(std::ceil(effective_duration * m_data->sample_rate()));
}

Array<double>
Sound::get_formants(const Array<double> &times, int nformant, double nyquist_frequency, double max_bandwidth, double window_size,
                    int lpc_order)
{
	Array<double> result(nformant, 2, 0.0);

	for (auto t : times)
	{
		auto tmp = get_formants(t, nformant, nyquist_frequency, max_bandwidth, window_size, lpc_order);
		for (intptr_t i = 1; i <= nformant; i++)
		{
			for (intptr_t j = 1; j <= 2; j++)
			{
				result(i, j) += tmp(i, j);
			}
		}
	}

	for (intptr_t i = 1; i <= nformant; i++)
	{
		for (intptr_t j = 1; j <= 2; j++)
		{
			result(i, j) /= times.size();
		}
	}

	return result;
}

Array<double> Sound::get_formants(double time, int nformant, double nyquist_frequency, double max_bandwidth, double window_size, int lpc_order)
{
	load();
	Array<double> result(nformant, 2, 0.0);
	using namespace speech;
	PHON_LOG("Calculating formants");
	//std::cerr << "get_formants at time " << time << std::endl;

	window_size *= 2; // for the Gaussian window
	double Fs = nyquist_frequency * 2;
	int nframe_orig = int(ceil(window_size * this->sample_rate()));
	auto first_sample = m_data->time_to_frame(time) - nframe_orig / 2;
	auto last_sample = first_sample + nframe_orig;

	if (first_sample < 1) {
		throw error("File '%': time point % is too close to the beginning of the file", path(), time);
	}
	if (last_sample > m_data->size()) {
		throw error("File '%': time point % is too close to the end of the file", path(), time);
	}

	auto input = m_data->copy(first_sample, last_sample);
	std::vector<double> tmp; // not needed if sampling rates are equal
	std::span<double> output;
	// Apply pre-emphasis from 50 Hz.
	pre_emphasis(input, m_data->sample_rate(), 50);

	if (Fs == m_data->sample_rate())
	{
		output = input;
	}
	else
	{
		tmp = resample(input, m_data->sample_rate(), Fs);
		output = std::span<double>(tmp);
	}
	int nframe = output.size();
	auto win = create_window(nframe, nframe, WindowType::Gaussian);
	Array<double> buffer(nframe, 0.0);

	// Apply window.
	auto it = output.begin();
	for (int j = 1; j <= nframe; j++)
	{
		buffer[j] = *it++ * win[j];
	}

	auto coeffs = get_lpc_coefficients(buffer, lpc_order);
	std::vector<double> freqs, bw;
	bool ok = speech::get_formants(coeffs, Fs, freqs, bw);

	if (!ok)
	{
		for (int i = 1; i <= nformant; i++)
		{
			result(i, 1) = std::nan("");
			result(i, 2) = std::nan("");
		}

		return result;
	}

	int count = 0;
	const double max_freq = Fs / 2 - 50;
	for (int k = 0; k < freqs.size(); k++)
	{
		auto freq = freqs[k];
		if (freq > 50 && freq < max_freq && bw[k] < max_bandwidth)
		{
			result(++count, 1) = freq;
			result(count, 2) = bw[k];
		}
		if (count == nformant) break;
	}
	for (int k = count+1; k <= nformant; k++)
	{
		result(k, 1) = std::nan("");
		result(k, 2) = std::nan("");
	}

	return result;
}

double Sound::get_pitch(double time, double min_pitch, double max_pitch, double threshold)
{
	load();
	PHON_LOG("Calculating pitch");
	const double time_step = 0.01;
	// We use 90ms window centered around the time of measurement, with a time step = 10ms. This will yield a 9 point
	// pitch estimate, and we return the center point (i.e. the 5th).
	intptr_t step = time_step * m_data->sample_rate();
	auto first_sample = m_data->time_to_frame(time) - step * 4.5;
	auto last_sample = first_sample + step * 7;

	if (first_sample < 1) {
		throw error("File '%': time point % is too close to the beginning of the file", path(), time);
	}
	if (last_sample > m_data->size()) {
		throw error("File '%': time point % is too close to the end of the file", path(), time);
	}

	auto input = m_data->get(first_sample, last_sample);
	// Borrow reference to avoid copying
	vector vec;
	vec.x = (int) input.size();
	vec.v = input.data();

	PHON_LOG("Running SWIPE");
	auto tmp = swipe(vec, m_data->sample_rate(), min_pitch, max_pitch, threshold, time_step);
	auto pitch = Array<double>::from_memory(tmp.v, tmp.x);
	PHON_LOG("point estimate at time %f = %f\n", time, pitch[5]);

	return pitch[5];
}

double Sound::get_intensity(double time)
{
	load();
	int window_size = get_intensity_window_size();
	auto first_sample = m_data->time_to_frame(time) - (window_size / 2);
	auto start = m_data->data() + first_sample - 1;

	if (start < m_data->data()) {
		throw error("File '%': time point % is too close to the beginning of the file", path(), time);
	}
	if (start + window_size > m_data->data() + m_data->size()) {
		throw error("File '%': time point % is too close to the end of the file", path(), time);
	}

	std::span<double> frame(start, window_size);
	auto win = speech::create_window(window_size, window_size, speech::WindowType::Hamming);

	return speech::get_intensity(frame, win);
}

std::vector<double> Sound::get_intensity(intptr_t start_pos, intptr_t end_pos, double time_step)
{
	auto input = m_data->get(start_pos, end_pos);
	int window_size = get_intensity_window_size();

	return speech::get_intensity(input, m_data->sample_rate(), window_size, time_step);
}

void Sound::initialize(Runtime &rt)
{
	auto cls = rt.create_type<AutoSound>("Sound", rt.get_object_class());
	rt.add_global("Sound", cls);

	auto sound_get_field = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto &key = cast<String>(args[1]);
		if (key == "path") {
			return sound->path();
		}
		else if (key == "duration") {
			return sound->duration();
		}
		else if (key == "nchannel") {
			return sound->nchannel();
		}
		else if (key == "sample_rate") {
			return sound->sample_rate();
		}
		throw error("[Index error] Sound type has no member named \"%\"", key);
	};

	auto add_property = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &snd = cast<AutoSound>(args[0]);
		auto &category = cast<String>(args[1]);
		snd->open();
		std::any value;

		if (check_type<String>(args[2])) {
			value = cast<String>(args[2]);
		}
		else if (check_type<bool>(args[2])) {
			value = cast<bool>(args[2]);
		}
		else if (args[2].resolve().is_number()) {
			value = args[2].resolve().get_number();
		}
		else {
			throw error("Invalid property type: %", args[2].class_name());
		}
		snd->add_property(Property(category, std::move(value)));

		return Variant();
	};

	auto remove_property = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &snd = cast<AutoSound>(args[0]);
		auto &category = cast<String>(args[1]);
		snd->open();
		snd->remove_property(category);
		return Variant();
	};
	
	auto get_property = [](Runtime &, std::span<Variant> args) -> Variant  {
    	auto &snd = cast<AutoSound>(args[0]);
    	auto category = cast<String>(args[1]);
		snd->open();
    	auto prop = snd->get_property(category);

    	if (prop.valid())
	    {
    		if (prop.is_text())
    			return prop.value();
    		else if (prop.is_numeric())
    			return prop.numeric_value();
    		else if (prop.is_boolean())
    			return prop.boolean_value();
		    else
		    	throw error("[Internal error] Invalid property type");
	    }

    	return Variant();
    };

	auto get_intensity = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		sound->open();
		if (time < 0 || time > sound->duration()) {
			throw error("File '%': invalid time %", sound->path(), time);
		}
		return sound->get_intensity(time);
	};

	auto get_pitch1 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		String category("pitch_tracking");
		auto min_pitch = Settings::get_number(category, "minimum_pitch");
		auto max_pitch = Settings::get_number(category, "minimum_pitch");
		auto threshold = Settings::get_number(category, "voicing_threshold");
		sound->open();
		return sound->get_pitch(time, min_pitch, max_pitch, threshold);
	};

	auto get_pitch2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		auto min_pitch = args[2].resolve().get_number();
		auto max_pitch = args[3].resolve().get_number();
		auto threshold = Settings::get_number("pitch_tracking", "voicing_threshold");
		sound->open();
		return sound->get_pitch(time, min_pitch, max_pitch, threshold);
	};

	auto get_pitch3 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		auto min_pitch = args[2].resolve().get_number();
		auto max_pitch = args[3].resolve().get_number();
		auto threshold = args[4].resolve().get_number();
		sound->open();
		return sound->get_pitch(time, min_pitch, max_pitch, threshold);
	};

	auto get_formants1 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		String category("formants");
		intptr_t nformant = Settings::get_number(category, "number_of_formants");
		double nyquist = Settings::get_number(category, "max_frequency");
		double max_bw = Settings::get_number(category, "max_bandwidth");
		double win_size = Settings::get_number(category, "window_size");
		intptr_t lpc_order = Settings::get_number(category, "lpc_order");
		return sound->get_formants(time, nformant, nyquist, max_bw, win_size, lpc_order);
	};

	auto get_formants2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &sound = cast<AutoSound>(args[0]);
		auto time = args[1].resolve().get_number();
		intptr_t nformant = cast<intptr_t>(args[2]);
		double nyquist = args[3].resolve().get_number();
		double max_bw = args[4].resolve().get_number();
		double win_size = args[5].resolve().get_number();
		intptr_t lpc_order = cast<intptr_t>(args[6]);
		return sound->get_formants(time, nformant, nyquist, max_bw, win_size, lpc_order);
	};

	auto hz2bark1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::hertz_to_bark(args[0].resolve().get_number());
	};

	auto hz2bark2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::hertz_to_bark);
	};

	auto bark2hz1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::bark_to_hertz(args[0].resolve().get_number());
	};

	auto bark2hz2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::bark_to_hertz);
	};

	auto hz2erb1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::hertz_to_erb(args[0].resolve().get_number());
	};

	auto hz2erb2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::hertz_to_erb);
	};

	auto erb2hz1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::erb_to_hertz(args[0].resolve().get_number());
	};

	auto erb2hz2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::erb_to_hertz);
	};

	auto hz2mel1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::hertz_to_mel(args[0].resolve().get_number());
	};

	auto hz2mel2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::hertz_to_mel);
	};

	auto mel2hz1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::mel_to_hertz(args[0].resolve().get_number());
	};

	auto mel2hz2 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		return apply(array, speech::mel_to_hertz);
	};

	auto hz2st1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::hertz_to_semitones(args[0].resolve().get_number());
	};

	auto hz2st2 = [](Runtime &, std::span<Variant> args) -> Variant {
		double f = args[0].resolve().get_number();
		double ref = args[1].resolve().get_number();
		return speech::hertz_to_semitones(f, ref);
	};

	auto hz2st3 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		auto f = [=](double st) { return speech::hertz_to_semitones(st); };
		return apply(array, f);
	};

	auto hz2st4 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		double ref = args[1].resolve().get_number();
		auto f = [=](double st) { return speech::hertz_to_semitones(st, ref); };
		return apply(array, f);
	};


	auto st2hz1 = [](Runtime &, std::span<Variant> args) -> Variant {
		return speech::semitones_to_hertz(args[0].resolve().get_number());
	};

	auto st2hz2 = [](Runtime &, std::span<Variant> args) -> Variant {
		double f = args[0].resolve().get_number();
		double ref = args[1].resolve().get_number();
		return speech::semitones_to_hertz(f, ref);
	};

	auto st2hz3 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		auto f = [=](double st) { return speech::semitones_to_hertz(st); };
		return apply(array, f);
	};

	auto st2hz4 = [](Runtime &, std::span<Variant> args) -> Variant {
		auto &array = cast<Array<double>>(args[0]);
		double ref = args[1].resolve().get_number();
		auto f = [=](double st) { return speech::semitones_to_hertz(st, ref); };
		return apply(array, f);
	};


	
#define CLS(T) get_class<T>()
	cls = CLS(AutoSound);
	cls->add_method("get_field", sound_get_field, { CLS(String) });

	rt.add_global("get_pitch", get_pitch1, { CLS(AutoSound), CLS(Number) });
	rt.add_global("get_pitch", get_pitch2, { CLS(AutoSound), CLS(Number), CLS(Number), CLS(Number) });
	rt.add_global("get_pitch", get_pitch3, { CLS(AutoSound), CLS(Number), CLS(Number), CLS(Number), CLS(Number) });

	rt.add_global("get_formants", get_formants1, { CLS(AutoSound), CLS(Number) });
	rt.add_global("get_formants", get_formants2, { CLS(AutoSound), CLS(Number), CLS(intptr_t), CLS(Number), CLS(Number), CLS(Number), CLS(intptr_t) });

	rt.add_global("hertz_to_bark", hz2bark1, { CLS(Number) });
	rt.add_global("hertz_to_bark", hz2bark2, { CLS(Array<double>) });
	rt.add_global("bark_to_hertz", bark2hz1, { CLS(Number) });
	rt.add_global("bark_to_hertz", bark2hz2, { CLS(Array<double>) });
	rt.add_global("hertz_to_erb", hz2erb1, { CLS(Number) });
	rt.add_global("hertz_to_erb", hz2erb2, { CLS(Array<double>) });
	rt.add_global("erb_to_hertz", erb2hz1, { CLS(Number) });
	rt.add_global("erb_to_hertz", erb2hz2, { CLS(Array<double>) });
	rt.add_global("hertz_to_mel", hz2mel1, { CLS(Number) });
	rt.add_global("hertz_to_mel", hz2mel2, { CLS(Array<double>) });
	rt.add_global("mel_to_hertz", mel2hz1, { CLS(Number) });
	rt.add_global("mel_to_hertz", mel2hz2, { CLS(Array<double>) });
	rt.add_global("hertz_to_semitones", hz2st1, { CLS(Number) });
	rt.add_global("hertz_to_semitones", hz2st2, { CLS(Number), CLS(Number) });
	rt.add_global("hertz_to_semitones", hz2st3, { CLS(Array<double>) });
	rt.add_global("hertz_to_semitones", hz2st4, { CLS(Array<double>), CLS(Number) });
	rt.add_global("semitones_to_hertz", st2hz1, { CLS(Number) });
	rt.add_global("semitones_to_hertz", st2hz2, { CLS(Number), CLS(Number) });
	rt.add_global("semitones_to_hertz", st2hz3, { CLS(Array<double>) });
	rt.add_global("semitones_to_hertz", st2hz4, { CLS(Array<double>), CLS(Number) });

	rt.add_global("add_property", add_property, { CLS(AutoSound), CLS(String), CLS(Object) });
	rt.add_global("remove_property", remove_property, { CLS(AutoSound), CLS(String) });
	rt.add_global("get_property", get_property, { CLS(AutoSound), CLS(String) });
	rt.add_global("get_intensity", get_intensity, { CLS(AutoSound), CLS(Number) });

#undef CLS
}


} // namespace phonometrica