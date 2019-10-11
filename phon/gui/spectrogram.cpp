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
#include <QMessageBox>
#include <QDebug>
#include <phon/runtime/runtime.hpp>
#include <phon/application/settings.hpp>
#include <phon/gui/spectrogram.hpp>
#include <phon/speech/speech_utils.hpp>
#include <phon/include/reset_spectrogram_settings_phon.hpp>
#include <phon/utils/matrix.hpp>

namespace phonometrica {

Spectrogram::Spectrogram(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent) :
	SpeechPlot(rt, std::move(data), parent)
{
	try
	{
		readSettings();
	}
	catch (std::exception &)
	{
		run_script(rt, reset_spectrogram_settings);
		readSettings();
	}
}

void Spectrogram::drawYAxis(QWidget *y_axis, int y1, int y2)
{
	QPainter painter(y_axis);
	auto font_metrics = painter.fontMetrics();
	auto top = QString::number(long(ceiling_freq));
	QString bottom = QString::number(0);
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

		// Min and max can only be equal if we have zeros, in which case we don't fill the image.
		if (min_dB != max_dB)
		{
			//qDebug() << "max = " << max_dB << " min = " << min_dB;
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
	// FIXME: Praat uses a Gaussian-like window which is twice as long as a regular window, but using a Gaussian window
	//  similar to MATLAB's gives poorer results than a Hann a window.
	//nframe *= 2;
	int nfft = 256;
	while (nfft < nframe) nfft *= 2;

	auto half_nfft = nfft / 2;

	std::vector<double>amplitude(half_nfft, 0.0);
	intptr_t w = this->width();
	intptr_t h = this->height();

	// An m x n matrix, where m represents the number of horizontal pixels and n represents the number of vertical pixels.
	// Each horizontal pixel/point represents the center of an analysis window.
	Matrix<double> raster(w, h);
	raster.setZero(w, h);

	auto win = create_window(nframe, nfft, window_type);

	// Get audio data. If possible, we try to get a bit more data before and after the window so that we can calculate
	// frames at the edge.
	double total_duration = m_data->duration();
//	double start_time = (std::max<double>)(0.0, window_start - window_length / 2);
//	double end_time = (std::min<double>)(total_duration, window_end + window_length / 2);
	auto first_sample = m_data->time_to_frame(window_start);
	auto last_sample = m_data->time_to_frame(window_end);
	auto sample_count = last_sample - first_sample + 1;

	// The waveform splits samples equally among all the horizontal pixels except the last one, and assign left over
	// samples to the last pixel. We follow the same logic here, and for each pixel, we calculate an analysis window
	// centered on the middle sample of the waveform.
	if (sample_count < w) return raster; // white image.

	// Subtract 1 to width so that the last pixel is assigned the left-over frames.
	auto samples_per_pixel = sample_count / (w - 1);
	auto leftover_samples = sample_count % samples_per_pixel;

	// Adjust first and last samples.
	first_sample = first_sample + (samples_per_pixel / 2) - (nframe / 2);
	last_sample = first_sample + (samples_per_pixel * (w - 1)) + (samples_per_pixel / 2) + (nframe / 2);

	if (first_sample < 1)
	{
		first_sample = 1;
	}
	auto total_sample_count = m_data->size();
	if (last_sample > total_sample_count)
	{
		last_sample = total_sample_count;
	}

	// Weight power.
	double weight = 0;
	for (double x : win) weight += x * x;
	double k1 = 1 / (sample_rate * weight); // at DC and Nyquist frequencies.
	double k2 = 2 / (sample_rate * weight); // at other frequencies

	ffts_plan_t *plan = ffts_init_1d(nfft, FFTS_FORWARD);
	std::vector<std::complex<float>> input(nfft, std::complex<float>(0, 0));
	std::vector<std::complex<float>> output(nfft, std::complex<float>(0, 0));

	Array<double> buffer = m_data->get(first_sample, last_sample);
	pre_emphasis(buffer, sample_rate, preemph_threshold);

	intptr_t left_offset = samples_per_pixel / 2 - nframe / 2;

//	qDebug() << "Computing spectrogram*****************";
//	qDebug() << "width in pixels: " << w;
//	qDebug() << "window start: " << window_start;
//	qDebug() << "window end: " << window_end;
//	qDebug() << "samples per pixel: " << samples_per_pixel;
//	qDebug() << "window size (in samples): " << nframe;
//	qDebug() << "NFFT: " << nfft;
//	qDebug() << "first sample: " << first_sample;
//	qDebug() << "last sample: " << last_sample << "\n";

	for (intptr_t x = 0; x < w; x++)
	{
		auto from_sample = x * samples_per_pixel + left_offset;
		auto to_sample = from_sample + nframe;
		auto it = buffer.begin() + from_sample;

		if (from_sample < 0 || to_sample > last_sample)
		{
			//qDebug() << "white column at x = " << x << "(from_sample = " << from_sample << ", to_sample = " << to_sample << ")";

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
			auto sample = float((*it++) * win[j + 1]);
			input[j] = {sample, 0 };
		}
		for (intptr_t j = nframe; j < nfft; j++)
		{
			input[j] = {0, 0};
		}

		ffts_execute(plan, input.data(), output.data());

		for (size_t y = 0; y < half_nfft; y++)
		{
			double a = output[y].real() * output[y].real() + output[y].imag() * output[y].imag();
			assert(std::isfinite(a));
			double k = (y == 0 || y == half_nfft - 1) ? k1 : k2;
			a = k * a / nfft;
			constexpr double Iref = 4.0e-10;
			double dB = 10 * log10(a / Iref);
			amplitude[y] = dB;
		}

		// Create raster: frequencies on the y axis are mapped to the closest frequency bin.
		double ceiling_bin = ceiling_freq * half_nfft / nyquist_frequency;
		for (intptr_t y = 1; y <= h; y++)
		{
			double freq = double(y * ceiling_bin) / (h-1);
			int bin = int(freq);

			if (bin == half_nfft)
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
	emptyCache();
	QWidget::resizeEvent(e);
}

void Spectrogram::readSettings()
{
	using namespace speech;

	String cat("spectrogram");
	ceiling_freq = Settings::get_number(rt, cat, "frequency_range");
	double nyquist = double(m_data->sample_rate()) / 2;
	ceiling_freq = (std::min<double>)(ceiling_freq, nyquist);
	window_length = Settings::get_number(rt, cat, "window_size");
	dynamic_range = (int) Settings::get_number(rt, cat, "dynamic_range");
	preemph_threshold = Settings::get_number(rt, cat, "preemphasis_threshold");

	String win = Settings::get_string(rt, cat, "window_type");
	if (win == "Bartlett")
		window_type = WindowType::Bartlett;
	else if (win == "Blackman")
		window_type = WindowType::Blackman;
	else if (win == "Gaussian")
		window_type = WindowType::Gaussian;
	else if (win == "Hamming")
		window_type = WindowType::Hamming;
	else if (win == "Hann")
		window_type = WindowType::Hann;
	else if (win == "Rectangular")
		window_type = WindowType::Rectangular;
	else
	{
		QMessageBox::warning(this, tr("Invalid window type"), tr("Unknown window type. Using Hann window instead."));
		Settings::set_value(rt, cat, "window_type", "Hann");
		window_type = WindowType::Hann;
	}
}

void Spectrogram::emptyCache()
{
	image = QImage();
}

} // namespace phonometrica
