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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QDebug>
#include <phon/application/audio_player.hpp>
#include <phon/application/settings.hpp>


#if PHON_WINDOWS
#define SOUND_API RtAudio::WINDOWS_DS
#elif PHON_MACOS
#define SOUND_API RtAudio::MACOSX_CORE
#define MAC_SAMPLE_RATE 44100.0
#else
#define SOUND_API RtAudio::LINUX_ALSA
#endif

#define MONO_CHANNEL 0
#define KEEP_PLAYING 0
#define STOP_PLAYING 1

namespace phonometrica {

AudioPlayer::AudioPlayer(Environment &env, QObject *parent, std::shared_ptr<AudioData> data) :
    QThread(parent), env(env), m_stream(SOUND_API), data(std::move(data))
{
    connect(this, &QThread::finished, this, &AudioPlayer::close);
    prepare();
    initialize_resampling(output_rate);
}

AudioPlayer::~AudioPlayer()
{
    if (isRunning()) {
        close();
    }
    if (resampler) {
        speex_resampler_destroy(resampler);
    }
}

int AudioPlayer::playback(void *out, void *, unsigned int nframes, double,
                          RtAudioStreamStatus status, void *d)
{
    auto player = reinterpret_cast<AudioPlayer*>(d);
    auto output = reinterpret_cast<float*>(out);

    if (player->paused())
    {
        auto size = size_t(sizeof(float) * nframes * player->data->channels());
        play_silence(output, size);

        return KEEP_PLAYING;
    }

    if (status)
        qDebug("Stream underflow detected!");

    auto handle = player->data->handle();
    // seek to the current file position
    handle.seek(player->position, SEEK_SET);


    // how far are we from the end?
    sf_count_t left_over = player->remaining_frames();
    sf_count_t len = (nframes > left_over) ? left_over : nframes;

    player->position += handle.readf(output, len);

    auto t = double(player->position) / handle.samplerate();
    emit player->current_time(t);

    if (left_over > 0)
        return KEEP_PLAYING;
    else
    {
        // zero out buffer to avoid playing it twice
        memset(output, 0, sizeof(float) * nframes);
    }

    emit player->done();
    return STOP_PLAYING;
}

void AudioPlayer::prepare()
{
    m_params.deviceId = m_stream.getDefaultOutputDevice();

#if PHON_MACOS
    m_params.nChannels = 2;
    output_rate = MAC_SAMPLE_RATE;
#else
    m_params.nChannels = (unsigned int) data->channels();
    output_rate = (unsigned int) data->sample_rate();
#endif
    m_params.firstChannel = 0;

#if PHON_LINUX
    m_options.flags = RTAUDIO_ALSA_USE_DEFAULT;
#elif PHON_MACOS
    if (m_data->channels() == 1)
        m_options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
	else
		m_options.flags = 0;
#else
    m_options.flags = RTAUDIO_SCHEDULE_REALTIME;
#endif
}

void AudioPlayer::play(double from, double to)
{
    first_frame = data->time_to_frame(from);
    last_frame = data->time_to_frame(to);
    position = first_frame;
    start();
}

void AudioPlayer::run()
{
    unsigned int frame_count = 1024; //4096;

    m_stream.openStream(&m_params,
                         nullptr,
                         RTAUDIO_FLOAT32,
                         output_rate,
                         &frame_count,
                         &AudioPlayer::playback,
                         this,
                         &m_options,
                         error_callback);

    m_stream.startStream();

    while (m_stream.isStreamRunning())
        QThread::msleep(2); // wait for 2 ms
}

void AudioPlayer::initialize_resampling(uint32_t output_rate)
{
    if (resampler != nullptr) {
        return;
    }

    int error = 0;
    auto input_rate = (uint32_t) data->sample_rate();
    auto quality = int(Settings::get_number(env, "resampling_quality"));
    resampler = speex_resampler_init(data->channels(), input_rate, output_rate, quality, &error);

    if (error != 0 && resampler)
    {
        speex_resampler_destroy(resampler);
        resampler = nullptr;
    }
}

bool AudioPlayer::paused() const
{
    return m_paused.load(std::memory_order_relaxed);
}

void AudioPlayer::pause()
{
    m_paused.store(true, std::memory_order_relaxed);
}

void AudioPlayer::resume()
{
    m_paused.store(false, std::memory_order_relaxed);
}

void AudioPlayer::interrupt()
{
    close();
    quit();
}

void AudioPlayer::play_silence(float *data, size_t size)
{
    memset(data, 0, size);
}

void AudioPlayer::error_callback(RtAudioError::Type type, const std::string &msg)
{
    qDebug() << msg.data();
}

void AudioPlayer::close()
{
    if (m_stream.isStreamOpen()) {
        m_stream.closeStream();
    }
}


} // namespace phonometrica