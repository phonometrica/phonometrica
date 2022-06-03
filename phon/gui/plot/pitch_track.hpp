/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Created: 12/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Pitch track in sound and annotation views.                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PITCH_TRACK_HPP
#define PHONOMETRICA_PITCH_TRACK_HPP

#include <phon/gui/plot/sound_plot.hpp>

namespace phonometrica {

class PitchTrack final : public SoundPlot
{
public:

	PitchTrack(wxWindow *parent, const Handle <Sound> &snd, int channel);


private:

	void DrawYAxis(PaintDC &dc, const wxRect &rect) override;

	void UpdateCache() override;

	void ReadSettings() override;

	void DrawBitmap();

	std::vector<double> CalculatePitch();

	double YPosToHertz(int y) const;

	double PitchToYPos(double hz) const;

	std::vector<double> m_pitch;

	double min_pitch;

	double max_pitch;

	double time_step; // in seconds

	double voicing_threshold; // 0.5 > t > 0.2

};

} // namespace phonometrica



#endif // PHONOMETRICA_PITCH_TRACK_HPP
