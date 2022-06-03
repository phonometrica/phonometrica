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
#include <phon/application/resampler.hpp>
#include <phon/analysis/speech_utils.hpp>

namespace phonometrica {

Spectrogram::Spectrogram(wxWindow *parent, const Handle <Sound> &snd, int channel) : SoundPlot(parent, snd, channel)
{
	try
	{
		ReadSettings();
	}
	catch (std::exception &)
	{
		Settings::reset_spectrogram();
		Settings::reset_formants();
		ReadSettings();
	}

	timer.Bind(wxEVT_TIMER, &Spectrogram::OnTimer, this);
}

void Spectrogram::UpdateCache()
{
    // FIXME: On Windows, we must open a new scope to make sure that the cached bitmap is not shared; otherwise we will
    //  get an assertion failure "can't copy bitmap locked for raw access!" in wxMemoryDC::SelectObject().
	{
		auto raster = ComputeSpectrogram();
		wxBitmap bmp(GetSize());
		wxNativePixelData data(bmp);
		// We can't use Eigen's maxCoeff()/minCoeff() because we have nan's in the matrix.
		double max_dB = -100000;
		double min_dB = 100000;

		for (int i = 0; i < raster.rows(); i++) {
			for (int j = 0; j < raster.cols(); j++) {
				double val = raster(i, j);

				if (std::isfinite(val)) {
					if (val > max_dB) max_dB = val;
					if (val < min_dB) min_dB = val;
				}
			}
		}

		// Min and max can only be equal if we have zeros, in which case we don't fill the image.
		if (min_dB != max_dB) {
			// Adjust minimum to fit the dynamic range. All values lower than max - dynamic_range will be white.
			min_dB = (std::max)(min_dB, max_dB - dynamic_range);
			auto px = data.GetPixels();

			// The image needs to be reversed
			for (int j = raster.cols(); j-- > 0;) {
				auto row_start = px;
				for (int i = 0; i < raster.rows(); i++) {

					// shade of gray
					double value = (std::max)(raster(i, j), min_dB);
					if (std::isnan(value)) value = min_dB; // handle data that could not be calculated.
					int g = 255 - round((value - min_dB) * 255 / (max_dB - min_dB));
					if (unlikely(g < 0))
					{
						PHON_LOG("Invalid gray value in spectrogram: %d\n", g);
						g = min_dB;
					}

					px.Red() = px.Green() = px.Blue() = g;
					++px;
				}
				px = row_start;
				px.OffsetY(data, 1);
			}
		}
		m_cached_bmp = bmp;
		assert(m_cached_bmp.IsOk());
	}

	if (show_formants && !formant_error)
	{
		try
		{
			EstimateFormants();
			assert(m_cached_bmp.IsOk());
			DrawFormants();
			assert(m_cached_bmp.IsOk());
		}
		catch (std::exception &e)
		{
			wxMemoryDC dc;
			dc.SelectObject(m_cached_bmp);
			auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
			if (!gc) return;
			wxString formant_message = e.what();
			dc.SetTextForeground(*wxRED);
			auto font = dc.GetFont();
			font.MakeLarger();
			dc.SetFont(font);
			auto sz = dc.GetTextExtent(formant_message);
			auto x = (GetWidth() - sz.x) / 2;
			auto y = (GetHeight() - sz.y) / 2;
			wxPoint origin(x, y);
			dc.DrawText(formant_message, origin);
			timer.StartOnce(2000);
			dc.SelectObject(wxNullBitmap);
		}
	}
}

void Spectrogram::DrawYAxis(PaintDC &dc, const wxRect &rect)
{
	auto top = wxString::Format("%d Hz", int(max_freq));
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
	time_step = Settings::get_number(category, "time_step");
}

Matrix<double> Spectrogram::ComputeSpectrogram()
{
	using namespace speech;

	auto sample_rate = m_sound->sample_rate();
	auto nyquist_frequency = double(sample_rate) / 2;
	auto analysis_window_duration = spectrum_window_length;
	// Praat uses a Gaussian-like window which is twice as long as a regular window.
	if (window_type == WindowType::Gaussian) {
		analysis_window_duration *= 2;
	}
	auto nframe = int(ceil(sample_rate * analysis_window_duration));
	if (nframe % 2 == 1) {
		nframe++;
	}
	int nfft = 256;
	while (nfft < nframe) nfft *= 2;
	auto half_nfft = nfft / 2;

	std::vector<double>amplitude(half_nfft, 0.0);
	intptr_t w = GetWidth();
	intptr_t h = GetHeight();

	// Get audio data. We will try to get a bit more data before and after the window so that we can calculate
	// frames at the edge.
	auto slice_duration = GetWindowDuration() / w;
	auto offset = (slice_duration - analysis_window_duration) / 2;
	auto first_sample = m_sound->time_to_frame(m_window.first + offset);
	auto last_sample = m_sound->time_to_frame(m_window.first + (w-1) * slice_duration + offset) + nframe + 1;
	if (first_sample < 1)
	{
		first_sample = 1;
	}
	auto total_sample_count = m_sound->channel_size();
	if (last_sample > total_sample_count)
	{
		last_sample = total_sample_count;
	}

	// An m x n matrix, where m represents the number of horizontal pixels and n represents the number of vertical pixels.
	// Each horizontal pixel/point represents the center of an analysis window.
	Matrix<double> raster(w, h);
	raster.setZero(w, h);

	auto win = create_window(nframe, nfft, window_type);

	// Weight power.
	double weight = 0;
	for (double x : win) weight += x * x;
	double k1 = 1 / (sample_rate * weight); // at DC and Nyquist frequencies.
	double k2 = 2 / (sample_rate * weight); // at other frequencies

	std::vector<double> input(nfft, 0.0);
	std::vector<std::complex<double>> output(nfft, std::complex<double>(0, 0));
	fftw_plan plan = fftw_plan_dft_r2c_1d(nfft, input.data(), (fftw_complex*)output.data(), FFTW_ESTIMATE);

	auto data = m_sound->get_channel(m_channel, first_sample, last_sample);
	pre_emphasis(data, sample_rate, preemph_threshold);


	for (intptr_t x = 0; x < w; x++)
	{
		auto t = m_window.first + x * slice_duration + offset;
		auto from_sample = m_sound->time_to_frame(t) - first_sample;
		auto to_sample = from_sample + nframe;
		auto it = data.begin() + from_sample;

		if (from_sample < 0 || to_sample >= data.size())
		{
			// Can't calculate FFT because we would get outside of the bounds.
			// Display a white vertical column and move to the next time point.
			for (int j = 0; j < h; j++)
			{
				raster(x, j) = std::nan(""); // it will be converted to the minimum intensity when it is displayed.
			}
//			PHON_LOG("pixel %d, from = %d, to = %d [SKIPPED]\n", int(x), int(from_sample), int(to_sample));
			continue;
		}
		else
		{
//			PHON_LOG("pixel %d, from = %d, to = %d\n", int(x), int(from_sample), int(to_sample));
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

		for (size_t y = 0; y < (size_t)half_nfft; y++)
		{
			double a = output[y].real() * output[y].real() + output[y].imag() * output[y].imag();
			assert(std::isfinite(a));
			double k = (y == 0 || y == (size_t)half_nfft - 1) ? k1 : k2;
			a = k * a / nfft;
			constexpr double Iref = 4.0e-10;
			// Intensity is undefined if the raw amplitude is equal to 0.
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
				// amp might be nan if the raw amplitude is 0
				raster(x, y - 1) = amp;
			}
		}
	}

//	PHON_LOG("------------------------------- SPECTROGRAM\n");
//	PHON_LOG("window: %f, start: %f, end: %f\n", GetWindowDuration(), m_window.first, m_window.second);
//	PHON_LOG("slice duration: %f\n", slice_duration);
//	PHON_LOG("offset: %f\n", offset);
//	PHON_LOG("first sample: %d\n", (int)first_sample);
//	PHON_LOG("last sample: %d\n", (int)last_sample);
//	PHON_LOG("data size: %d\n", (int)data.size());
//	PHON_LOG("width: %d\n", int(w));
//	PHON_LOG("nframe: %d, nfft: %d\n", nframe, nfft);

	fftw_free(plan);
	m_cached_size = GetSize();

	return raster;
}

void Spectrogram::EstimateFormants()
{
	using namespace speech;

	// At least 2 measurements per window
	if (GetWindowDuration() <= 2 * time_step) {
		throw error("Zoom out to see formants");
	}
	// At most 2 measurements per pixel
	if (GetWindowDuration() / time_step > GetWidth() * 2) {
		throw error("Zoom in to see formants");
	}

	auto npoint = int(ceil((GetWindowDuration() - time_step) / time_step));
	formants = Matrix<double>(npoint, nformant);
	Matrix<double> bandwidths(npoint, nformant);
	formants.setZero(npoint, nformant);
	bandwidths.setZero(npoint, nformant);

	// Window length will be multiplied by 2 because of the Gaussian window,
	// so 'formant_window_length' is effectively half a window.
	auto start_time = m_window.first + time_step - formant_window_length;
	auto end_time = m_window.first + time_step * (npoint + 1) + formant_window_length;
	if (start_time < 0.0) start_time = 0.0;
	if (end_time > GetSoundDuration()) end_time = GetSoundDuration();
	auto start_frame = m_sound->time_to_frame(start_time);
	auto end_frame = m_sound->time_to_frame(end_time);
	auto data = m_sound->get_channel(m_channel, start_frame, end_frame);
	double Fs = max_formant_frequency * 2;

	if (Fs != m_sound->sample_rate()) {
		data = resample(data, m_sound->sample_rate(), Fs);
	}
	// Apply pre-emphasis from 50 Hz.
	pre_emphasis(data, Fs, 50);

	auto nframe = int(ceil(formant_window_length * Fs)) * 2; // x 2 for Gaussian window
	auto win = create_window(nframe, nframe, WindowType::Gaussian);
	Array<double> buffer(nframe, 0.0);
	auto len = data.size();
	auto t = m_window.first;

//	PHON_LOG("-------------------------------\n");
//	PHON_LOG("first sample: %d\n", (int)start_frame);
//	PHON_LOG("last sample: %d\n", (int)end_frame);

	for (int i = 0; i < npoint; i++)
	{
		t += time_step;

		auto from_sample = intptr_t(round((t - formant_window_length - start_time) * Fs));
		auto to_sample = from_sample + nframe;

		// Don't estimate formants at the edge if we can't fill a window.
		if (from_sample < 0 || to_sample >= len)
		{
			for (int j = 0; j < nformant; j++) {
				formants(i,j) = std::nan("");
			}
//			PHON_LOG("formants: pixel = %d, time = %f, start = %d, end = %d [SKIPPED]\n", i, t, int(from_sample), int(to_sample));
			continue;
		}
		else {
//			PHON_LOG("formants: pixel = %d, time = %f, start = %d, end = %d\n", i, t, int(from_sample), int(to_sample));
		}
		// Apply window.
		auto it = data.begin() + from_sample;
		for (int j = 1; j <= nframe; j++)
		{
			buffer[j] = *it++ * win[j];
		}

		auto coeffs = get_lpc_coefficients(buffer, lpc_order);
		std::vector<double> freqs, bw;
		bool ok = get_formants(coeffs, Fs, freqs, bw);

		if (!ok)
		{
			for (int j = 0; j < formants.cols(); j++) {
				formants(i, j) = std::nan("");
			}
			continue;
		}

		int count = 0;
		const double lowest_freq = 50.0;
		const double highest_frequency = Fs / 2 - lowest_freq;
		for (int k = 0; k < (int)freqs.size(); k++)
		{
			auto freq = freqs[k];
			if (freq > lowest_freq && freq < highest_frequency)
			{
				formants(i, count) = freq;
				bandwidths(i, count++) = bw[k];
			}
			if (count == nformant) break;
		}
		for (int k = count; k < nformant; k++)
		{
			formants(i, k) = std::nan("");
			bandwidths(i, k) = std::nan("");
		}
	}

#if 0
	auto slice_duration = GetWindowDuration() / width;
	auto time_offset = slice_duration / 2 - formant_window_length / 2;
	auto offset = int(ceil(time_offset * m_sound->sample_rate()));

	auto first_sample = m_sound->time_to_frame(m_window.first) + offset;
	auto last_sample = m_sound->time_to_frame(m_window.second) - offset;
	if (first_sample < 1)
	{
		first_sample = 1;
	}
	auto total_sample_count = m_sound->channel_size();
	if (last_sample > total_sample_count)
	{
		last_sample = total_sample_count;
	}

	PHON_LOG("----------------------------\n");
	PHON_LOG("original data: samples = %d, duration = %f\n", int(data.size()), double(data.size())/m_sound->sample_rate());



	// Calculate LPC at each time point.
	auto len = data.size();

	PHON_LOG("nframe = %d\n", int(nframe));
	PHON_LOG("window duration: %f\n", GetWindowDuration());
	PHON_LOG("time offset: %f\n", time_offset);
	PHON_LOG("data: samples = %d, duration = %f\n", int(data.size()), double(data.size())/Fs);
	PHON_LOG("----------------------------\n");

	for (int i = 0; i < width; i++)
	{
		auto t = i * slice_duration;
		auto start_frame = intptr_t(floor(t * Fs));
		auto end_frame = start_frame + nframe;

#endif
}

bool Spectrogram::HasFormants() const
{
	return show_formants;
}

void Spectrogram::ShowFormants(bool value)
{
	show_formants = value;
	InvalidateCache();
	Refresh();
}

void Spectrogram::DrawFormants()
{
	wxMemoryDC dc;
	dc.SelectObject(m_cached_bmp);
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	gc->SetPen(wxPen(*wxRED, 1));
	gc->SetBrush(wxBrush(*wxRED));
	auto t = m_window.first;

	for (int i = 0; i < formants.rows(); i++)
	{
		t += time_step;
		auto x = TimeToXPos(t);

		for (int j = 0; j < formants.cols(); j++)
		{
			auto f = formants(i, j);
			if (std::isnan(f)) continue;
			auto y = FormantToYPos(f);
			gc->DrawEllipse(x-1, y-1, 3, 3); // -1 to center ellipses.
		}
	}
	dc.SelectObject(wxNullBitmap);
}

int Spectrogram::FormantToYPos(double hz)
{
	auto h = GetHeight();
	return h - int(round((hz * h / max_freq)));
}

void Spectrogram::OnMotion(wxMouseEvent &e)
{
	SoundPlot::OnMotion(e);
	if (m_track_mouse)
	{
		auto f = round(YPosToHertz(e.GetPosition().y));
		auto msg = wxString::Format("Frequency at cursor: %d Hz", (int) f);
		update_status(msg);
	}
}

double Spectrogram::YPosToHertz(int y) const
{
	auto height = GetSize().GetHeight();
	return (max_freq * (height - y)) / height;
}

void Spectrogram::OnTimer(wxTimerEvent &event)
{
	// Force a redraw, but without formant tracking
	InvalidateCache();
	formant_error = true;
	Refresh();
}

void Spectrogram::InvalidateCache()
{
	SpeechWidget::InvalidateCache();
	formant_error = false;
	// Stop the timer to make sure we don't erase formants during fast scrolling.
	timer.Stop();
}

} // namespace phonometrica
