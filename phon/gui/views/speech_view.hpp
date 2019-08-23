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
 * Created: 20/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: base class for sound views and annotation views. They are essentially the same thing, except for the fact *
 * that annotation views can display annotation layers in addition to sound plots.                                    *
 *                                                                                                                    *
 **********************************************************************************************************************/

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
#include <phon/runtime/environment.hpp>

class QToolBar;

namespace phonometrica {

class SpeechView : public View
{
    Q_OBJECT

public:

    SpeechView(Environment &env, const std::shared_ptr<AudioData> &data, QWidget *parent = nullptr);

    void post_initialize() override;

protected slots:

    void zoomIn(bool);

    void zoomOut(bool);

    void zoomToSelection(bool);

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

    void setSelection(double t1, double t2);

    QToolBar *makeToolbar();

    void setPlayIcon();

    void setPauseIcon();

    virtual void addAnnotationMenu(QToolBar *) { }

    virtual void addAnnotationLayers(QVBoxLayout *) { }

    void setInitialWindow();

    Environment &env;

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
