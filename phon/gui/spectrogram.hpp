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
 * Created: 04/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: display spectrogram in speech view.                                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPECTROGRAM_HPP
#define PHONOMETRICA_SPECTROGRAM_HPP

#include <QImage>
#include <phon/gui/speech_plot.hpp>
#include <phon/utils/matrix.hpp>
#include <phon/analysis/signal_processing.hpp>

namespace phonometrica {

class Runtime;


class Spectrogram final : public SpeechPlot
{
    Q_OBJECT

public:

    Spectrogram(Runtime &rt, const AutoSound &sound, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

    void enableFormantTracking(bool value);

    bool hasFormants() const;

protected:

    void renderPlot(QPaintEvent *event) override;

    virtual bool needsRefresh() const override;

    void resizeEvent(QResizeEvent *) override;

	void readSettings() override;

	void emptyCache() override;

	void mouseMoveEvent(QMouseEvent *event) override;

private:

	Matrix<double> computeSpectrogram();

	void estimateFormants();

	int formantToYPos(double hz);

	void readSpectrogramSettings();

	void readFormantsSettings();

	double yPosToHertz(int y) const;

	// Cached spectrogram.
	QImage image;

	// A matrix containing i time measurements across j formants.
	Matrix<double> formants;

	QList<QPainterPath> formant_paths;

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

	// Formant candidates whose bandwidth is higher than this threshold will be discarded.
	double max_formant_bandwidth;

	// Number of prediction coefficients for LPC analysis.
	int lpc_order;

	// Number of formants to display.
	int nformant;

	// Window type for the spectrogram.
	speech::WindowType window_type;

	// Enable formant tracking.
	bool show_formants = false;
};

} // namespace phonometrica

#endif //PHONOMETRICA_SPECTROGRAM_HPP
