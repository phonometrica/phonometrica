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
 * Purpose: Sound file.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_HPP
#define PHONOMETRICA_SOUND_HPP

#include <phon/application/vfs.hpp>
#include <phon/application/audio_data.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>

namespace phonometrica {

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

    void resample(const String &path, int sample_rate, Format fmt);

private:

	void load() override;

	void write() override;

	static Array<String> the_supported_sound_formats, the_common_sound_formats;

	std::shared_ptr<AudioData> m_data;
};

using AutoSound = std::shared_ptr<Sound>;

} // namespace phonometrica

#endif // PHONOMETRICA_SOUND_HPP
