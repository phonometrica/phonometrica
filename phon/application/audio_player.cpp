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
 * Created: 22/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
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

#define FRAME_COUNT 1024

#define MINIMUM_DURATION 1.0

namespace phonometrica {

AudioPlayer::AudioPlayer(Runtime &rt, QObject *parent, std::shared_ptr<AudioData> data) :
    QThread(parent), rt(rt), m_stream(SOUND_API), data(std::move(data))
{
    connect(this, &QThread::finished, this, &AudioPlayer::stop);
    prepare();
    initialize_resampling(output_rate);
}

AudioPlayer::~AudioPlayer()
{
    assert(m_error == nullptr);

    if (isRunning()) {
	    stop();
    }
    if (m_resampler) {
        speex_resampler_destroy(m_resampler);
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

#if PHON_MACOS
    // TO BE CONTINUED...
    sf_count_t size_in_sf = FRAME_COUNT / player->ratio();
    // Read data with libsndfile
    sf_count_t nread_in = handle.readf(player->buffer(), size_in_sf);
    player->position += nread_in;
    // Set size of the output buffer
    spx_uint32_t nread_out = nread_in * player->ratio();

    spx_uint32_t nread_in_spx = (spx_uint32_t)nread_in;
    int error = speex_resampler_process_float(player->resampler(), MONO_CHANNEL, player->buffer(), &nread_in_spx, output, &nread_out);
#else
    player->position += handle.readf(output, len);
#endif
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
    m_ratio = double(output_rate) / data->sample_rate();
    m_buffer.resize(FRAME_COUNT);
#else
    m_params.nChannels = (unsigned int) data->channels();
    output_rate = (unsigned int) data->sample_rate();
#endif
    m_params.firstChannel = 0;

#if PHON_LINUX
    m_options.flags = RTAUDIO_ALSA_USE_DEFAULT;
#elif PHON_MACOS
    if (data->channels() == 1)
        m_options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_SCHEDULE_REALTIME;
	else
		m_options.flags = 0;
#else
    m_options.flags = RTAUDIO_SCHEDULE_REALTIME;
#endif
}

void AudioPlayer::play(double from, double to)
{
	if (from == to)
	{
		double duration = data->duration();

		if (duration < MINIMUM_DURATION)
		{
			from = 0.0;
			to = MINIMUM_DURATION;
		}
		else
		{
			from -= MINIMUM_DURATION / 2;
			to += MINIMUM_DURATION / 2;
		}
	}
    first_frame = data->time_to_frame(from);
    last_frame = data->time_to_frame(to);
    position = first_frame;
    start();
}

void AudioPlayer::run()
{
    unsigned int frame_count = FRAME_COUNT;

	try
	{
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
	}
	catch (...)
	{
		m_error = std::current_exception();
		this->quit();		
	}

    while (m_stream.isStreamRunning())
        QThread::msleep(2); // wait for 2 ms
}

void AudioPlayer::initialize_resampling(uint32_t output_rate)
{
    if (m_resampler != nullptr) {
        return;
    }

    int error = 0;
    auto input_rate = (uint32_t) data->sample_rate();
    auto quality = int(Settings::get_number(rt, "resampling_quality"));
    m_resampler = speex_resampler_init(data->channels(), input_rate, output_rate, quality, &error);

    if (error != 0 && m_resampler)
    {
        speex_resampler_destroy(m_resampler);
        m_resampler = nullptr;
    }
}

bool AudioPlayer::paused() const
{
    return m_paused.load(std::memory_order_relaxed);
}

void AudioPlayer::raise_error()
{
	auto e = std::move(m_error);
	m_error = nullptr;
	std::rethrow_exception(std::move(e));
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
	stop();
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

void AudioPlayer::stop()
{
    if (m_stream.isStreamOpen()) {
        m_stream.closeStream();
    }
}


} // namespace phonometrica