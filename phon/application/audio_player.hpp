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
 * Created: 22/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Audio player. This class is responsible for all sound playback in phonometrica. Each sound/annotation      *
 * view has its own player.                                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_AUDIO_PLAYER_HPP
#define PHONOMETRICA_AUDIO_PLAYER_HPP

#include <atomic>
#include <memory>
#include <phon/application/resampler.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>
#include <phon/application/sound.hpp>

namespace phonometrica {

class Runtime;

class AudioPlayer final
{
public:

    explicit AudioPlayer(const Handle<Sound> &data);

    AudioPlayer(const AudioPlayer &) = delete;

    AudioPlayer(AudioPlayer &&) = delete;

    ~AudioPlayer();

    void play(double from, double to);

    bool paused() const;

	bool has_error() const { return m_error != nullptr;  }

	void raise_error();

#if PHON_MACOS
	double *buffer() { return m_buffer.data(); }
#endif

    Resampler *resampler() { return m_resampler.get(); }

//    void current_time(double);
//
//    void error(const std::string &);

    void interrupt();

    void pause();

    void resume();

    void stop();

private:

	void run();

    static int playback(void *output, void *input, unsigned int nframe, double stream_time,
						RtAudioStreamStatus status, void *data);

    void prepare();

    void initialize_resampling(uint32_t output_rate);

    static void play_silence(double *data, size_t size);

    intptr_t remaining_frames() const { return  last_frame - position; }

    static void error_callback(RtAudioError::Type type, const std::string &msg);

    RtAudio::StreamParameters m_params;

    RtAudio::StreamOptions m_options;

    RtAudio m_stream;

    std::unique_ptr<Resampler> m_resampler;

    unsigned int output_rate = 0;

    std::atomic<bool> m_paused = false;

    intptr_t first_frame = -1, last_frame = -1;

    intptr_t position = 0;

	std::exception_ptr m_error;

#if PHON_MACOS

	double * cache() { return m_cache.data(); }

	intptr_t remaining = 0;

	std::vector<double> m_cache; // extra samples are stored here for the next iteration

	intptr_t cached_samples = 0; // number of samples in the cache

    Array<double> m_buffer;
#endif

    Handle<Sound> data;

};

} // namespace phonometrica

#endif // PHONOMETRICA_AUDIO_PLAYER_HPP
