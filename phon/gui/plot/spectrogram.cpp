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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <fftw3.h>
#include <wx/dcmemory.h>
#include <wx/rawbmp.h>
#include <wx/msgdlg.h>
#include <phon/gui/plot/spectrogram.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

Spectrogram::Spectrogram(wxWindow *parent, const Handle<Sound> &snd) : SoundPlot(parent, snd)
{
	try
	{
		ReadSettings();
	}
	catch (std::exception &)
	{
		Settings::reset_spectrogram();
		ReadSettings();
	}

	Bind(wxEVT_PAINT, &Spectrogram::OnPaint, this);
}

void Spectrogram::InvalidateCache()
{
	m_cached_size = wxDefaultSize;
}

void Spectrogram::UpdateCache()
{
	if (GetSize() == m_cached_size) {
		return;
	}

	auto raster = ComputeSpectrogram();

	wxBitmap bmp(GetSize());
	wxNativePixelData data(bmp);


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
		// Adjust minimum to fit the dynamic range. All values lower than max - dynamic_range will be white.
		min_dB = (std::max)(min_dB, max_dB - dynamic_range);
		auto px = data.GetPixels();

		// The image needs to be reversed
		for (int j = raster.cols(); j-- > 0; )
		{
			auto row_start = px;
			for (int i = 0; i < raster.rows(); i++)
			{

				// shade of gray
				double value = (std::max)(raster(i, j), min_dB);
				if (std::isnan(value)) value = min_dB; // handle data that could not be calculated.
				int g = 255 - round((value - min_dB) * 255 / (max_dB - min_dB));
				assert(g >= 0);
				px.Red() = px.Green() = px.Blue() = g;
				++px;
			}
			px = row_start;
			px.OffsetY(data, 1);
		}
	}
	m_cached_bmp = bmp;

	if (show_formants)
	{
		EstimateFormants();
	}
}

void Spectrogram::DrawYAxis(wxPaintDC &dc, const wxRect &rect)
{
	auto top = wxString::Format("%d Hz", int(max_freq));
	wxString center("0");
	wxString bottom("0 Hz");
	wxCoord w, h;
	int padding = 2;
	dc.GetTextExtent(top, &w, &h);
    int x1 = rect.width - w - padding;
    dc.DrawText(top, x1, rect.y);

    dc.GetTextExtent(bottom, &w, &h);
    int x3 = rect.width - w - padding;
    int y3 = rect.y + rect.height - h;
    dc.DrawText(bottom, x3, y3);
}

void Spectrogram::ReadSettings()
{
	ReadSpectrogramSettings();
	ReadFormantSettings();
}

void Spectrogram::ReadSpectrogramSettings()
{
	using namespace speech;

	String category("spectrogram");
	max_freq = Settings::get_number(category, "frequency_range");
	double nyquist = double(m_sound->sample_rate()) / 2;
	max_freq = (std::min<double>)(max_freq, nyquist);
	spectrum_window_length = Settings::get_number(category, "window_size");
	dynamic_range = (int) Settings::get_number(category, "dynamic_range");
	preemph_threshold = Settings::get_number(category, "preemphasis_threshold");

	String win = Settings::get_string(category, "window_type");
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
		wxMessageBox(_("Unknown window type. Using Hann window instead."), _("Invalid window type"), wxICON_WARNING);
		Settings::set_value(category, "window_type", "Hann");
		window_type = WindowType::Hann;
	}
}

void Spectrogram::ReadFormantSettings()
{
	String category("formants");
	nformant = (int) Settings::get_number(category, "number_of_formants");
	formant_window_length = Settings::get_number(category, "window_size");
	lpc_order = (int) Settings::get_number(category, "lpc_order");
	max_formant_frequency = Settings::get_number(category, "max_frequency");
	max_formant_bandwidth = Settings::get_number(category, "max_bandwidth");
}

void Spectrogram::OnPaint(wxPaintEvent &)
{
	UpdateCache();
	wxPaintDC dc(this);
	Render(dc);
}

void Spectrogram::Render(wxPaintDC &dc)
{
	dc.DrawBitmap(m_cached_bmp, 0.0, 0.0, true);
	DrawSelection(dc);
	DrawCursor(dc);
	DrawTimeTick(dc);
	// The Y axis is repainted before the plots, so we need to explicitly update it on repaint events,
	// otherwise the magnitude values might be incorrect.
	y_axis_modified();
}

Matrix<double> Spectrogram::ComputeSpectrogram()
{
	using namespace speech;

	auto sample_rate = m_sound->sample_rate();
	auto nyquist_frequency = double(sample_rate) / 2;
	auto nframe = int(sample_rate * spectrum_window_length);
	if (nframe % 2 == 1) nframe++;
	// FIXME: Praat uses a Gaussian-like window which is twice as long as a regular window.
	if (window_type == WindowType::Gaussian) {
		nframe *= 2;
	}
	int nfft = 256;
	while (nfft < nframe) nfft *= 2;

	auto half_nfft = nfft / 2;

	std::vector<double>amplitude(half_nfft, 0.0);
	intptr_t w = GetSize().GetWidth();
	intptr_t h = GetSize().GetHeight();

	// An m x n matrix, where m represents the number of horizontal pixels and n represents the number of vertical pixels.
	// Each horizontal pixel/point represents the center of an analysis window.
	Matrix<double> raster(w, h);
	raster.setZero(w, h);

	auto win = create_window(nframe, nfft, window_type);

	// Get audio data. If possible, we try to get a bit more data before and after the window so that we can calculate
	// frames at the edge.
	auto first_sample = m_sound->time_to_frame(m_window.first);
	auto last_sample = m_sound->time_to_frame(m_window.second);
	auto sample_count = last_sample - first_sample + 1;

	// The waveform splits samples equally among all the horizontal pixels except the last one, and assign left over
	// samples to the last pixel. We follow the same logic here, and for each pixel, we calculate an analysis window
	// centered on the middle sample of the waveform.
	if (sample_count < w) return raster; // white image.

	// Subtract 1 to width so that the last pixel is assigned the left-over frames.
	auto samples_per_pixel = sample_count / (w - 1);

	// Adjust first and last samples.
	first_sample = first_sample + (samples_per_pixel / 2) - (nframe / 2);
	last_sample = first_sample + (samples_per_pixel * (w - 1)) + (samples_per_pixel / 2) + (nframe / 2);

	if (first_sample < 0)
	{
		first_sample = 0;
	}
	auto total_sample_count = m_sound->size();
	if (last_sample >= total_sample_count)
	{
		last_sample = total_sample_count - 1;
	}

	// Weight power.
	double weight = 0;
	for (double x : win) weight += x * x;
	double k1 = 1 / (sample_rate * weight); // at DC and Nyquist frequencies.
	double k2 = 2 / (sample_rate * weight); // at other frequencies

	std::vector<double> input(nfft, 0.0);
	std::vector<std::complex<double>> output(nfft, std::complex<double>(0, 0));
	fftw_plan plan = fftw_plan_dft_r2c_1d(nfft, input.data(), (fftw_complex*)output.data(), FFTW_ESTIMATE);

	auto buffer = m_sound->average_channels(first_sample, last_sample);
	pre_emphasis(buffer, sample_rate, preemph_threshold);

	intptr_t left_offset = samples_per_pixel / 2 - nframe / 2;

	for (intptr_t x = 0; x < w; x++)
	{
		auto from_sample = x * samples_per_pixel + left_offset;
		auto to_sample = from_sample + nframe;
		auto it = buffer.begin() + from_sample;

		if (from_sample < 0 || to_sample > last_sample)
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
			auto sample = float((*it++) * win[j + 1]);
			input[j] = sample;
		}
		for (intptr_t j = nframe; j < nfft; j++)
		{
			input[j] = 0;
		}

		fftw_execute(plan);

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
		double ceiling_bin = max_freq * half_nfft / nyquist_frequency;
		for (intptr_t y = 1; y <= h; y++)
		{
			double freq = double(y * ceiling_bin) / (h-1.0);
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

	fftw_free(plan);
	m_cached_size = GetSize();

	return raster;
}

void Spectrogram::EstimateFormants()
{

}

} // namespace phonometrica
