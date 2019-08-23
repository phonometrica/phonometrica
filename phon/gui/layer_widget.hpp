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
 * Created: 24/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Display a layer in an annotation view. The layer is managed by a LayerModel.                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_LAYER_WIDGET_HPP
#define PHONOMETRICA_LAYER_WIDGET_HPP

#include <phon/application/agraph.hpp>
#include <phon/gui/speech_widget.hpp>

namespace phonometrica {

class LayerWidget final : public SpeechWidget
{
    Q_OBJECT

public:

    LayerWidget(AGraph &graph, double duration, intptr_t layer_index, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

    void unfocus();

    void setEventFocus(double time);

    static QColor lineColor();

    void followMovingAnchor(double time);

    void clearMovingAnchor() { followMovingAnchor(-1); }

signals:

    void got_focus(intptr_t i);

    void current_time(double t, bool force);

    void interval_selected(double, double);

    void focus_event(intptr_t layer, double time);

    void window_moved(double t1, double t2);

    void modified();

    void anchor_moving(intptr_t layer, double time);

    void anchor_has_moved(intptr_t layer);

public slots:

    void setWindow(double start_time, double end_time);

protected:

    void paintEvent(QPaintEvent *) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:

    void drawAnchor(QPainter &painter, double time);

    bool needsRefresh() const;

    bool anchorHasCursor(double anchor_time, double current_time) const;

    bool eventHasCursor(const Event &event, double time, double *out_time);

    void setSelectedAnchor(const std::shared_ptr<Event> &event, double time, bool selected = true);

    void trackAnchor(double time);

    void focusPreviousEvent();

    void focusNextEvent();

    void setSelectedEvent(const std::shared_ptr<Event> &event);

    void editEvent(std::shared_ptr<Event> &event);

    void updateEvents();

    bool has_instants() const { return graph.get(layer_index)->has_instants; }

    bool has_intervals() const { return !has_instants(); }

    EventList filter_events(double t1, double t2) const { return graph.get_layer_events(layer_index, t1, t2); }

    double clipLeft(double time) const { return (std::max)(0.0, time); }

    double clipRight(double time) const { return (std::min)(m_duration, time); }

    bool hasDroppedAnchor() const { return dropped_anchor_time > 0; }

    void clearResizingEvent();

    AGraph &graph;

    intptr_t layer_index;

    // Duration of the sound file
    double m_duration;

    EventList event_cache;

    std::shared_ptr<Event> selected_event;

    double cached_start = -1;
    double cached_end = -1;

    // Event being resized
    std::shared_ptr<Event> resizing_event;

    // Anchor clicked by the user, which is about to be moved. If this is a valid value, the anchor is not
    // painted. Instead, we look up dropped_anchor_time and paint that one.
    double dragged_anchor_time = -1;

    // New time for the dragged anchor
    double dropped_anchor_time = -1;

    // When an anchor is moved on another layer, we track it
    double moving_anchor_time = -1;

    // Which edge of the selected event was clicked? For instants, this is always the end.
    bool event_start_selected = false;

    // True if the user has clicked an anchor
    bool dragging_anchor = false;

    // Layer currently selected by the user
    bool has_focus = false;

    // If the user moves an event editor, we record the shift to display the next one
    // at the same height. This avoids hiding useful information when a user needs information
    // from several tiers.
    int edit_y_shift = 0;
};

} // namespace phonometrica

#endif // PHONOMETRICA_LAYER_WIDGET_HPP
