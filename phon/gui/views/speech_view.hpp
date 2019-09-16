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
 * Created: 20/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: base class for sound views and annotation views. They are essentially the same thing, except for the fact  *
 * that annotation views can display annotation layers in addition to sound plots.                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPEECH_VIEW_HPP
#define PHONOMETRICA_SPEECH_VIEW_HPP

#include <QLabel>
#include <QAction>
#include <QVBoxLayout>
#include <phon/gui/views/view.hpp>
#include <phon/gui/wave_bar.hpp>
#include <phon/gui/sound_zoom.hpp>
#include <phon/gui/waveform.hpp>
#include <phon/gui/intensity_plot.hpp>
#include <phon/gui/pitch_plot.hpp>
#include <phon/gui/space_line.hpp>
#include <phon/gui/y_axis_widget.hpp>
#include <phon/application/audio_player.hpp>
#include <phon/application/audio_data.hpp>
#include <phon/runtime/runtime.hpp>

class QToolBar;

namespace phonometrica {

class SpeechView : public View
{
    Q_OBJECT

public:

    SpeechView(Runtime &rt, const std::shared_ptr<AudioData> &data, QWidget *parent = nullptr);

    void post_initialize() override;

	void setSelection(double t1, double t2);

protected slots:

	void zoomToSelection(bool);

    void zoomIn(bool);

    void zoomOut(bool);

    void viewAll(bool);

    void moveForward(bool);

    void moveBackward(bool);

    void chooseSelection(bool);

    void play(bool);

    void stop(bool);

    void setWindowTimes(double, double);

    void onPlayingFinished();

    void enableMouseTracking(bool enable);

    void showPitch(bool);

    void showIntensity(bool);

    void changePitchSettings(bool);

protected:

    QToolBar *makeToolbar();

    void setPlayIcon();

    void setPauseIcon();

    virtual void addEditButtons(QToolBar *) { }

    virtual void addAnnotationMenu(QToolBar *) { }

    virtual void addAnnotationLayers(QVBoxLayout *) { }

    void setInitialWindow();

    Runtime &rt;

    std::shared_ptr<AudioData> m_data;

    AudioPlayer player;

    Waveform *waveform;

    PitchPlot *pitch_plot;

    IntensityPlot *intensity_plot;

    Array<SpeechPlot*> plots; // keep plots together to make it easier to connect them

    SpaceLine *pitch_line, *intensity_line;

    YAxisWidget *y_axis;

    WaveBar *wavebar;

    QLabel *label_start, *label_end; // display start and end of sound file

    QAction *play_action;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SPEECH_VIEW_HPP
