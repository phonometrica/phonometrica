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
 * Purpose: base class for all speech plots.                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPEECH_PLOT_HPP
#define PHONOMETRICA_SPEECH_PLOT_HPP

#include <QPainter>
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

    double selectionDuration() const { return sel_end - sel_start; }

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
