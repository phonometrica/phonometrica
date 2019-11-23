/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 23/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: display and edit annotation.                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ANNOTATION_VIEW_HPP
#define PHONOMETRICA_ANNOTATION_VIEW_HPP

#include <phon/gui/views/speech_view.hpp>
#include <phon/gui/layer_widget.hpp>
#include <phon/application/annotation.hpp>
#include <phon/gui/toolbar.hpp>

class QAction;
class QToolButton;

namespace phonometrica {

class AnnotationView final : public SpeechView
{
    Q_OBJECT

public:

    AnnotationView(Runtime &rt, std::shared_ptr<Annotation> annot, QWidget *parent = nullptr);

    bool save() override;

    void openSelection(intptr_t layer, double from, double to);

    AutoAnnotation annotation() const { return annot; }

	bool finalize() override;

protected:

    void addAnnotationMenu(Toolbar *toolbar) override;

    void addAnnotationLayers(QVBoxLayout *layout) override;

	void addLayersToYAxis() override;

private slots:

    void focusLayer(intptr_t i);

    void focusEvent(intptr_t index, double time, bool forward);

    void setMovingAnchor(intptr_t layer, double time);

    void resetAnchorMovement(intptr_t layer);

    void saveAnnotation(bool);

    void createLayer(bool);

    void removeLayer(bool);

    void clearLayer(bool);

    void renameLayer(bool);

    void duplicateLayer(bool);

    void setAddAnchorEnabled(bool checked);

    void setRemoveAnchorEnabled(bool checked);

    void showHideLayers(bool checked);

    void notifyAnchorAdded(intptr_t layer_index, double time);

    void notifyAnchorRemoved(intptr_t layer_index, double time);

	void notifyAnchorMoved(intptr_t layer_index, double from, double to);

    void onAnchorSelected(intptr_t layer_index, double time);

    void onEditAnchor(intptr_t layer_index, double time);

    void setTemporaryAnchor(intptr_t layer_index, double time);

private:

	int widgetIndex(int layer_index);

	int getFocusedLayer() const;

	void updateLayerInfo();

	void clearGhostAnchors();

	void setAnchorSharing(bool value);

	LayerWidget * addAnnotationLayer(intptr_t i);

	QToolButton *link_button = nullptr;

	QAction *add_anchor_action = nullptr;

	QAction *remove_anchor_action = nullptr;

    std::shared_ptr<Annotation> annot;

    // Layer widgets, in the order they are stored and displayed
    Array<LayerWidget*> layers;
};

} // namespace phonometrica

#endif // PHONOMETRICA_ANNOTATION_VIEW_HPP
