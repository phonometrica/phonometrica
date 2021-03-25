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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
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

AudioPlayer::AudioPlayer(const Handle<Sound> &snd) : m_stream(SOUND_API), data(snd)
{
	PHON_LOG("constructing audio player");
    prepare();
	PHON_LOG("initializing resampling");
    initialize_resampling(output_rate);
	PHON_LOG("resampling initialized");
}

AudioPlayer::~AudioPlayer()
{
    assert(m_error == nullptr);
}

int AudioPlayer::playback(void *out, void *, unsigned int nframe, double, RtAudioStreamStatus status, void *d)
{
    auto player = reinterpret_cast<AudioPlayer*>(d);
    auto output = reinterpret_cast<double*>(out);
    const intptr_t sample_count = nframe * player->data->nchannel();
    play_silence(output, sample_count); // make sure we have a clean output buffer

	if (status) {
		PHON_LOG("Stream underflow detected!");
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
			std::span<double> input(player->buffer(), nframe);

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
    player->current_time(t);

    if (left_over > 0)
    {
		return KEEP_PLAYING;
    }
    else
    {
		player->m_running.store(false, std::memory_order_relaxed);
		player->done();

		return STOP_PLAYING;
	}
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
    m_cache_data.resize(input_size);
#else
    m_params.nChannels = (unsigned int) data->nchannel();
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
	run();
}

void AudioPlayer::initialize_resampling(uint32_t output_rate)
{
    if (m_resampler != nullptr) {
        return;
    }
    auto input_rate = (uint32_t) data->sample_rate();
#if PHON_MACOS
    int size = m_buffer.size();
#else
	int size = FRAME_COUNT;
#endif
    m_resampler = std::make_unique<Resampler>(input_rate, output_rate, size);
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
}

void AudioPlayer::play_silence(double *data, size_t size)
{
    memset(data, 0, size * sizeof(double));
}

void AudioPlayer::error_callback(RtAudioError::Type type, const std::string &msg)
{
//    qDebug() << msg.data();
}

void AudioPlayer::stop()
{
    if (m_stream.isStreamOpen()) {
    	m_stream.stopStream();
    }
	if (m_stream.isStreamOpen()) {
		m_stream.closeStream();
	}
	m_running.store(false, std::memory_order_relaxed);
}

void AudioPlayer::run()
{
	unsigned int frame_count = FRAME_COUNT;

	try
	{
		m_running.store(true, std::memory_order_relaxed);
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
		m_running.store(false, std::memory_order_relaxed);
		m_error = std::current_exception();
	}
}

bool AudioPlayer::running() const
{
	return m_running.load(std::memory_order_relaxed);
}


} // namespace phonometrica