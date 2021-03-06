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
 * Created: 26/03/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display a spectrogram in a sound or annotation view.                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPECTROGRAM_HPP
#define PHONOMETRICA_SPECTROGRAM_HPP

#include <wx/timer.h>
#include <phon/gui/plot/sound_plot.hpp>
#include <phon/analysis/signal_processing.hpp>

namespace phonometrica {

class Spectrogram final : public SoundPlot
{
public:

	Spectrogram(wxWindow *parent, const Handle <Sound> &snd, int channel);

	bool HasFormants() const;

	void ShowFormants(bool value);

private:

	void DrawYAxis(PaintDC &dc, const wxRect &rect) override;

	void ReadSettings() override;

	void ReadSpectrogramSettings();

	void ReadFormantSettings();

	void UpdateCache() override;

	void DrawFormants();

	Matrix<double> ComputeSpectrogram();

	void EstimateFormants();

	int FormantToYPos(double hz);

	void OnMotion(wxMouseEvent &e) override;

	double YPosToHertz(int y) const;

	void OnTimer(wxTimerEvent &event);

	void InvalidateCache() override;

	// A matrix containing i time measurements across j formants.
	Matrix<double> formants;

	// Duration of the analysis window for spectrograms.
	double spectrum_window_length;

	// Highest frequency.
	double max_freq;

	// Pre-emphasis factor.
	double preemph_threshold;

	// Dynamic range (in dB). Values below the threshold [max_dB - dynamic_range] are treated as 0.
	int dynamic_range;

	// Duration of the analysis window for formants.
	double formant_window_length;

	// Nyquist frequency range for formant analysis.
	double max_formant_frequency;

	// Time step for formant analysis.
	double time_step;

	// Timer for the error message in formant tracking.
	wxTimer timer;

	// Number of prediction coefficients for LPC analysis.
	int lpc_order;

	// Number of formants to display.
	int nformant;

	// Window type for the spectrogram.
	speech::WindowType window_type;

	// Enable formant tracking.
	bool show_formants = true;

	// Whether an error message has been shown for the current window in formant tracking.
	bool formant_error = false;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SPECTROGRAM_HPP
