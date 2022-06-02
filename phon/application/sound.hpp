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

#if PHON_WINDOWS
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif

#include <cmath>
#include <memory>

#include <phon/application/vfs.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>
#include <phon/utils/matrix.hpp>
#include <sndfile.hh>
#include <phon/array.hpp>
#include <phon/utils/slice.hpp>
#include <phon/utils/signal.hpp>


namespace phonometrica {

class Runtime;
class Object;


class Sound final : public Document
{
public:

	enum class Format
	{
		WAV  = SF_FORMAT_WAV,
		AIFF = SF_FORMAT_AIFF,
		FLAC = SF_FORMAT_FLAC,
		OGG  = SF_FORMAT_OGG
	};

	Sound(Directory *parent, String path);

	static void set_sound_formats();

	static const Array<String> &supported_sound_formats();

	static const Array<String> &common_sound_formats();

	static Array<String> supported_sound_format_names();

	static String rtaudio_version();

	static String libsndfile_version();

	static bool supports_format(const String &format);

	double duration() const;

	int sample_rate() const;

	intptr_t nframes() const;

	double max_value() const;

	double min_value() const;

    const Array<double> &data() const;

    Array<double> &data();

    SndfileHandle handle() const;

    int nchannel() const;

    void convert(const String &path, int sample_rate, Format fmt);

    double get_pitch(double time, double min_pitch, double max_pitch, double threshold);

	double get_intensity(int channel, double time);

	Array<double> get_intensity(int channel, double from, double to, double time_step, bool &start_at_zero);

	Array<double> get_formants(int channel, double time, int nformant, double nyquist_frequency, double window_size, int lpc_order);

	Array<double> get_formants(int channel, const Array<double> &times, int nformant, double nyquist_frequency, double window_size, int lpc_order);

	static void initialize(Runtime &rt);

	intptr_t channel_size() const;

	intptr_t size() const;

	Array<double> get_channel(int n, intptr_t first_sample, intptr_t last_sample) const;

	bool is_mono() const;

	double frame_to_time(intptr_t index) const;

	intptr_t time_to_frame(double time) const;

	double get_sample(int channel, intptr_t index) const;

	constexpr double get_intensity_window_duration() const
	{
		// Praat's settings: use 3.2 pitch periods
		constexpr double min_pitch = 100;
		constexpr double effective_duration = 3.2 / min_pitch;

		return effective_duration;
	}

	int get_intensity_window_size() const;

	static Signal<const String&, const String&, int> start_loading;

	static Signal<int> update_loading;

private:

	void load() override;

	void write() override;

	Array<double> average_channels(intptr_t first_frame = 0, intptr_t last_frame = -1) const;

	std::span<const double> get_channel_view(int n) const;

	static Array<String> the_supported_sound_formats, the_common_sound_formats;

	Array<double> m_data;

	mutable SndfileHandle m_handle;
};

namespace traits {
template<> struct maybe_cyclic<Sound> : std::false_type { };
}
} // namespace phonometrica

#endif // PHONOMETRICA_SOUND_HPP
