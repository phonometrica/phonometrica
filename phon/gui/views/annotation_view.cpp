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
 * Created: 23/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QToolBar>
#include <QFileDialog>
#include <QSpacerItem>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>
#include <phon/gui/views/annotation_view.hpp>

namespace phonometrica {

AnnotationView::AnnotationView(Runtime &rt, std::shared_ptr<Annotation> annot, QWidget *parent) :
    SpeechView(rt, annot->sound()->data(), parent), annot(std::move(annot))
{
    this->annot->open();
}

void AnnotationView::addEditButtons(Toolbar *toolbar)
{
	auto save_action = new QAction(QIcon(":/icons/save.png"), "Save annotation");
	toolbar->addAction(save_action);
	toolbar->addSeparator();

	connect(save_action, &QAction::triggered, this, &AnnotationView::saveAnnotation);
}

void AnnotationView::addAnnotationMenu(Toolbar *toolbar)
{
    auto layer_action = new QAction(QIcon(":/icons/layers.png"), "Manage layers");
    auto anchor_action = new QAction(QIcon(":/icons/anchor.png"), "Manage anchors");

    toolbar->addAction(layer_action);
    toolbar->addAction(anchor_action);
}

void AnnotationView::addAnnotationLayers(QVBoxLayout *layout)
{
    intptr_t count = annot->size();

    for (intptr_t i = 1; i <= count; i++)
    {
        auto duration = annot->sound()->duration();
        auto widget = new LayerWidget(annot->graph(), duration, i);
        //layout->addSpacing(2);
        layout->addWidget(widget);
        layers.append(widget);
        connect(widget, &LayerWidget::got_focus, this, &AnnotationView::focusLayer);
        connect(widget, &LayerWidget::current_time, waveform, &Waveform::setCurrentTime);
        connect(widget, &LayerWidget::current_time, pitch_plot, &PitchPlot::setCurrentTime);
        connect(widget, &LayerWidget::current_time, intensity_plot, &IntensityPlot::setCurrentTime);

        connect(widget, &LayerWidget::interval_selected, waveform, &Waveform::setSelection);
        connect(widget, &LayerWidget::interval_selected, pitch_plot, &PitchPlot::setSelection);
        connect(widget, &LayerWidget::interval_selected, intensity_plot, &IntensityPlot::setSelection);

        connect(widget, &LayerWidget::focus_event, this, &AnnotationView::focusEvent);
        connect(widget, &LayerWidget::modified, this, &AnnotationView::modified);
        connect(widget, &LayerWidget::anchor_moving, this, &AnnotationView::setMovingAnchor);
        connect(widget, &LayerWidget::anchor_has_moved, this, &AnnotationView::resetAnchorMovement);
        // When a layer triggers a window shift, we need to update the scrollbar and the plots
        connect(widget, &LayerWidget::window_moved, wavebar, &WaveBar::setTimeSelection);
        connect(widget, &LayerWidget::window_moved, waveform, &Waveform::setWindow);
        connect(widget, &LayerWidget::window_moved, pitch_plot, &PitchPlot::setWindow);
        connect(widget, &LayerWidget::window_moved, intensity_plot, &IntensityPlot::setWindow);

        connect(waveform, &Waveform::windowHasChanged, widget, &LayerWidget::setWindow);
    }
}

void AnnotationView::save()
{
    if (annot->modified())
    {
        if (!annot->has_path())
        {
            QString dir = Settings::get_string(rt, "last_directory");
            auto path = QFileDialog::getSaveFileName(this, tr("Save annotation..."), dir, tr("Annotation (*.annot)"));

            if (path.isEmpty()) {
                return; // cancelled
            }
            annot->set_path(path, true);
            Project::instance()->register_file(path, annot);
        }
        annot->save();
        emit saved();
    }
}

void AnnotationView::focusLayer(intptr_t index)
{
    for (intptr_t i = 1; i <= layers.size(); i++)
    {
        auto layer = layers[i];
        if (i != index) layer->unfocus();
    }
}

void AnnotationView::focusEvent(intptr_t index, double time)
{
    for (intptr_t i = 1; i <= layers.size(); i++)
    {
        auto layer = layers[i];
        if (i == index)
        {
            layer->setEventFocus(time);
        }
        else
        {
            layer->unfocus();
        }
    }
}

void AnnotationView::setMovingAnchor(intptr_t layer, double time)
{
    for (intptr_t i = 1; i <= layers.size(); i++)
    {
        if (i != layer)
        {
            layers[i]->followMovingAnchor(time);
        }
    }
    waveform->setCurrentTime(time, true);
}

void AnnotationView::resetAnchorMovement(intptr_t layer)
{
    for (intptr_t i = 1; i <= layers.size(); i++)
    {
        if (i != layer)
        {
            layers[i]->clearMovingAnchor();
        }
    }

    waveform->clearCurrentTime();
}

void AnnotationView::openSelection(intptr_t layer, double from, double to)
{
	setSelection(from, to);
	auto t = from + (to - from) / 2;
	focusEvent(layer, t);
}

void AnnotationView::saveAnnotation(bool)
{
	if (!annot->has_path())
	{
		QString dir = Settings::get_string(rt, "last_directory");
		auto path = QFileDialog::getSaveFileName(this, tr("Save annotation..."), dir, tr("Annotation (*.phon-annot)"));

		if (path.isEmpty()) {
			return; // cancelled
		}
		if (!path.endsWith(PHON_EXT_ANNOTATION)) {
			path.append(PHON_EXT_ANNOTATION);
		}
		annot->set_path(path, true);
	}

	annot->save();
	Project::instance()->notify_update();
}

void AnnotationView::addLayersToYAxis()
{
	for (auto &layer : layers) {
		y_axis->addWidget(layer);
	}
}


} // namespace phonometrica