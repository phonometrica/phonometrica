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
