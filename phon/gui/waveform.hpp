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
 * Created: 18/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Widget displaying a waveform.                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVEFORM_HPP
#define PHONOMETRICA_WAVEFORM_HPP

#include <QWidget>
#include <phon/gui/speech_plot.hpp>

namespace phonometrica {


class Waveform final : public SpeechPlot
{
    Q_OBJECT

    enum class Scaling
    {
    	Global,
    	Local,
    	Fixed
    };

public:

    Waveform(Runtime &rt, const AutoSound &sound, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

    void informWindow();

    void setGlobalMagnitude(double value);

public slots:

	void updateWindow(double start_time, double end_time) override;

protected:

    void renderPlot(QPaintEvent *event) override;

    bool isMainPlot() const override { return true; }

    bool needsRefresh() const override;

	void moveWindow(double t1, double t2) override;

	void readSettings() override;

	void emptyCache() override;

	void mouseMoveEvent(QMouseEvent *event) override;

private:

	void setMagnitude(double value);

    void drawWave();

    void setLocalMagnitude(const double *from, const double *to);

    // Map sample to plot y coordinate.
    double sampleToHeight(double s) const;

    double yPosToAmplitude(int y) const;

    // We cache the sinusoidal so that we don't need to recalculate it on each redraw if the geometry and window
    // haven't changed.
    QPainterPath cached_path;

    // Cache the width and height of the plot
    int cached_width = 0;
    int cached_height = 0;

    Scaling scaling = Scaling::Fixed;
    double magnitude = 1.0;
    double global_magnitude = 1.0;

    std::pair<double,double> extrema = {-1, 1};
};

} // namespace phonometrica

#endif // PHONOMETRICA_WAVEFORM_HPP
