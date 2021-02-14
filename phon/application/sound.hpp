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
 * Purpose: Sound file.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_HPP
#define PHONOMETRICA_SOUND_HPP

#include <phon/application/vfs.hpp>
#include <phon/application/audio_data.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>
#include <phon/utils/matrix.hpp>

namespace phonometrica {

class Runtime;
class Object;


class Sound final : public VFile
{
public:

	enum class Format
	{
		WAV  = SF_FORMAT_WAV,
		AIFF = SF_FORMAT_AIFF,
		FLAC = SF_FORMAT_FLAC
	};

	Sound(VFolder *parent, String path);

	static void set_sound_formats();

	static const Array<String> &supported_sound_formats();

	static const Array<String> &common_sound_formats();

	static Array<String> supported_sound_format_names();

	static String rtaudio_version();

	static String libsndfile_version();

	static bool supports_format(const String &format);

	bool is_sound() const override;

	const char *class_name() const override;

	double duration() const;

	int sample_rate() const;

	intptr_t nframes() const;

    std::shared_ptr<AudioData> data();

    std::shared_ptr<AudioData> light_data() const;

    SndfileHandle handle() const;

    int nchannel() const;

    void convert(const String &path, int sample_rate, Format fmt);

    double get_pitch(double time, double min_pitch, double max_pitch, double threshold);

	double get_intensity(double time);

	std::vector<double> get_intensity(intptr_t start_pos, intptr_t end_pos, double time_step);

	Array<double> get_formants(double time, int nformant, double nyquist_frequency, double max_bandwidth, double window_size, int lpc_order);

	Array<double> get_formants(const Array<double> &times, int nformant, double nyquist_frequency, double max_bandwidth, double window_size, int lpc_order);

	static void initialize(Runtime &rt);

private:

	void load() override;

	void write() override;

	int get_intensity_window_size() const;

	static Array<String> the_supported_sound_formats, the_common_sound_formats;

	std::shared_ptr<AudioData> m_data;
};

using AutoSound = std::shared_ptr<Sound>;

namespace traits {
template<> struct maybe_cyclic<AutoSound> : std::false_type { };
}
} // namespace phonometrica

#endif // PHONOMETRICA_SOUND_HPP
