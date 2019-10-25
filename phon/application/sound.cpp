/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
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
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/sound.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/resampler.hpp>
#include <phon/speech/signal_processing.hpp>
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
	auto size = input->size() / input->channels();
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

Matrix<double> Sound::get_formants(double time, int nformant, double nyquist_frequency, double window_size, int lpc_order)
{
	Matrix<double> result(nformant, 2);
	result.setZero(nformant, 2);
	using namespace speech;
	assert(m_data);
	PHON_LOG("Calculating formants");

	double Fs = nyquist_frequency * 2;
	int nframe_orig = int(ceil(window_size * this->sample_rate()));
	auto first_sample = m_data->time_to_frame(time) - nframe_orig / 2;
	auto last_sample = first_sample + nframe_orig;

	if (first_sample < 1) {
		throw error("Time point % is to close to the beginning of the file", time);
	}
	if (last_sample > m_data->size()) {
		throw error("Time point % is to close to the end of the file", time);
	}

	auto input = m_data->copy(first_sample, last_sample);
	std::vector<double> tmp; // not needed if sampling rates are equal
	Span<double> output;
	// Apply pre-emphasis from 50 Hz.
	pre_emphasis(input, m_data->sample_rate(), 50);

	if (Fs == m_data->sample_rate())
	{
		output = input;
	}
	else
	{
		tmp = resample(input, m_data->sample_rate(), Fs);
		output = Span<double>(tmp);
	}
	int nframe = output.size();
	auto win = create_window(nframe, nframe, WindowType::Hann);
	std::vector<double> buffer(nframe, 0.0);

	// Apply window.
	auto it = output.begin();
	for (int j = 0; j < nframe; j++)
	{
		buffer[j] = *it++ * win[j+1];
	}

	auto coeffs = get_lpc_coefficients(buffer, lpc_order);
	std::vector<double> freqs, bw;
	bool ok = speech::get_formants(coeffs, Fs, freqs, bw);

	if (!ok)
	{
		for (int i = 0; i < nformant; i++)
		{
			result(i, 0) = std::nan("");
			result(i, 1) = std::nan("");
		}

		return result;
	}

	int count = 0;
	const double max_freq = Fs / 2 - 50;
	for (int k = 0; k < freqs.size(); k++)
	{
		auto freq = freqs[k];
		if (freq > 50 && freq < max_freq && bw[k] < 400)
		{
			result(count, 0) = freq;
			result(count++, 1) = bw[k];
		}
		if (count == nformant) break;
	}
	for (int k = count; k < nformant; k++)
	{
		result(k, 0) = std::nan("");
		result(k, 1) = std::nan("");
	}

	return result;
}

double Sound::get_pitch(double time, double min_pitch, double max_pitch, double threshold)
{
	assert(m_data);
	PHON_LOG("Calculating pitch");
	const double time_step = 0.01;
	// We use 90ms window centered around the time of measurement, with a time step = 10ms. This will yield a 9 point
	// pitch estimate, and we return the center point (i.e. the 5th).
	intptr_t step = time_step * m_data->sample_rate();
	auto first_sample = m_data->time_to_frame(time) - step * 4.5;
	auto last_sample = first_sample + step * 7;

	if (first_sample < 1) {
		throw error("Time point % is to close to the beginning of the file", time);
	}
	if (last_sample > m_data->size()) {
		throw error("Time point % is to close to the end of the file", time);
	}

	auto input = m_data->get(first_sample, last_sample);
	// Borrow reference to avoid copying
	vector vec;
	vec.x = (int) input.size();
	vec.v = input.data();

	PHON_LOG("Running SWIPE");
	auto tmp = swipe(vec, m_data->sample_rate(), min_pitch, max_pitch, threshold, time_step);
	auto pitch = Array<double>::from_memory(tmp.v, tmp.x);
	PHON_LOG("point estimate at time " << time << " = " pitch[5]);

	return pitch[5];
}

double Sound::get_intensity(double time)
{
	assert(m_data);

	int window_size = get_intensity_window_size();
	auto first_sample = m_data->time_to_frame(time) - (window_size / 2);
	auto start = m_data->data() + first_sample - 1;

	if (start < m_data->data()) {
		throw error("Time point % is to close to the beginning of the file", time);
	}
	if (start + window_size > m_data->data() + m_data->size()) {
		throw error("Time point % is to close to the end of the file", time);
	}

	Span<double> frame(start, window_size);
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
	metaobject = new Object(rt, PHON_CUSERDATA, rt.object_meta);
	rt.permanent_objects.append(metaobject);

	auto new_sound = [](Runtime &rt) {
		rt.push_null(); // TODO: Sound ctor
	};

	auto sound_path = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		rt.push(sound->path());
	};

	auto sound_duration = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		rt.push(sound->duration());
	};

	auto sound_sample_rate = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		rt.push(sound->sample_rate());
	};

	auto sound_channel_count = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		rt.push(sound->nchannel());
	};

	auto add_property = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto category = rt.to_string(1);
		std::any value;
		if (rt.is_boolean(2))
			value = rt.to_boolean(2);
		else if (rt.is_number(2))
			value = rt.to_number(2);
		else
			value = rt.to_string(2);

		sound->add_property(Property(std::move(category), std::move(value)));
		rt.push_null();
	};

	auto remove_property = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto category = rt.to_string(1);
		sound->remove_property(category);
		rt.push_null();
	};

	auto get_property = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto category = rt.to_string(1);
		auto prop = sound->get_property(category);

		if (prop.valid())
		{
			if (prop.is_text())
				rt.push(prop.value());
			else if (prop.is_numeric())
				rt.push(prop.numeric_value());
			else if (prop.is_boolean())
				rt.push_boolean(prop.boolean_value());
			else
			{
				throw error("[Internal error] Invalid property type");
			}
		}
		else
		{
			rt.push_null();
		}
	};

	auto get_intensity = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto time = rt.to_number(1);
		sound->open();
		if (time < 0 || time > sound->duration()) {
			throw error("Invalid time %", time);
		}
		rt.push(sound->get_intensity(time));
	};

	auto get_pitch = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto time = rt.to_number(1);
		sound->open();
		double min_pitch, max_pitch, threshold;
		String category("pitch_tracking");

		if (rt.arg_count() >= 2 && rt.is_number(2))
		{
			min_pitch = rt.to_number(2);
		}
		else
		{
			min_pitch = Settings::get_number(rt, category, "minimum_pitch");
		}
		if (rt.arg_count() >= 3 && rt.is_number(3))
		{
			max_pitch = rt.to_number(3);
		}
		else
		{
			max_pitch = Settings::get_number(rt, category, "maximum_pitch");
		}
		if (rt.arg_count() >= 4 && rt.is_number(4))
		{
			threshold = rt.to_number(4);
		}
		else
		{
			threshold = Settings::get_number(rt, category, "voicing_threshold");
		}
		rt.push(sound->get_pitch(time, min_pitch, max_pitch, threshold));
	};

	auto get_formants = [](Runtime &rt) {
		auto sound = rt.cast_user_data<AutoSound>(0);
		auto time = rt.to_number(1);
		String category("formants");
		double nyquist, win_size;
		int nformant, lpc_order;

		if (rt.arg_count() >= 2)
			nformant = rt.to_integer(2);
		else
			nformant = Settings::get_number(rt, category, "number_of_formants");

		if (rt.arg_count() >= 3)
			nyquist = rt.to_number(3);
		else
			nyquist = Settings::get_number(rt, category, "max_frequency");

		if (rt.arg_count() >= 4)
			win_size = rt.to_number(4);
		else
			win_size = Settings::get_number(rt, category, "window_size");

		if (rt.arg_count() == 5)
			lpc_order = rt.to_integer(5);
		else
			lpc_order = Settings::get_number(rt, category, "lpc_order");

		auto result = sound->get_formants(time, nformant, nyquist, win_size, lpc_order);
		rt.push(std::move(result));
	};

	rt.push(metaobject);
	{
		rt.add_accessor("path", sound_path);
		rt.add_accessor("duration", sound_duration);
		rt.add_accessor("sample_rate", sound_sample_rate);
		rt.add_accessor("channel_count", sound_channel_count);
		rt.add_method("Sound.meta.add_property", add_property, 2);
		rt.add_method("Sound.meta.remove_property", remove_property, 2);
		rt.add_method("Sound.meta.get_property", get_property, 1);
		rt.add_method("Sound.meta.get_intensity", get_intensity, 1);
		rt.add_method("Sound.meta.get_pitch", get_pitch, 1);
		rt.add_method("Sound.meta.get_formants", get_formants, 1);
	}
	rt.new_native_constructor(new_sound, new_sound, "Sound", 1);
	rt.def_global("Sound", PHON_DONTENUM);
}

} // namespace phonometrica