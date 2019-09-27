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
 * Created: 24/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display a layer in an annotation view.                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_LAYER_WIDGET_HPP
#define PHONOMETRICA_LAYER_WIDGET_HPP

#include <phon/application/agraph.hpp>
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

    LayerWidget(AGraph &graph, double duration, intptr_t layer_index, QWidget *parent = nullptr);

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

signals:

    void got_focus(intptr_t i);

    void current_time(double t, MouseTracking tracking);

    void event_selected(double, double);

    void focus_event(intptr_t layer, double time);

    void window_moved(double t1, double t2);

    void modified();

    void anchor_moving(intptr_t layer, double time);

    void anchor_has_moved(intptr_t layer);

    void anchor_added(bool);

    void anchor_removed(bool);

    void anchor_selected(intptr_t layer, double time);

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

    bool has_instants() const { return layer->has_instants; }

    bool has_intervals() const { return !has_instants(); }

    EventList filter_events(double t1, double t2) const { return graph.get_layer_events(layer->index, t1, t2); }

    double clipLeft(double time) const { return (std::max)(0.0, time); }

    double clipRight(double time) const { return (std::min)(m_duration, time); }

    bool hasDroppedAnchor() const { return dropped_anchor_time > 0; }

    void clearResizingEvent();

    AutoEvent findEvent(double time);

    double timeAtCursor(QMouseEvent *e) const;

    void createAnchor(double time);

    bool removeAnchor(double time);

	void updateUi();

	void setButtonIcon();

	void clearEditAnchor() { edit_anchor_time = -1; }

	bool hasEditAnchor() const { return edit_anchor_time >= 0; }

	bool hasGhostAnchor() const { return ghost_anchor_time >= 0; }

	// Metadata button displayed in the y axis.
    QPushButton *button;

    // Metadata dialog controlled by the button.
    LayerInfoDialog *dialog;

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
