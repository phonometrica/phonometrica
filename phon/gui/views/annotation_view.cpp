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
 * Created: 23/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

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

void AnnotationView::addAnnotationMenu(QToolBar *toolbar)
{
    auto layer_action = new QAction(QIcon(":/icons/layers.png"), "Manage layers");
    auto anchor_action = new QAction(QIcon(":/icons/anchor.png"), "Manage anchors");
    toolbar->addAction(layer_action);
    toolbar->addAction(anchor_action);
    toolbar->addSeparator();
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

} // namespace phonometrica