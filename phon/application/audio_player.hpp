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
 * Purpose: Audio player. This class is responsible for all sound playback in phonometrica. Each sound/annotation      *
 * view has its own player.                                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_AUDIO_PLAYER_HPP
#define PHONOMETRICA_AUDIO_PLAYER_HPP

#include <atomic>
#include <memory>
#include <QThread>
#include <phon/application/audio_data.hpp>
#include <phon/application/resampler.hpp>
#include <phon/third_party/rtaudio/RtAudio.h>

namespace phonometrica {

class Runtime;

class AudioPlayer final : public QThread
{
    Q_OBJECT

public:

    AudioPlayer(Runtime &rt, QObject *parent, std::shared_ptr<AudioData> data);

    AudioPlayer(const AudioPlayer &) = delete;

    AudioPlayer(AudioPlayer &&) = delete;

    ~AudioPlayer() override;

    void play(double from, double to);

    void run() override;

    bool paused() const;

	bool has_error() const { return m_error != nullptr;  }

	void raise_error();

#if PHON_MACOS
	double *buffer() { return m_buffer.data(); }
#endif

    Resampler *resampler() { return m_resampler.get(); }

signals:

    void current_time(double);

    void done();

    void error(const std::string &);

public slots:

    void interrupt();

    void pause();

    void resume();

    void stop();

private:

    static int playback(void *output, void *input, unsigned int nframe, double stream_time,
						RtAudioStreamStatus status, void *data);

    void prepare();

    void initialize_resampling(uint32_t output_rate);

    static void play_silence(double *data, size_t size);

    intptr_t remaining_frames() const { return  last_frame - position; }

    static void error_callback(RtAudioError::Type type, const std::string &msg);

    Runtime &rt;

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

    // Current audio data being played, if any
    std::shared_ptr<AudioData> data;

};

} // namespace phonometrica

#endif // PHONOMETRICA_AUDIO_PLAYER_HPP
