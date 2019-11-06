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
 * Purpose: Abstract base class for all widgets (plots and annotation layers) that can be displayed in a speech view.  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPEECH_WIDGET_HPP
#define PHONOMETRICA_SPEECH_WIDGET_HPP

#include <QWidget>

namespace phonometrica {

class SpeechWidget : public QWidget
{
public:

    SpeechWidget(QWidget *parent = nullptr);

    virtual void drawYAxis(QWidget *y_axis, int y1, int y2) = 0;

	virtual void setAddingAnchor(bool value) { adding_anchor = value; }

	virtual void setRemovingAnchor(bool value) { removing_anchor = value; }

	std::pair<double,double> currentWindow() const { return { window_start, window_end }; }

	virtual void setYAxisItemVisible(bool value) { }

protected:

    double xPosToTime(double x) const;

    double timeToXPos(double t) const;

    // Window size in seconds
    double windowDuration() const { return window_end - window_start; }

    // Current window (in seconds)
    double window_start, window_end;

	// True when the "Add anchor" button has been clicked, and until the user adds an anchor or unchecks the button.
	bool adding_anchor = false;

	// True when the "Remove button" has been clicked, and until the user removes an anchor or unchecks the button.
	bool removing_anchor = false;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SPEECH_WIDGET_HPP
