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
#include <phon/application/resampler.hpp>
#include <phon/speech/signal_processing.hpp>

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

Array<double> Sound::get_intensity(intptr_t start_pos, intptr_t end_pos, double time_step)
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
	}
	rt.new_native_constructor(new_sound, new_sound, "Sound", 1);
	rt.def_global("Sound", PHON_DONTENUM);
}

} // namespace phonometrica