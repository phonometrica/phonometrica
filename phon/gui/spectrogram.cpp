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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <complex>
#include <vector>
#include <ffts.h>
#include <QDebug>
#include <phon/gui/spectrogram.hpp>
#include <phon/speech/signal_processing.hpp>
#include <phon/speech/speech_utils.hpp>
#include <phon/utils/matrix.hpp>

namespace phonometrica {

Spectrogram::Spectrogram(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent) :
	SpeechPlot(rt, std::move(data), parent)
{

}

void Spectrogram::drawYAxis(QWidget *y_axis, int y1, int y2)
{
	QPainter painter(y_axis);
	auto font_metrics = painter.fontMetrics();
	auto top = QString::number(long(ceiling_freq));
	QString bottom = QString::number(long(floor_freq));
	top.append(" Hz");
	bottom.append(" Hz");
	int h = font_metrics.height();

	int x1 = y_axis->width() - font_metrics.width(top);
	painter.drawText(x1, y1 + 5 + h/2, top);

	int x2 = y_axis->width() - font_metrics.width(bottom);
	painter.drawText(x2, y2-5, bottom);
}

void Spectrogram::renderPlot(QPaintEvent *event)
{
	if (needsRefresh())
	{
		auto raster = computeSpectrogram();

		// Values below the threshold [max_dB - dynamic_range] are treated as 0.
		int dynamic_range = 70;

		image = QImage(width(), height(), QImage::Format_RGB32);
		image.fill(QColor(255, 255, 255));

		// We can't use Eigen's maxCoeff()/minCoeff() because we have nan's in the matrix.
		double max_dB = -100000;
		double min_dB = 100000;

		for (int i = 0; i < raster.rows(); i++)
		{
			for (int j = 0; j < raster.cols(); j++)
			{
				double val = raster(i,j);

				if (std::isfinite(val))
				{
					if (val > max_dB) max_dB = val;
					if (val < min_dB) min_dB = val;
				}
			}
		}
		qDebug() << "max = " << max_dB << " min = " << min_dB;
		// Adjust minimum to fit the dynamic range. All values lower than max - dynamic_range will be white.
		min_dB = (std::max)(min_dB, max_dB - dynamic_range);

		for (int i = 0; i < raster.rows(); i++)
		{
			// The image needs to be reversed
			for (int j = 0; j < raster.cols(); j++)
			{
				// shade of gray
				double value = (std::max)(raster(i, j), min_dB);
				if (std::isnan(value)) value = min_dB; // handle pixels that could not be calculated.
				int g = 255 - round((value - min_dB) * 255 / (max_dB - min_dB));
				assert(g >= 0);
				image.setPixel(i, height() - j - 1, qRgb(g,g,g));
			}
		}
	}

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.drawImage(0, 0, image);
}

bool Spectrogram::needsRefresh() const
{
	return SpeechPlot::needsRefresh() || image.isNull();
}

Matrix<double> Spectrogram::computeSpectrogram()
{
	using namespace speech;

	auto sample_rate = m_data->sample_rate();
	auto nyquist_frequency = double(sample_rate) / 2;
	auto nframe = int(sample_rate * window_length);
	if (nframe % 2 == 1) nframe++;
	//win_size *= 2;
	int nfft = 256;
	while (nfft < nframe) nfft *= 2;

	auto half_win = nfft / 2;

	std::vector<double>amplitude(half_win, 0.0);
	intptr_t w = this->width();
	intptr_t h = this->height();

	// An m x n matrix, where m represents the number of horizontal pixels and n represents the number of vertical pixels.
	// Each cell stores the intensity of the corresponding pixel.
	Matrix<double> raster(w, h);
	raster.setZero(w, h);

	auto hann = create_window(nframe, nfft, WindowType::Hann);


	// Get audio data. If possible, we try to get a bit more data before and after the window so that we can calculate
	// frames at the edge.
	double total_duration = m_data->duration();
	double start_time = (std::max<double>)(0.0, window_start - window_length / 2);
	double end_time = (std::min<double>)(total_duration, window_end + window_length / 2);
	auto first_sample = m_data->time_to_frame(start_time);
	auto last_sample = m_data->time_to_frame(end_time);

	// Each point represents the center of an analysis window, corresponding to a horizontal pixel.
	auto xpoints = linspace(window_start, window_end, w, true);

	double weight = 0;
	for (double x : hann) weight += x * x;
	double k1 = 1 / (sample_rate * weight); // at DC and Nyquist frequencies.
	double k2 = 2 / (sample_rate * weight); // at other frequencies


	ffts_plan_t *plan = ffts_init_1d(nfft, FFTS_FORWARD);
	std::vector<std::complex<float>> input(nfft, std::complex<float>(0, 0));
	std::vector<std::complex<float>> output(nfft, std::complex<float>(0, 0));

	Array<float> buffer = m_data->float_data(first_sample, last_sample);
	pre_emphasis(buffer);

	for (intptr_t x = 0; x < xpoints.size(); x++)
	{
		auto xpoint = xpoints[x];
		auto mid_sample = m_data->time_to_frame(xpoint);
		auto offset = (mid_sample - first_sample) - (nframe / 2);
		auto it = buffer.begin() + offset;

		if (offset < 0 || mid_sample + (nframe / 2) > last_sample)
		{
			// Can't calculate FFT because we would get outside of the bounds.
			// Display a white vertical column and move to the next time point.
			for (int j = 0; j < h; j++)
			{
				raster(x, j) = std::nan(""); // it will be converted to the minimum intensity when it is displayed.
			}
			continue;
		}

		// Calculate fft
		for (intptr_t j = 0; j < nframe; j++)
		{
			auto sample = float((*it++) * hann[j+1]);
			input[j] = {sample, 0 };
		}
		for (intptr_t j = nframe; j < nfft; j++)
		{
			input[j] = {0, 0};
		}

		ffts_execute(plan, input.data(), output.data());

		for (size_t y = 0; y < half_win; y++)
		{
			double a = output[y].real() * output[y].real() + output[y].imag() * output[y].imag();
			assert(std::isfinite(a));
			double k = (y == 0 || y == half_win-1) ? k1 : k2;
			a = k * a / nfft;
			constexpr double Iref = 4.0e-10;
			double dB = 10 * log10(a / Iref);
			amplitude[y] = dB;
		}

		// Create raster: frequencies on the y axis are mapped to the closest frequency bin.
		double ceiling_bin = ceiling_freq * half_win / nyquist_frequency;
		for (intptr_t y = 1; y <= h; y++)
		{
			double freq = double(y * ceiling_bin) / (h-1);
			int bin = int(freq);

			if (bin == half_win)
			{
				raster(x, y - 1) = amplitude[bin];
			}
			else
			{
				// Do a linear interpolation.
				double a1 = amplitude[bin];
				double a2 = amplitude[bin+1];
				double delta = a2 - a1;
				double remainder = freq - bin;
				double amp = a1 + (delta * remainder);
				assert(!std::isnan(amp));
				raster(x, y - 1) = amp;
			}
		}
	}

	ffts_free(plan);
	cached_start = window_start;
	cached_end = window_end;

	return raster;
}

void Spectrogram::resizeEvent(QResizeEvent *e)
{
	image = QImage();
	QWidget::resizeEvent(e);
}
} // namespace phonometrica
