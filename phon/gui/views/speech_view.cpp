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

#include <QMenu>
#include <QDebug>
#include <QMessageBox>
#include <QToolButton>
#include <phon/gui/toolbar.hpp>
#include <phon/gui/views/speech_view.hpp>
#include <phon/gui/time_selection_dialog.hpp>
#include <phon/gui/waveform_settings.hpp>
#include <phon/gui/pitch_settings.hpp>
#include <phon/gui/intensity_settings.hpp>
#include <phon/gui/spectrogram_settings.hpp>
#include <phon/gui/formant_settings.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

SpeechView::SpeechView(Runtime &rt, const std::shared_ptr<AudioData> &data, QWidget *parent) :
		View(parent), m_data(data), player(rt, this, data), runtime(rt)
{
	PHON_LOG("Constructing speech annotation");
    // We can't set up the UI in the constructor because we need to call virtual methods. Instead, we do it
    // in post_initialize(), which is called by the viewer after the view is created and has its vtable set.
}

void SpeechView::post_initialize()
{
	PHON_LOG("post-initializing speech view");
	setupUi();
	setInitialWindow();
	setPlotVisibility();
	PHON_LOG("Speech view post-initialization done");
}

Toolbar *SpeechView::makeToolbar()
{
	PHON_LOG("Adding toolbar to speech view");

	auto wave_menu = new QMenu;
	auto action_wave_settings = new QAction(tr("Waveform settings..."), this);
	wave_menu->addAction(action_wave_settings);

	auto spectrum_menu = new QMenu;
	action_enable_spectrum = new QAction(tr("Show spectrogram"), this);
	action_enable_spectrum->setCheckable(true);
	action_enable_spectrum->setChecked(true);
	auto action_spectrum_settings = new QAction(tr("Spectrogram settings..."), this);
	spectrum_menu->addAction(action_enable_spectrum);
	spectrum_menu->addAction(action_spectrum_settings);
	
	auto formants_menu = new QMenu;
	action_enable_formants = new QAction(tr("Show formants"), this);
	action_enable_formants->setCheckable(true);
	auto action_formants_settings = new QAction(tr("Formant settings..."), this);
	formants_menu->addAction(action_enable_formants);
	formants_menu->addAction(action_formants_settings);

    auto pitch_menu = new QMenu;
    action_enable_pitch = new QAction(tr("Show pitch"), this);
    action_enable_pitch->setCheckable(true);
    action_enable_pitch->setChecked(true);
    auto action_pitch_settings = new QAction(tr("Pitch settings..."), this);
    pitch_menu->addAction(action_enable_pitch);
    pitch_menu->addAction(action_pitch_settings);

    auto intensity_menu = new QMenu;
    action_enable_intensity = new QAction(tr("Show intensity"), this);
    action_enable_intensity->setCheckable(true);
    action_enable_intensity->setChecked(true);
    auto action_intensity_settings = new QAction(tr("Intensity settings..."), this);
    intensity_menu->addAction(action_enable_intensity);
    intensity_menu->addAction(action_intensity_settings);

    auto options_menu = new QMenu;

    auto action_enable_tracking = new QAction(tr("Enable mouse tracking"), this);
    action_enable_tracking->setCheckable(true);
    action_enable_tracking->setChecked(Settings::get_boolean(runtime, "enable_mouse_tracking"));
    options_menu->addAction(action_enable_tracking);

    auto toolbar = new Toolbar(this);
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
    wave_button->setMenu(wave_menu);
    wave_button->setPopupMode(QToolButton::InstantPopup);

    auto spectrum_button = new QToolButton;
    spectrum_button->setIcon(QIcon(":/icons/spectrum.png"));
    spectrum_button->setToolTip("Spectrogram options");
    spectrum_button->setMenu(spectrum_menu);
    spectrum_button->setPopupMode(QToolButton::InstantPopup);
    
    auto formants_button = new QToolButton;
    formants_button->setIcon(QIcon(":/icons/formants.png"));
    formants_button->setToolTip("Formants options");
    formants_button->setMenu(formants_menu);
    formants_button->setPopupMode(QToolButton::InstantPopup);

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
    toolbar->addWidget(formants_button);
    toolbar->addWidget(pitch_button);
    toolbar->addWidget(intensity_button);
    toolbar->addSeparator();
    toolbar->addWidget(options_button);

#if PHON_MACOS || PHON_WINDOWS
	toolbar->setMaximumHeight(30);
#endif

#if PHON_MACOS
    toolbar->addStretch();
#else
    auto spacing = new QWidget;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacing);
#endif
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
    connect(action_enable_pitch, &QAction::toggled, this, &SpeechView::showPitch);
    connect(action_enable_formants, &QAction::toggled, this, &SpeechView::showFormants);
	connect(action_enable_spectrum, &QAction::toggled, this, &SpeechView::showSpectrogram);
	connect(action_enable_intensity, &QAction::toggled, this, &SpeechView::showIntensity);
	connect(action_wave_settings, &QAction::triggered, this, &SpeechView::changeWaveformSettings);
    connect(action_pitch_settings, &QAction::triggered, this, &SpeechView::changePitchSettings);
    connect(action_formants_settings, &QAction::triggered, this, &SpeechView::changeFormantsSettings);
    connect(action_spectrum_settings, &QAction::triggered, this, &SpeechView::changeSpectrogramSettings);
    connect(action_intensity_settings, &QAction::triggered, this, &SpeechView::changeIntensitySettings);
    connect(waveform, &SpeechPlot::windowHasChanged, this, &SpeechView::setWindowTimes);
    connect(doc, &QAction::triggered, this, &SpeechView::showDocumentation);

    return toolbar;
}

void SpeechView::zoomIn(bool)
{
	for (auto plot : plots) {
		plot->zoomIn();
	}
}

void SpeechView::zoomOut(bool)
{
	for (auto plot : plots) {
		plot->zoomOut();
	}
}

void SpeechView::zoomToSelection(bool)
{
	for (auto plot : plots) {
		plot->zoomToSelection();
	}
}

void SpeechView::viewAll(bool)
{
	for (auto plot : plots) {
		plot->viewAll();
	}
}

void SpeechView::moveForward(bool)
{
	for (auto plot : plots) {
		plot->moveForward();
	}
}

void SpeechView::moveBackward(bool)
{
	for (auto plot : plots) {
		plot->moveBackward();
	}
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

    for (auto plot : plots) {
    	plot->setWindow(t1, t2);
    }
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
    Settings::set_value(runtime, "enable_mouse_tracking", enable);

    for (auto plot : plots) {
    	plot->enableMouseTracking(enable);
    	plot->repaint();
    }
}

void SpeechView::showSpectrogram(bool checked)
{
	auto s = String::format("phon.settings.sound_plots['spectrogram'] = %s", (checked ? "true" : "false"));
	runtime.do_string(s);
	spectrogram->setVisible(checked);
	spectrogram_line->setVisible(checked);
}

void SpeechView::showPitch(bool checked)
{
	auto s = String::format("phon.settings.sound_plots['pitch'] = %s", (checked ? "true" : "false"));
	runtime.do_string(s);
    pitch_plot->setVisible(checked);
    pitch_line->setVisible(checked);
}

void SpeechView::showFormants(bool checked)
{
	auto s = String::format("phon.settings.sound_plots['formants'] = %s", (checked ? "true" : "false"));
	runtime.do_string(s);
	spectrogram->enableFormantTracking(checked);
	spectrogram->repaint();
}

void SpeechView::changeWaveformSettings(bool)
{
	WaveformSettings dlg(runtime, this);

	if (dlg.exec() == QDialog::Accepted)
	{
		waveform->updateSettings();
	}
}

void SpeechView::changePitchSettings(bool)
{
    PitchSettings dlg(runtime, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        pitch_plot->updateSettings();
    }
}

void SpeechView::changeFormantsSettings(bool)
{
	FormantSettings dlg(runtime, this);
	
	if (dlg.exec() == QDialog::Accepted)
	{
		spectrogram->updateSettings();
	}
}

void SpeechView::changeIntensitySettings(bool)
{
    IntensitySettings dlg(runtime, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        intensity_plot->updateSettings();
    }
}

void SpeechView::changeSpectrogramSettings(bool)
{
    SpectrogramSettings dlg(runtime, this);

    if (dlg.exec() == QDialog::Accepted)
    {
		spectrogram->updateSettings();
    }
}

void SpeechView::showIntensity(bool checked)
{
	auto s = String::format("phon.settings.sound_plots['intensity'] = %s", (checked ? "true" : "false"));
	runtime.do_string(s);
    intensity_plot->setVisible(checked);
    intensity_line->setVisible(checked);
}

void SpeechView::setInitialWindow()
{
    waveform->setInitialWindow();
    spectrogram->setInitialWindow();
    pitch_plot->setInitialWindow();
    intensity_plot->setInitialWindow();
    // Let the layers and the wavebar know about the size of the window
    waveform->informWindow();
}

void SpeechView::setupUi()
{
	PHON_LOG("Creating UI for speech view");

	// Create objects first so that we can connect them to signals
	waveform = new Waveform(runtime, m_data, this);
	spectrogram = new Spectrogram(runtime, m_data, this);
	pitch_plot = new PitchPlot(runtime, m_data, this);
	intensity_plot = new IntensityPlot(runtime, m_data, this);

	plots.append(waveform);
	plots.append(spectrogram);
	plots.append(pitch_plot);
	plots.append(intensity_plot);

	pitch_plot->setMaximumHeight(100);
	intensity_plot->setMaximumHeight(100);

	auto zoom = new SoundZoom(this);
	wavebar = new WaveBar(m_data, this);
	waveform->setGlobalMagnitude(wavebar->magnitude());

	bool track = Settings::get_boolean(runtime, "enable_mouse_tracking");
	waveform->enableMouseTracking(track);
	spectrogram->enableMouseTracking(track);
	pitch_plot->enableMouseTracking(track);
	intensity_plot->enableMouseTracking(track);

	main_layout = new QHBoxLayout;
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

	inner_layout = new QVBoxLayout;
	inner_layout->addWidget(toolbar);
	inner_layout->addLayout(hl);
	inner_layout->addWidget(waveform);
	inner_layout->addWidget(new SpaceLine);
	inner_layout->addWidget(spectrogram);
	spectrogram_line = new SpaceLine;
	inner_layout->addWidget(spectrogram_line);
	inner_layout->addWidget(pitch_plot);
	pitch_line = new SpaceLine;
	inner_layout->addWidget(pitch_line);
	inner_layout->addWidget(intensity_plot);
	intensity_line = new SpaceLine;
	inner_layout->addWidget(intensity_line);
	layer_start = 8;
	addAnnotationLayers(inner_layout);
	inner_layout->addWidget(zoom);
	inner_layout->addWidget(wavebar);
	inner_layout->addSpacing(5);
	inner_layout->setSpacing(0);
//	inner_layout->setStretchFactor(waveform, 2);
//	inner_layout->setStretchFactor(spectrogram, 2);
//	inner_layout->setStretchFactor(pitch_plot, 1);
//	inner_layout->setStretchFactor(intensity_plot, 1);

	y_axis = new YAxisWidget;
	y_axis->setFixedWidth(60);

	for (auto plot : plots) {
		y_axis->addWidget(plot);
	}
	addLayersToYAxis();

	main_layout->addWidget(y_axis);
	main_layout->addLayout(inner_layout);
	main_layout->addSpacing(10);
	main_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(main_layout);

	// All updates of the scrollbar update the zoomer, but only updates created by the user also update the plots
	connect(wavebar, &WaveBar::updatedXAxisSelection, zoom, &SoundZoom::setXAxisSelection);
	connect(wavebar, &WaveBar::selectionStarted, zoom, &SoundZoom::hideSelection);
	connect(waveform, &Waveform::windowHasChanged, wavebar, &WaveBar::setTimeSelection);


	for (auto plot : plots)
	{
		connect(wavebar, &WaveBar::timeSelection, plot, &SpeechPlot::setWindow);
		connect(plot, &SpeechPlot::zoomInRequested, this, &SpeechView::zoomIn);
		connect(plot, &SpeechPlot::zoomOutRequested, this, &SpeechView::zoomOut);
		connect(plot, &SpeechPlot::zoomToSelectionRequested, this, &SpeechView::zoomToSelection);
		connect(plot, &SpeechPlot::yAxisModified, y_axis, &YAxisWidget::refresh);
	}

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

	connect(&player, &AudioPlayer::current_time, waveform, &Waveform::updateTick);
	connect(&player, &AudioPlayer::finished, waveform, &Waveform::hideTick);
	connect(&player, &AudioPlayer::done, this, &SpeechView::onPlayingFinished);
}

void SpeechView::showDocumentation(bool)
{
	runtime.do_string(R"__(
		var page = phon.config.get_documentation_page("annotation.html")
		phon.show_documentation(page)
		)__");
}

void SpeechView::setPlotVisibility()
{
	bool value;
	String category("sound_plots");
	value = Settings::get_boolean(runtime, category, "formants");
	action_enable_formants->setChecked(value);
	value = Settings::get_boolean(runtime, category, "spectrogram");
	action_enable_spectrum->setChecked(value);
	value = Settings::get_boolean(runtime, category, "pitch");
	action_enable_pitch->setChecked(value);
	value = Settings::get_boolean(runtime, category, "intensity");
	action_enable_intensity->setChecked(value);
}

} // namespace phonometrica