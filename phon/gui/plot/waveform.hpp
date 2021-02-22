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

private:

	void InvalidateCache() override;

	void DrawYAxis(wxPaintDC &dc, const wxRect &rect) override;

	void UpdateCache();

	std::vector<std::pair<double,double>> DownsampleWaveform();

	void ReadSettings();

	// Map sample to plot y coordinate.
    double SampleToHeight(double s) const;

    void SetMagnitude(double value);

    void SetLocalMagnitude(std::span<const double> data);

    void SetGlobalMagnitude(double value);

	void OnPaint(wxPaintEvent &);

	void Render(wxPaintDC &dc);

	void DrawBitmap();

	// We precompute and cache a downsampled representation of the signal. Each point corresponds to a pixed and stores
	// the amplitude extrema found in the range represented by the pixel.
    wxBitmap m_cached_bmp;

    // Cache the size of the plot when we compute the data
    wxSize m_cached_size;

    Scaling scaling = Scaling::Fixed;

    double magnitude = 1.0;

    double global_magnitude = 1.0;

    std::pair<double,double> extrema = {-1, 1};

    int m_channel;
};

} // namespace phonometrica



#endif // PHONOMETRICA_WAVEFORM_HPP
