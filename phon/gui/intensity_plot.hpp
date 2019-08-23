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
 * Created: 31/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Intensity plot in a sound or annotation view. The normalized intensity of the sound wave is interpreted   *
 * in Pascal. Intensity is expressed in dB SPL, with a reference intensity set to 20 micropascal (or 0.00002 Pascal). *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_INTENSITY_PLOT_HPP
#define PHONOMETRICA_INTENSITY_PLOT_HPP

#include <phon/gui/speech_plot.hpp>

namespace phonometrica {

class IntensityPlot final : public SpeechPlot
{
    Q_OBJECT

public:

    IntensityPlot(Environment &env, std::shared_ptr<AudioData> data, QWidget *parent = nullptr);


    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

protected:

    void renderPlot(QPaintEvent *event) override;

    bool needsRefresh() const override;

private:

    void calculateIntensity();

    double intensityToYPos(double dB) const;

    double findValueAtTime(double time);

    Array<double> db_data;

    double min_dB = 50;
    double max_dB = 100;

    double time_step = 0.01;
};

} // namespace phonometrica

#endif // PHONOMETRICA_INTENSITY_PLOT_HPP
