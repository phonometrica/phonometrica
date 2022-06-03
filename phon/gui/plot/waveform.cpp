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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/dcmemory.h>
#include <wx/dcgraph.h>
#include <wx/msgdlg.h>
#include <phon/gui/plot/waveform.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

Waveform::Waveform(wxWindow *parent, const Handle<Sound> &snd, int channel) :
		SoundPlot(parent, snd, channel)
{
	try
	{
		ReadSettings();
	}
	catch (...)
	{
		Settings::reset_waveform();
		ReadSettings();
	}
}

void Waveform::ReadSettings()
{
	String category("waveform");
	auto m = Settings::get_number(category, "magnitude");

	if (m <= 0 || m > 1.0)
	{
		wxMessageBox( _("Your waveform settings have an invalid magnitude and will be reinitialized."), _("Invalid settings"), wxICON_WARNING);
		throw std::runtime_error("");
	}
	SetMagnitude(m);

	String method = Settings::get_string(category, "scaling");

	if (method == "global")
	{
		scaling = Scaling::Global;
	}
	else if (method == "local")
	{
		scaling = Scaling::Local;
	}
	else if (method == "fixed")
	{
		scaling = Scaling::Fixed;
	}
	else
	{
		wxMessageBox(_("Your waveform settings have an invalid magnitude and will be reinitialized."), _("Invalid settings"), wxICON_WARNING);
		throw std::runtime_error("");
	}
}

void Waveform::UpdateCache()
{
	DrawBitmap();
	m_cached_size = GetSize();
}

double Waveform::SampleToHeight(double s) const
{
	const double H = (double)GetHeight() / 2;
    return H - s * H / magnitude;
}

void Waveform::SetMagnitude(double value)
{
    magnitude = value;
    extrema = {-value, value};
}

void Waveform::SetLocalMagnitude(std::span<const double> data)
{
	double e1 = std::abs(double(*std::max_element(data.begin(), data.end())));
	double e2 = std::abs(double(*std::min_element(data.begin(), data.end())));
	auto m = (std::max)(e1, e2);
	SetMagnitude(m);
}

void Waveform::SetGlobalMagnitude(double value)
{
	global_magnitude = value;
}

void Waveform::DrawBitmap()
{
	wxMemoryDC dc;
	wxBitmap bmp(GetSize());
	dc.SelectObject(bmp);
	dc.SetBackground(*wxWHITE);
	dc.Clear();
    auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	auto height = GetHeight();
	auto width = GetWidth();

	try
	{
		auto wave = ComputeWaveform();
		// Draw zero-crossing line.
		dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_DOT));
		dc.DrawLine(0, height / 2, width, height / 2);
		gc->SetPen(wxPen(*wxBLACK, 1));
		wxGraphicsPath path = gc->CreatePath();

		if (wave.size() == (size_t) width * 2)
		{
			path.MoveToPoint(0.0, wave[0]);
			path.AddLineToPoint(0.0, wave[1]);

			for (size_t i = 2; i < wave.size(); i += 2)
			{
				auto x = double(i) / 2;
				path.AddLineToPoint(x, wave[i]);
				path.AddLineToPoint(x, wave[i + 1]);
			}
		}
		else
		{
			auto offset = double(width) / wave.size();
			gc->SetBrush(wxBrush(*wxBLACK));
			path.MoveToPoint(0.0, wave.front());
			for (size_t i = 1; i < wave.size(); ++i)
			{
				auto x = double(i) * offset;
				path.AddLineToPoint(x, wave[i]);
			}

			if (wave.size() < (size_t)width)
			{
				// Try to draw a line to the sample past the end of the window to avoid having a broken line at the right edge.
				auto extra_frame = m_sound->time_to_frame(m_window.second) + 1;
				if (extra_frame <= m_sound->channel_size())
				{
					auto sample = m_sound->get_sample(m_channel, extra_frame);
					//if (sample < -magnitude) sample = -magnitude;
					auto x = double(wave.size()) * offset;
					auto y = SampleToHeight(sample);
					path.AddLineToPoint(x, y);
				}
			}

			if (wave.size() < (size_t)width/4)
			{
				// Draw individual samples.
				for (size_t i = 0; i < wave.size(); ++i)
				{
					auto x = double(i) * offset;
					gc->DrawEllipse(x-1, wave[i]-1, 3, 3);
				}
			}
		}

		gc->StrokePath(path);
	}
	catch (std::exception &e)
	{
		wxString msg = e.what();
		auto sz = dc.GetTextExtent(msg);
		auto x = double(GetWidth() - sz.x) / 2;
		auto y = double(GetHeight() - sz.y) / 2;
		gc->SetFont(gc->CreateFont(dc.GetFont()));
		gc->DrawText(msg, x, y);
	}

	dc.SelectObject(wxNullBitmap);
	m_cached_bmp = bmp;
	assert(m_cached_bmp.IsOk());
}

std::vector<double> Waveform::ComputeWaveform()
{
	auto first_sample = m_sound->time_to_frame(m_window.first);
	auto last_sample = m_sound->time_to_frame(m_window.second);
	if ((last_sample - first_sample) < 1) {
		throw error("zoom out to see waveform");
	}
	auto data = m_sound->get_channel(m_channel, first_sample, last_sample);
	auto sample_count = data.size();

	if (scaling == Scaling::Global)
	{
		SetMagnitude(global_magnitude);
	}
	else if (scaling == Scaling::Local)
	{
		SetLocalMagnitude(data);
	}
	auto width = GetWidth();
	assert(m_window.second <= m_sound->duration());

//	PHON_LOG("-----------------------------------\n");
//	PHON_LOG("window: %f to %f\n", m_window.first, m_window.second);
//	PHON_LOG("width: %d\n", width);
//	PHON_LOG("first sample: %ld\n", first_sample);
//	PHON_LOG("last sample: %ld\n", last_sample);
//	PHON_LOG("sample count: %ld\n", sample_count);

	if (sample_count >= width * 2)
	{
		// If the number of samples to display is greater than twice the number of pixels,
		// map several frames to one pixel. We find the maximum and minimum amplitudes and draw
		// a vertical line for this pixel. We allow one overlapping frame for each pixel to spread
		// frames uniformly across pixels.
		std::vector<double> wave(GetWidth() * 2, 0.0);
		auto current_point = wave.begin();

		// Frames per pixel
		auto offset = double(sample_count) / width;
//		PHON_LOG("frames per pixel: %f\n", offset);

		for (int i = 0; i < width; i++)
		{
			auto x1 = intptr_t(floor(i * offset));
			auto x2 = intptr_t(ceil((i+1) * offset));
			if (x2 > sample_count) {
				x2 = sample_count;
			}

			auto it = data.begin() + x1;
			auto limit = data.begin() + x2;

			// Read first sample.
			auto maximum = *it++;
			auto minimum = maximum;

			while (it != limit)
			{
				auto sample = *it++;

				if (sample < minimum) {
					minimum = sample;
				}
				else if (sample > maximum) {
					maximum = sample;
				}
			}

			double y1 = SampleToHeight(maximum);
			double y2 = SampleToHeight(minimum);
			*current_point++ = y1;
			*current_point++ = y2;
		}
		assert(current_point == wave.end());

		return wave;
	}
	else
	{
		// Draw all the points
		std::vector<double> wave(data.size(), 0.0);
		auto it = wave.begin();

		for (auto sample : data) {
			*it++ = SampleToHeight(sample);
		}

		return wave;
	}
}

void Waveform::DrawYAxis(PaintDC &dc, const wxRect &rect)
{
	auto top = wxString::Format("+%.4f", extrema.second);
	wxString center("0");
	auto bottom = wxString::Format("%.4f", extrema.first);
	wxCoord w, h;
	int padding = 2;
	dc.GetTextExtent(top, &w, &h);
    int x1 = rect.width - w - padding;
    dc.DrawText(top, x1, rect.y);

    dc.GetTextExtent(center, &w, &h);
    int x2 = rect.width - w - padding;
    int y2 = rect.y + (rect.height - h) / 2;
    dc.DrawText(center, x2, y2);

    dc.GetTextExtent(bottom, &w, &h);
    int x3 = rect.width - w - padding;
    int y3 = rect.y + rect.height - h;
    dc.DrawText(bottom, x3, y3);
}

void Waveform::OnMotion(wxMouseEvent &e)
{
	SoundPlot::OnMotion(e);
	if (m_track_mouse)
	{
		double amp = YPosToAmplitude(e.GetPosition().y);
		auto msg = wxString::Format("Amplitude at cursor: %.6f", amp);
		update_status(msg);
	}
}

double Waveform::YPosToAmplitude(int y) const
{
	const double H = double(GetSize().GetHeight()) / 2;
	return (H - y) * magnitude / H;
}

} // namespace phonometrica
