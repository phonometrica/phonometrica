/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 25/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Intensity track in sound and annotation views.                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_INTENSITY_TRACK_HPP
#define PHONOMETRICA_INTENSITY_TRACK_HPP

#include <phon/gui/plot/sound_plot.hpp>

namespace phonometrica {

class IntensityTrack final : public SoundPlot
{
public:

	IntensityTrack(wxWindow *parent, const Handle <Sound> &snd, int channel);

private:

	void DrawYAxis(PaintDC &dc, const wxRect &rect) override;

	void UpdateCache() override;

	void ReadSettings() override;

	void OnMotion(wxMouseEvent &e) override;

	void DrawBitmap();

	Array<double> CalculateIntensity();

    double IntensityToYPos(double dB) const;

    double YPosToIntensity(int y) const;


    double min_dB = 50;

    double max_dB = 100;

    double time_step = 0.01;
};

} // namespace phonometrica



#endif // PHONOMETRICA_INTENSITY_TRACK_HPP
