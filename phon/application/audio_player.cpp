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
	PHON_LOG("constructing audio player");
    connect(this, &QThread::finished, this, &AudioPlayer::stop);
    prepare();
	PHON_LOG("initializing resampling");
    initialize_resampling(output_rate);
	PHON_LOG("resampling initialized");
}

AudioPlayer::~AudioPlayer()
{
    assert(m_error == nullptr);
    if (isRunning()) {
		interrupt();
    }
}

int AudioPlayer::playback(void *out, void *, unsigned int nframe, double, RtAudioStreamStatus status, void *d)
{
    auto player = reinterpret_cast<AudioPlayer*>(d);
    auto output = reinterpret_cast<double*>(out);
    const intptr_t sample_count = nframe * player->data->channels();
    play_silence(output, sample_count); // make sure we have a clean output buffer

	if (status) {
		qDebug("Stream underflow detected!");
	}
    if (player->paused()) {
        return KEEP_PLAYING;
    }

    auto handle = player->data->handle();
    // seek to the current file position
    handle.seek(player->position, SEEK_SET);

    // how far are we from the end?
    sf_count_t left_over = player->remaining_frames();
    sf_count_t len = (nframe > left_over) ? left_over : nframe;

#if PHON_MACOS
    // The resampler doesn't provide us with a predictable number of samples, whereas RTAudio expects a fixed-size
    // buffer. We perform several calls to the resampler until we have a full buffer and cache extra samples for the
    // next iteration
    auto resampler = player->resampler();
	double *end = output + nframe; // end of first channel
	double *it = output;

	while (it < end)
	{
		// First, flush the cache and write it to the output buffer. If we have more samples in the cache than fits
		// the output buffer, save the extra samples for the next iteration and return.
		intptr_t extra = it + player->cached_samples - end;
		if (extra >= 0)
		{
			intptr_t consumed = player->cached_samples - extra;
			memcpy(it, player->cache(), consumed * sizeof(double));
			memmove(player->cache(), player->cache() + consumed, extra * sizeof(double));
			player->cached_samples = extra;
			break;
		}
		else
		{
			memcpy(it, player->cache(), player->cached_samples * sizeof(double));
			it += player->cached_samples;
			player->cached_samples = 0;
		}

		// Do we still have samples to write? (Note that it's possible, and in fact likely, that we'll still have
		// samples to write even though we have finished reading, because of the resampler's latency.)
		if (player->remaining > 0)
		{
			double *buffer = player->buffer();

			// If we have finished reading, fill the buffer with silence.
			if (len == 0)
			{
				play_silence(buffer, nframe);
			}
			else
			{
				intptr_t count = handle.readf(buffer, len);
				player->position += count;
				for (intptr_t i = count; i < nframe; i++) buffer[i] = 0.0;
			}
			Span<double> input(player->buffer(), nframe);

			double *resampled_buffer = nullptr;
			auto written = resampler->process(input.data(), input.size(), resampled_buffer);
			if (written > player->remaining) written = player->remaining;
			// We now try to fill the output; left over samples are cached.
			auto buffer_end = (std::min)(it + written, end);
			intptr_t consumed = 0;
			while (it != buffer_end)
			{
				*it++ = resampled_buffer[consumed++];
			}
			player->cached_samples = written - consumed;
			memcpy(player->cache(), resampled_buffer + consumed, player->cached_samples * sizeof(double));
			player->remaining -= written;
		}
		else
		{
			auto size = size_t(nframe * player->data->channels());
			play_silence(output, size);
		}
	}

	// Duplicate first channel.
	memcpy(output + nframe, output, sizeof(double) * nframe);
#else
    player->position += handle.readf(output, len);
#endif

    auto t = double(player->position) / handle.samplerate();
    emit player->current_time(t);

    if (left_over > 0) {
		return KEEP_PLAYING;
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
    remaining = (data->size() / data->channels()) * double(output_rate) / data->sample_rate();
    auto input_size = floor(FRAME_COUNT * double(output_rate) / data->sample_rate());
    m_buffer.resize(input_size);
    m_cache.resize(input_size);
#else
    input_size = FRAME_COUNT;
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
			to = duration;
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
			RTAUDIO_FLOAT64,
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
    auto input_rate = (uint32_t) data->sample_rate();
    m_resampler = std::make_unique<Resampler>(input_rate, output_rate, m_buffer.size());
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

void AudioPlayer::play_silence(double *data, size_t size)
{
    memset(data, 0, size * sizeof(double));
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