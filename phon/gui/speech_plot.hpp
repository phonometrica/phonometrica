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
 * Created: 20/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: base class for all speech plots.                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPEECH_PLOT_HPP
#define PHONOMETRICA_SPEECH_PLOT_HPP

#include <QPainter>
#include <QMouseEvent>
#include <phon/application/sound.hpp>
#include <phon/gui/speech_widget.hpp>
#include <phon/gui/mouse_tracking.hpp>

namespace phonometrica {

class Runtime;

class SpeechPlot : public SpeechWidget
{
    Q_OBJECT

public:

    SpeechPlot(Runtime &rt, const AutoSound &sound, QWidget *parent = nullptr);

    void zoomIn();

    void zoomOut();

    void zoomToSelection();

    void viewAll();

    void moveForward();

    void moveBackward();

    std::pair<double, double> times() const;

    void enableMouseTracking(bool enable);

    void clearCurrentTime();

    void setInitialWindow();

    void updateSettings();

    double persistentCursor() const { return m_persistent_cursor; }

	void clearPersistentCursor() { m_persistent_cursor = -1; }

	void setPersistentCursor(double value);

	bool hasPersistentCursor() const { return m_persistent_cursor >= 0; }

	double selectionDuration() const { return sel_end - sel_start; }

	double windowDuration() const { return window_end - window_start; }

signals:

    void timeSelection(double, double);

    void windowHasChanged(double, double);

    void yAxisModified();

    void currentTime(double, MouseTracking);

    void selectionStarted();

	void zoomInRequested(bool dummy);

	void zoomOutRequested(bool dummy);

	void zoomToSelectionRequested(bool dummy);

	void persistentCursorRequested(double time);

	void statusMessage(const QString &);

public slots:

    void setWindow(double start_time, double end_time);

	virtual void updateWindow(double start_time, double end_time);

    void setSelection(double start_time, double end_time);

    void updateTick(double time);

    void hideTick();

    void setCurrentTime(double time, MouseTracking tracking);

    void invalidateCurrentTime() { current_time = -1.0; }

protected:

    virtual bool needsRefresh() const;

    virtual void readSettings() { }

    virtual void emptyCache() { }

    void paintEvent(QPaintEvent* event) override;

    void wheelEvent(QWheelEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void leaveEvent(QEvent *event) override;

    void updateSelectionEnd(double t);

    virtual bool isMainPlot() const { return false; }

    virtual void moveWindow(double t1, double t2);

    bool hasSelection() const;

    void clearSelection();

    void drawSelection();

    bool hasTick() const { return tick >= 0; }

    double clipTime(double t) const;

    void updatePlot();

    bool hasCurrentTime() const { return current_time >= 0; }

    bool trackCursor() const;

    // Subclasses must draw the plot in this method
    virtual void renderPlot(QPaintEvent *event) = 0;

    Runtime &rt;

    AutoSound m_sound;

    std::shared_ptr<AudioData> m_data;

    // Cache window (times in seconds)
    double cached_start = 0.0;
    double cached_end = 0.0;

    // Current selection (in seconds)
    double sel_start = -1, sel_end = -1;

    // Track mouse pointer
    double current_time = -1;

    // Tick displayed while sound is playing
    double tick = -1;

    // Time of the persistent cursor.
    double m_persistent_cursor = -1;

    // track mouse button
    bool button_pressed = false;

    // Mouse tracking occurs if a user checks the mouse tracking option or if an anchor is being dragged.
    MouseTracking mouse_tracking = MouseTracking::Enabled;
    MouseTracking mouse_state = MouseTracking::Disabled;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SPEECH_PLOT_HPP
