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
 * Created: 19/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/wave_bar.hpp>

namespace phonometrica {

WaveBar::WaveBar(wxWindow *parent, const Handle <Sound> &snd) : wxWindow(parent, wxID_ANY),
	m_sound(snd), m_sel({-1, -1})
{
	SetMinSize(wxSize(-1, 50));
	SetMaxSize(wxSize(-1, 50));

    auto raw_data = m_sound->data().data();
    auto sample_count = m_sound->size();

    for (intptr_t i = 0; i < sample_count; i++)
    {
        double m = std::abs(raw_data[i]);

        if (m > raw_magnitude) {
	        raw_magnitude = m;
        }
    }

    Bind(wxEVT_ERASE_BACKGROUND, &WaveBar::OnEraseBackground, this);
	Bind(wxEVT_PAINT, &WaveBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &WaveBar::OnStartSelection, this);
    Bind(wxEVT_LEFT_UP, &WaveBar::OnEndSelection, this);
    Bind(wxEVT_MOTION, &WaveBar::OnMotion, this);
   	Bind(wxEVT_MOUSEWHEEL, &WaveBar::OnMouseWheel, this);
	Bind(wxEVT_LEAVE_WINDOW, &WaveBar::OnLeaveWindow, this);
}

void WaveBar::OnEraseBackground(wxEraseEvent &)
{

}

void WaveBar::OnPaint(wxPaintEvent &)
{
	wxBufferedPaintDC dc(this);
	dc.SetBackground(*wxWHITE);
	dc.Clear();
	Render(dc);
}

void WaveBar::Render(wxBufferedPaintDC &dc)
{
	UpdateCache();

	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	auto height = GetSize().GetHeight();
	auto width = GetSize().GetWidth();

	// Draw zero-crossing line.
	dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_DOT));
	dc.DrawLine(0, height/2, width, height/2);

	// Draw wave
	gc->SetPen(wxPen(*wxBLACK, 1));
	wxGraphicsPath path = gc->CreatePath();

	if (m_cache.size() == size_t(width * 2))
	{
		path.MoveToPoint(0.0, m_cache[0]);
		path.AddLineToPoint(0.0, m_cache[1]);

		for (size_t i = 2; i < m_cache.size(); i += 2)
		{
			auto x = double(i) / 2;
			path.AddLineToPoint(x, m_cache[i]);
			path.AddLineToPoint(x, m_cache[i + 1]);
		}
	}
	else
	{
		auto offset = double(width) / m_cache.size();
		path.MoveToPoint(0.0, m_cache[0]);

		for (size_t i = 1; i < m_cache.size(); ++i)
		{
			auto x = double(i) * offset;
			path.AddLineToPoint(x, m_cache[i]);
		}
	}

	gc->DrawPath(path);

	if (!HasSelection()) {
		return;
	}
	path.MoveToPoint(m_sel.first, 0.0);
	path.AddLineToPoint(m_sel.second, 0.0);
	path.AddLineToPoint(m_sel.second, height);
	path.AddLineToPoint(m_sel.first, height);
	path.AddLineToPoint(m_sel.first, 0.0);
	wxBrush brush;
	brush.SetColour(WAVEBAR_SEL_COLOUR);
	gc->SetBrush(brush);
	gc->FillPath(path);
}

void WaveBar::UpdateCache()
{
	auto width = GetSize().GetWidth();

	if (m_cached_width == width) {
		return;
	}

	auto sample_count = m_sound->channel_size();
	auto &data = m_sound->data();
	int nchannel = m_sound->nchannel();

	// Same algorithm as for Waveform.
	if (sample_count >= width * 2)
	{
		std::vector<double> wave(width * 2, 0.0);
		auto current_point = wave.begin();
		// Frames per pixel
		auto offset = double(sample_count) / width;

		for (int i = 0; i < width; i++)
		{
			auto x1 = intptr_t(floor(i * offset)) + 1;
			auto x2 = intptr_t(ceil((i+1) * offset)) + 1;
			if (x2 > sample_count) {
				x2 = sample_count;
			}

			// Get average value for each sample
			double sample = 0;
			for (intptr_t j = 1; j <= nchannel; j++) {
				sample += data(x1,j);
			}
			sample /= nchannel;

			// Read first sample.
			auto maximum = sample;
			auto minimum = maximum;

			for (intptr_t x = x1+1; x <= x2; x++)
			{
				// Get average value for each sample
				double sample = 0;
				for (intptr_t j = 1; j <= nchannel; j++) {
					sample += data(x,j);
				}
				sample /= nchannel;

				if (sample < minimum) {
					minimum = sample;
				}
				else if (sample > maximum) {
					maximum = sample;
				}
			}

			double y1 = SampleToYPos(maximum);
			double y2 = SampleToYPos(minimum);
			*current_point++ = y1;
			*current_point++ = y2;
		}
		assert(current_point == wave.end());
		m_cache = std::move(wave);
	}
	else
	{
		// Draw all the points
		std::vector<double> wave(data.size(), 0.0);
		auto it = wave.begin();

		for (auto sample : data) {
			*it++ = SampleToYPos(sample);
		}

		m_cache = std::move(wave);
	}
	m_cached_width = width;
}

double WaveBar::TimeToXPos(double t) const
{
    return t * GetSize().GetWidth() / m_sound->duration();
}

double WaveBar::XPosToTime(double x) const
{
    return x * m_sound->duration() / GetSize().GetWidth();
}

double WaveBar::SampleToYPos(double s) const
{
    const int H = GetSize().GetHeight()/ 2;
    return H - s * (double(H) / raw_magnitude);
}

void WaveBar::SetSelection(PixelSelection sel)
{
	m_sel = sel;
	selection_changed(sel);
}

void WaveBar::OnStartSelection(wxMouseEvent &e)
{
	auto pos = e.GetPosition();
	m_sel_start = pos.x;
	e.Skip();
}

void WaveBar::OnEndSelection(wxMouseEvent &e)
{
	m_sel_start = -1;
	auto t1 = XPosToTime(m_sel.first);
	auto t2 = XPosToTime(m_sel.second);
	change_window(TimeWindow{t1, t2});
	e.Skip();
}

void WaveBar::OnMotion(wxMouseEvent &e)
{
	auto pos = e.GetPosition();

	if (m_sel_start >= 0)
	{
		double x = (std::max)(pos.x, 0);
		x = (std::min)(x, (double)GetSize().GetWidth());

		if (x < m_sel_start) {
			SetSelection({x, m_sel_start});
		}
		else {
			SetSelection({m_sel_start, x});
		}
		Refresh();
	}

	auto t = XPosToTime(pos.x);

	// Make sure we don't show an absurd time when the user drags the cursor outside of the widget.
	if (t >= 0 && t <= m_sound->duration())
	{
		auto msg = wxString::Format(_("Time at cursor: %f s"), t);
		update_status(msg);
	}
}

void WaveBar::SetTimeSelection(TimeWindow win)
{
	auto from = TimeToXPos(win.first);
	auto to = TimeToXPos(win.second);
	SetSelection(PixelSelection{from, to});
	Refresh();
}

void WaveBar::OnMouseWheel(wxMouseEvent &e)
{
	auto steps = e.GetWheelRotation() / e.GetWheelDelta();
	auto count = std::abs(steps);

	for (int i = 1; i <= count; i++)
	{
		if (steps > 0) {
			MoveBackward();
		}
		else {
			MoveForward();
		}
	}
}

void WaveBar::MoveForward()
{
	auto width = GetSize().GetWidth();
	if (m_sel.second == width) {
		return;
	}
	auto delta = (m_sel.second - m_sel.first) * 0.25; // shift by 25%
	auto x1 = m_sel.first + delta;
	auto x2 = (std::min<double>)(m_sel.second + delta, width);
	auto t1 = XPosToTime(x1);
	auto t2 = XPosToTime(x2);
	SetSelection({x1, x2});
	Refresh();
	change_window(TimeWindow{t1, t2});
}

void WaveBar::MoveBackward()
{
	if (m_sel.first == 0) {
		return;
	}
	auto delta = (m_sel.second - m_sel.first) * 0.25; // shift by 25%
	auto x1 = (std::max)(0.0, m_sel.first - delta);
	auto x2 = m_sel.second - delta;
	auto t1 = XPosToTime(x1);
	auto t2 = XPosToTime(x2);
	SetSelection({x1, x2});
	Refresh();
	change_window(TimeWindow{t1, t2});
}

double WaveBar::GetMagnitude() const
{
	return raw_magnitude;
}

void WaveBar::OnLeaveWindow(wxMouseEvent &e)
{
	update_status(wxString());
}

} // namespace phonometrica
