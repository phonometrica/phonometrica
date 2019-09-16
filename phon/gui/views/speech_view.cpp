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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QToolBar>
#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QToolButton>
#include <phon/gui/views/speech_view.hpp>
#include <phon/gui/time_selection_dialog.hpp>
#include <phon/gui/pitch_settings.hpp>
#include <phon/application/settings.hpp>


namespace phonometrica {

SpeechView::SpeechView(Runtime &rt, const std::shared_ptr<AudioData> &data, QWidget *parent) :
    View(parent), m_data(data), player(rt, this, data), rt(rt)
{
    // We can't set up the UI in the constructor because we need to call virtual methods. Instead, we do it
    // in post_initialize(), which is called by the viewer after the view is created and has its vtable set.
}

void SpeechView::post_initialize()
{
    // Create objects first so that we can connect them to signals
    waveform = new Waveform(rt, m_data, this);

    pitch_plot = new PitchPlot(rt, m_data, this);
    pitch_plot->setMaximumHeight(100);

    intensity_plot = new IntensityPlot(rt, m_data, this);
    intensity_plot->setMaximumHeight(100);

    auto zoom = new SoundZoom(this);
    wavebar = new WaveBar(m_data, this);
    waveform->setMagnitude(wavebar->magnitude());

    bool track = Settings::get_boolean(rt, "enable_mouse_tracking");
    waveform->enableMouseTracking(track);
    pitch_plot->enableMouseTracking(track);
    intensity_plot->enableMouseTracking(track);

    // The outer layout hold the Y axis widget and the inner layout which contains plots.
    auto outer_layout = new QHBoxLayout;

    auto inner_layout = new QVBoxLayout;
    auto toolbar = makeToolbar();

    auto hl = new QHBoxLayout;
    label_start = new QLabel;
    label_end = new QLabel;
    hl->addWidget(label_start);
    hl->addStretch();
    hl->addWidget(label_end);
    label_start->setFixedHeight(20);
    label_start->setText(QString::number(0.0, 'f'));
    label_end->setFixedHeight(20);
    label_end->setText(QString::number(m_data->duration(), 'f'));

    inner_layout->addWidget(toolbar);
    inner_layout->addLayout(hl);
    inner_layout->addWidget(waveform);
    inner_layout->addWidget(new SpaceLine);
    inner_layout->addWidget(pitch_plot);
    pitch_line = new SpaceLine;
    inner_layout->addWidget(pitch_line);
    inner_layout->addWidget(intensity_plot);
    intensity_line = new SpaceLine;
    inner_layout->addWidget(intensity_line);
    addAnnotationLayers(inner_layout);
    inner_layout->addWidget(zoom);
    inner_layout->addWidget(wavebar);
    inner_layout->addSpacing(5);
    inner_layout->setSpacing(0);

    y_axis = new YAxisWidget;
    y_axis->addPlot(waveform);
    y_axis->addPlot(pitch_plot);
    y_axis->addPlot(intensity_plot);
    y_axis->setFixedWidth(60);
    outer_layout->addWidget(y_axis);
    outer_layout->addLayout(inner_layout);
    outer_layout->addSpacing(10);
    outer_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(outer_layout);

    plots.append(waveform);
    plots.append(pitch_plot);
    plots.append(intensity_plot);

    // All updates of the scrollbar update the zoomer, but only updates created by the user also update the plots
    connect(wavebar, &WaveBar::updatedXAxisSelection, zoom, &SoundZoom::setXAxisSelection);
    connect(wavebar, &WaveBar::selectionStarted, zoom, &SoundZoom::hideSelection);
    connect(wavebar, &WaveBar::timeSelection, waveform, &Waveform::setWindow);
    connect(wavebar, &WaveBar::timeSelection, pitch_plot, &PitchPlot::setWindow);
    connect(wavebar, &WaveBar::timeSelection, intensity_plot, &PitchPlot::setWindow);
    connect(waveform, &Waveform::timeSelection, wavebar, &WaveBar::setTimeSelection);
    connect(pitch_plot, &PitchPlot::timeSelection, wavebar, &WaveBar::setTimeSelection);
    connect(intensity_plot, &IntensityPlot::timeSelection, wavebar, &WaveBar::setTimeSelection);

    connect(pitch_plot, &PitchPlot::yAxisModified, y_axis, &YAxisWidget::refresh);

    for (auto plot1 : plots)
    {
        for (auto plot2 : plots)
        {
            if (plot1 != plot2)
            {
                connect(plot1, &SpeechPlot::currentTime, plot2, &SpeechPlot::setCurrentTime);
                connect(plot1, &SpeechPlot::timeSelection, plot2, &SpeechPlot::setSelection);
                connect(plot1, &SpeechPlot::selectionStarted, plot2, &SpeechPlot::invalidateCurrentTime);
            }
        }
    }


//    connect(waveform, &Waveform::windowHasChanged, this, &SpeechView::setWindowTimes);
//    connect(waveform, &Waveform::windowHasChanged, pitch_plot, &PitchPlot::setWindow);

//    connect(waveform, &Waveform::windowHasChanged, pitch_plot, &PitchPlot::setWindow);
//    connect(pitch_plot, &PitchPlot::windowHasChanged, waveform, &Waveform::setWindow);

    connect(&player, &AudioPlayer::current_time, waveform, &Waveform::updateTick);
    connect(&player, &AudioPlayer::finished, waveform, &Waveform::hideTick);
    connect(&player, &AudioPlayer::done, this, &SpeechView::onPlayingFinished);

    setInitialWindow();
}

QToolBar *SpeechView::makeToolbar()
{
    auto pitch_menu = new QMenu;
    auto action_enable_pitch = new QAction(tr("Show pitch"), this);
    action_enable_pitch->setCheckable(true);
    action_enable_pitch->setChecked(true);
    auto action_pitch_settings = new QAction(tr("Change pitch settings..."), this);
    pitch_menu->addAction(action_enable_pitch);
    pitch_menu->addAction(action_pitch_settings);

    auto intensity_menu = new QMenu;
    auto action_enable_intensity = new QAction(tr("Show intensity"), this);
    action_enable_intensity->setCheckable(true);
    action_enable_intensity->setChecked(true);
    intensity_menu->addAction(action_enable_intensity);

    auto options_menu = new QMenu;

    auto action_enable_tracking = new QAction(tr("Enable mouse tracking"), this);
    action_enable_tracking->setCheckable(true);
    action_enable_tracking->setChecked(Settings::get_boolean(rt, "enable_mouse_tracking"));
    options_menu->addAction(action_enable_tracking);

    //    auto action_spectrogram = new QAction(tr("Spectrogram"), this);
//    action_spectrogram->setCheckable(true);
//    auto action_pitch = new QAction(tr("Pitch"), this);
//    action_pitch->setCheckable(true);


//    wave_menu->addAction(action_spectrogram);
//    wave_menu->addAction(action_pitch);
//    action_spectrogram->setEnabled(false);
//    action_pitch->setEnabled(false);

    auto toolbar = new QToolBar(this);
    play_action = new QAction(QIcon(":/icons/play.png"), "Play");
    auto stop = new QAction(QIcon(":/icons/stop.png"), "Stop");
    auto zoom_in = new QAction(QIcon(":/icons/zoom+.png"), "Zoom in");
    auto zoom_out = new QAction(QIcon(":/icons/zoom-.png"), "Zoom out");
    auto move_next = new QAction(QIcon(":/icons/next.png"), "Move forward");
    auto move_back = new QAction(QIcon(":/icons/back.png"), "Move backward");
    auto zoom_sel = new QAction(QIcon(":/icons/collapse.png"), "Zoom to selection");
    auto view_all = new QAction(QIcon(":/icons/expand.png"), "View whole file");
    auto select = new QAction(QIcon(":/icons/selection.png"), "Select window...");
    auto doc = new QAction(QIcon(":/icons/question.png"), "Documentation");
    auto options_button = new QToolButton;
    options_button->setIcon(QIcon(":/icons/settings.png"));
    options_button->setToolTip(tr("Waveform options"));
    options_button->setMenu(options_menu);
    options_button->setPopupMode(QToolButton::InstantPopup);

    auto wave_button = new QToolButton;
    wave_button->setIcon(QIcon(":/icons/waveform.png"));
    wave_button->setToolTip("Waveform options");

    auto spectrum_button = new QToolButton;
    spectrum_button->setIcon(QIcon(":/icons/spectrum.png"));
    spectrum_button->setToolTip("Spectrogram options");

    auto pitch_button = new QToolButton;
    pitch_button->setIcon(QIcon(":/icons/voice.png"));
    pitch_button->setToolTip("Pitch options");
    pitch_button->setMenu(pitch_menu);
    pitch_button->setPopupMode(QToolButton::InstantPopup);

    auto intensity_button = new QToolButton;
    intensity_button->setIcon(QIcon(":/icons/hearing.png"));
    intensity_button->setToolTip("Intensity options");
    intensity_button->setMenu(intensity_menu);
    intensity_button->setPopupMode(QToolButton::InstantPopup);

    toolbar->addAction(play_action);
    toolbar->addAction(stop);
    toolbar->addSeparator();
    addEditButtons(toolbar);
    toolbar->addAction(move_back);
    toolbar->addAction(move_next);
    toolbar->addAction(zoom_out);
    toolbar->addAction(zoom_in);
    toolbar->addAction(zoom_sel);
    toolbar->addAction(view_all);
    toolbar->addAction(select);
    toolbar->addSeparator();
    addAnnotationMenu(toolbar);
    toolbar->addWidget(wave_button);
    toolbar->addWidget(spectrum_button);
    toolbar->addWidget(pitch_button);
    toolbar->addWidget(intensity_button);
    toolbar->addSeparator();
    toolbar->addWidget(options_button);

    auto spacing = new QWidget;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacing);
    toolbar->addAction(doc);

    connect(play_action, &QAction::triggered, this, &SpeechView::play);
    connect(action_enable_tracking, &QAction::triggered, this, &SpeechView::enableMouseTracking);
    connect(stop, &QAction::triggered, this, &SpeechView::stop);
    connect(zoom_in, &QAction::triggered, this, &SpeechView::zoomIn);
    connect(zoom_out, &QAction::triggered, this, &SpeechView::zoomOut);
    connect(zoom_sel, &QAction::triggered, this, &SpeechView::zoomToSelection);
    connect(view_all, &QAction::triggered, this, &SpeechView::viewAll);
    connect(move_next, &QAction::triggered, this, &SpeechView::moveForward);
    connect(move_back, &QAction::triggered, this, &SpeechView::moveBackward);
    connect(select, &QAction::triggered, this, &SpeechView::chooseSelection);
    connect(action_enable_pitch, &QAction::triggered, this, &SpeechView::showPitch);
    connect(action_pitch_settings, &QAction::triggered, this, &SpeechView::changePitchSettings);
    connect(action_enable_intensity, &QAction::triggered, this, &SpeechView::showIntensity);
    connect(waveform, &SpeechPlot::windowHasChanged, this, &SpeechView::setWindowTimes);

#if PHON_MACOS
    toolbar->setMaximumHeight(30);
    toolbar->setStyleSheet("QToolBar{spacing:0px;}");
#endif

    return toolbar;
}

void SpeechView::zoomIn(bool)
{
    waveform->zoomIn();
    pitch_plot->zoomIn();
    intensity_plot->zoomIn();
}

void SpeechView::zoomOut(bool)
{
    waveform->zoomOut();
    pitch_plot->zoomOut();
    intensity_plot->zoomOut();
}

void SpeechView::zoomToSelection(bool)
{
    waveform->zoomToSelection();
    pitch_plot->zoomToSelection();
    intensity_plot->zoomToSelection();
}

void SpeechView::viewAll(bool)
{
    waveform->viewAll();
    pitch_plot->viewAll();
    intensity_plot->viewAll();
}

void SpeechView::moveForward(bool)
{
    waveform->moveForward();
    pitch_plot->moveForward();
    intensity_plot->moveForward();
}

void SpeechView::moveBackward(bool)
{
    waveform->moveBackward();
    pitch_plot->moveBackward();
    intensity_plot->moveBackward();
}

void SpeechView::chooseSelection(bool)
{
    double from = -1;
    double to = -1;
    TimeSelectionDialog dlg(this, &from, &to);
    dlg.exec();

    if (from >= 0)
    {
        if (to > 0 && to > m_data->duration())
        {
            QMessageBox msg(QMessageBox::Critical, tr("Error"), tr("Invalid time window"));
            msg.exec();
        }
        else
        {
            setSelection(from, to);
        }
    }
}

void SpeechView::setSelection(double t1, double t2)
{
    wavebar->setTimeSelection(t1, t2);
    waveform->setWindow(t1, t2);
    pitch_plot->setWindow(t1, t2);
    intensity_plot->setWindow(t1, t2);
    // TODO: set window for other plots here
}

void SpeechView::play(bool)
{
    if (player.isRunning())
    {
        if (player.paused())
        {
            setPauseIcon();
            player.resume();
        }
        else
        {
            setPlayIcon();
            player.pause();
        }
    }
    else
    {
        auto times = waveform->times();
        setPauseIcon();
		try {
			player.play(times.first, times.second);
			if (player.has_error())
			{
				player.raise_error();
			}
		}
		catch (std::exception& e)
		{
			auto msg = utils::format("Cannot play sound: %", e.what());
			QMessageBox dlg(QMessageBox::Critical, tr("Error"), QString::fromStdString(msg));
			dlg.exec();
		}
    }
}

void SpeechView::stop(bool)
{
    player.interrupt();
    setPlayIcon();
    waveform->hideTick();
}

void SpeechView::setWindowTimes(double t1, double t2)
{
    label_start->setText(QString::number(t1, 'f'));
    label_end->setText(QString::number(t2, 'f'));
}

void SpeechView::onPlayingFinished()
{
    play_action->setIcon(QIcon(":/icons/play.png"));
}

void SpeechView::setPlayIcon()
{
    play_action->setIcon(QIcon(":/icons/play.png"));
}

void SpeechView::setPauseIcon()
{
    play_action->setIcon(QIcon(":/icons/pause.png"));
}

void SpeechView::enableMouseTracking(bool enable)
{
    Settings::set_value(rt, "enable_mouse_tracking", enable);
    waveform->enableMouseTracking(enable);
    pitch_plot->enableMouseTracking(enable);
    intensity_plot->enableMouseTracking(enable);
    waveform->repaint();
    pitch_plot->repaint();
    intensity_plot->repaint();
}

void SpeechView::showPitch(bool checked)
{
    pitch_plot->setVisible(checked);
    pitch_line->setVisible(checked);
}

void SpeechView::changePitchSettings(bool)
{
    PitchSettings dlg(rt, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        pitch_plot->updateSettings();
    }
}

void SpeechView::showIntensity(bool checked)
{
    intensity_plot->setVisible(checked);
    intensity_line->setVisible(checked);
}

void SpeechView::setInitialWindow()
{
    waveform->setInitialWindow();
    pitch_plot->setInitialWindow();
    intensity_plot->setInitialWindow();
    wavebar->setInitialSelection();
    // Let the layers know about the size of the window
    waveform->informWindow();
}

} // namespace phonometrica