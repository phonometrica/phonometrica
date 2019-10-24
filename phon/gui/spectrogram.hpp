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
#include <phon/speech/signal_processing.hpp>

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
