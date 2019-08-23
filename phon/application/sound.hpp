/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Sound file.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_HPP
#define PHONOMETRICA_SOUND_HPP

#include <phon/application/vfs.hpp>
#include <phon/application/audio_data.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>

namespace phonometrica {

class Sound final : public VFile
{
public:

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

    SndfileHandle handle() const;

    int nchannel() const;

private:

	void load() override;

	void write() override;

	static Array<String> the_supported_sound_formats, the_common_sound_formats;

	std::shared_ptr<AudioData> m_data;

	SndfileHandle light_handle() const;

	//SndfileHandle m_handle;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SOUND_HPP
