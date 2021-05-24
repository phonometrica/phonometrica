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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Represents the waveform for a single channel in a sound file.                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVEFORM_HPP
#define PHONOMETRICA_WAVEFORM_HPP

#include <phon/gui/plot/sound_plot.hpp>

namespace phonometrica {

class Waveform final : public SoundPlot
{
	enum class Scaling
    {
    	Global,
    	Local,
    	Fixed
    };

public:

	Waveform(wxWindow *parent, const Handle<Sound> &snd, int channel);

	void SetGlobalMagnitude(double value);

private:

	void DrawYAxis(PaintDC &dc, const wxRect &rect) override;

	void UpdateCache() override;

	wxString GetStatus() override;

	std::vector<std::pair<double,double>> DownsampleWaveform();

	void ReadSettings() override;

	void OnMotion(wxMouseEvent &e) override;

	double YPosToAmplitude(int y) const;

	// Map sample to plot y coordinate.
    double SampleToHeight(double s) const;

    void SetMagnitude(double value);

    void SetLocalMagnitude(std::span<const double> data);

	void DrawBitmap();

    Scaling scaling = Scaling::Fixed;

    double magnitude = 1.0;

    double global_magnitude = 1.0;

    std::pair<double,double> extrema = {-1, 1};
};

} // namespace phonometrica



#endif // PHONOMETRICA_WAVEFORM_HPP
