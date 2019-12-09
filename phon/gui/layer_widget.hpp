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
 * Created: 24/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display a layer in an annotation view.                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_LAYER_WIDGET_HPP
#define PHONOMETRICA_LAYER_WIDGET_HPP

#include <phon/application/annotation.hpp>
#include <phon/gui/speech_widget.hpp>
#include <phon/gui/mouse_tracking.hpp>

class QPushButton;
class QDialog;

namespace phonometrica {

class LayerInfoDialog;


class LayerWidget final : public SpeechWidget
{
    Q_OBJECT

public:

    LayerWidget(const AutoAnnotation &annot, double duration, intptr_t layer_index, QWidget *parent = nullptr);

    ~LayerWidget() = default;

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

    void unfocus();

    void setEventFocus(double time);

    void followMovingAnchor(double time);

    void clearMovingAnchor() { followMovingAnchor(-1); }

	void setAddingAnchor(bool value) override;

    void setRemovingAnchor(bool value) override;

	void rename(const String &name);

	void updateInfo();

	void removeInfoButton();

	void clearGhostAnchor() { ghost_anchor_time = -1; }

	void hideAnchor(double time);

	void createAnchor(double time, bool silent);

	bool removeAnchor(double time, bool silent);

	bool moveAnchor(double from, double to);

	bool hasInstants() const { return layer->has_instants; }

	bool hasIntervals() const { return !hasInstants(); }

	void setEditAnchor(double value) { edit_anchor_time = value; }

	void setYAxisItemVisible(bool value) override;

signals:

    void got_focus(intptr_t i);

    void current_time(double t, MouseTracking tracking);

    void event_selected(double, double);

    void inform_selection(intptr_t layer, intptr_t event);

    void focus_event(intptr_t layer, double time, bool forward);

    void window_moved(double t1, double t2);

    void modified();

    void anchor_moving(intptr_t layer, double time);

    void anchor_has_moved(intptr_t layer);

    void anchor_added(intptr_t layer, double time);

    void anchor_removed(intptr_t layer, double time);

    void anchor_moved(intptr_t layer, double from, double to);

    void anchor_selected(intptr_t layer, double time);

    void editing_shared_anchor(intptr_t layer, double time);

    void temporary_anchor(intptr_t layer, double time);

public slots:

    void setWindow(double start_time, double end_time);

	void setAnchorSharing(bool value);

	void setGhostAnchorTime(double time);

protected:

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void leaveEvent(QEvent *event) override;

private:

    void drawAnchor(QPainter &painter, double time, bool is_instant);

    bool needsRefresh() const;

    bool anchorHasCursor(double anchor_time, double current_time) const;

    bool eventHasCursor(const Event &event, double time, double *out_time);

    void setSelectedAnchor(const AutoEvent &event, double time, bool selected = true);

    void trackAnchor(double time);

    void focusPreviousEvent();

    void focusNextEvent();

    void setSelectedEvent(const AutoEvent &event);

    void editEvent(AutoEvent &event);

    void updateEvents();

    EventList filter_events(double t1, double t2) const { return graph.get_layer_events(layer->index, t1, t2); }

    double clipLeft(double time) const { return (std::max)(0.0, time); }

    double clipRight(double time) const { return (std::min)(m_duration, time); }

    bool hasDroppedAnchor() const { return dropped_anchor_time > 0; }

    void clearResizingEvent();

    AutoEvent findEvent(double time);

    double timeAtCursor(QMouseEvent *e) const;

	void updateUi();

	void setButtonIcon();

	void clearEditAnchor() { setEditAnchor(-1); }

	bool hasEditAnchor() const { return edit_anchor_time >= 0; }

	bool hasGhostAnchor() const { return ghost_anchor_time >= 0; }

	double findClosestAnchorTime(double time);

	// Metadata button displayed in the y axis.
    QPushButton *button;

    // Metadata dialog controlled by the button.
    LayerInfoDialog *dialog;

    // We need to store a reference to the annotation to avoid a subtle bug that leads to a crash. Due to the way
    // destructors work, the AutoLayer pointer owned by this widget will be released after the AutoAnnotation pointer
    // owned by the annotation view. If the annotation pointer is unique (because it's a temporary annotation which is
    // not registered in the project), the annotation graph will already have been destroyed when we try to finalize
    // the layer. As a result, the anchors that are referenced by the layer's events will be invalid. Therefore,
    // we keep an extra pointer to the annotation *before* the layer pointer to ensure that the anchors still exist.
	AutoAnnotation unused;

	AGraph &graph;

    AutoLayer layer;

    // Duration of the sound file
    double m_duration;

    EventList event_cache;

    AutoEvent selected_event;

    double cached_start = -1;
    double cached_end = -1;

    // Event being resized
    AutoEvent resizing_event;

    // Anchor clicked by the user, which is about to be moved. If this is a valid value, the anchor is not
    // painted. Instead, we look up dropped_anchor_time and paint that one.
    double dragged_anchor_time = -1;

    // New time for the dragged anchor
    double dropped_anchor_time = -1;

    // When an anchor is moved on another layer, we track it
    double moving_anchor_time = -1;

    // When an anchor is being added, we track its time so that we can display a temporary anchor under the cursor.
    double edit_anchor_time = -1;

    // A "ghost" anchor is displayed when we are in anchor adding mode, and an anchor has been clicked or added.
    // Ghost anchors are displayed on all layers which don't have a real anchor at that time point.
    double ghost_anchor_time = -1;

    // When an anchor is being moved on another layer in sharing mode, we hide this anchor (if it exists) on this layer.
    double hidden_anchor_time = -1;

    // Which edge of the selected event was clicked? For instants, this is always the end.
    bool event_start_selected = false;

    // True if the user has clicked an anchor
    bool dragging_anchor = false;

    // Layer currently selected by the user
    bool has_focus = false;

    // Flag to share/unshare anchors on all layers
	bool sharing_anchors = true;

    // If the user moves an event editor, we record the shift to display the next one
    // at the same height. This avoids hiding useful information when a user needs information
    // from several layers.
    int edit_y_shift = 0;
};

} // namespace phonometrica

#endif // PHONOMETRICA_LAYER_WIDGET_HPP
