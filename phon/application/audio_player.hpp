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
 * Created: 22/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Audio player. This class is responsible for all sound playback in phonometrica. Each sound/annotation     *
 * view has its own player.                                                                                           *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_AUDIO_PLAYER_HPP
#define PHONOMETRICA_AUDIO_PLAYER_HPP

#include <atomic>
#include <memory>
#include <QThread>
#include <phon/application/audio_data.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>
#include <phon/third_party/speex/speex_resampler.h>

namespace phonometrica {

class Environment;

class AudioPlayer final : public QThread
{
    Q_OBJECT

public:

    AudioPlayer(Environment &env, QObject *parent, std::shared_ptr<AudioData> data);

    AudioPlayer(const AudioPlayer &) = delete;

    AudioPlayer(AudioPlayer &&) = delete;

    ~AudioPlayer() override;

    void play(double from, double to);

    void run() override;

    bool paused() const;

signals:

    void current_time(double);

    void done();

    void error(const std::string &);

public slots:

    void interrupt();

    void pause();

    void resume();

    void close();

private:

    static int playback(void *output, void *input, unsigned int nframes, double stream_time,
                        RtAudioStreamStatus status, void *data);

    void prepare();

    void initialize_resampling(uint32_t output_rate);

    static void play_silence(float *data, size_t size);

    intptr_t remaining_frames() const { return  last_frame - position; }

    static void error_callback(RtAudioError::Type type, const std::string &msg);

    Environment &env;

    RtAudio::StreamParameters m_params;

    RtAudio::StreamOptions m_options;

    RtAudio m_stream;

    SpeexResamplerState *resampler = nullptr;

    unsigned int output_rate;

    std::atomic<bool> m_paused = false;

    intptr_t first_frame = -1, last_frame = -1;

    intptr_t position = 0;

    // Current audio data being played, if any
    std::shared_ptr<AudioData> data;

};

} // namespace phonometrica

#endif // PHONOMETRICA_AUDIO_PLAYER_HPP
