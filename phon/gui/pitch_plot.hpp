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
 * Created: 29/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display pitch track.                                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PITCH_PLOT_HPP
#define PHONOMETRICA_PITCH_PLOT_HPP

#include <phon/gui/speech_plot.hpp>

namespace phonometrica {

class Runtime;


class PitchPlot final : public SpeechPlot
{
    Q_OBJECT

public:

    PitchPlot(Runtime &rt, const AutoSound &sound, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

protected:

    void renderPlot(QPaintEvent *event) override;

    bool needsRefresh() const override;

	void readSettings() override;

	void emptyCache() override;

	void mouseMoveEvent(QMouseEvent *event) override;

private:

    double pitchToYPos(double hz) const;

    double yPosToHertz(int y) const;

    void calculatePitch();

    std::vector<double> pitch_data;

    double min_pitch;

    double max_pitch;

    double time_step; // in seconds

    double voicing_threshold; // 0.5 > t > 0.2
};

} // namespace phonometrica

#endif // PHONOMETRICA_PITCH_PLOT_HPP
