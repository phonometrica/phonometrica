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
	SetBackgroundColour(*wxWHITE);
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

	Bind(wxEVT_PAINT, &WaveBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &WaveBar::OnStartSelection, this);
    Bind(wxEVT_LEFT_UP, &WaveBar::OnEndSelection, this);
    Bind(wxEVT_MOTION, &WaveBar::OnMotion, this);
   	Bind(wxEVT_MOUSEWHEEL, &WaveBar::OnMouseWheel, this);
}

void WaveBar::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);
	Render(dc);
}

void WaveBar::Render(wxPaintDC &dc)
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
	path.MoveToPoint(0.0, m_cache[0].first);
	path.AddLineToPoint(0.0, m_cache[0].second);

	for (size_t x = 1; x < m_cache.size(); x++)
	{
		path.AddLineToPoint(x-1, m_cache[x].first);
		path.AddLineToPoint(x-1, m_cache[x].second);
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
	if (m_cache.size() == (size_t)GetSize().GetWidth()) {
		return;
	}
	m_cache.clear();

	auto sample_count = m_sound->channel_size();
	auto &data = m_sound->data();
	int nchannel = m_sound->nchannel();
	auto width = GetSize().GetWidth();

	// Subtract 1 to width so that the last pixel is assigned the left-over frames.
	auto frames_per_pixel = sample_count / (width-1);

	auto maximum = (std::numeric_limits<double>::min)();
	auto minimum = (std::numeric_limits<double>::max)();

	for (intptr_t i = 1; i <= sample_count; i++)
	{
		// Get average value for each sample
		double sample = 0;
		for (intptr_t j = 1; j <= nchannel; j++)
		{
			sample += data(i,j);
		}
		sample /= nchannel;

		// Find extrema
		if (sample < minimum)
			minimum = sample;
		if (sample > maximum)
			maximum = sample;

		if (i % frames_per_pixel == 0)
		{
			auto y1 = SampleToYPos(maximum);
			auto y2 = SampleToYPos(minimum);
			m_cache.emplace_back(y1, y2);
			// reset values
			maximum = (std::numeric_limits<double>::min)();
			minimum = (std::numeric_limits<double>::max)();
		}
	}

	auto y1 = SampleToYPos(maximum);
	auto y2 = SampleToYPos(minimum);
	m_cache.emplace_back(y1, y2);
	assert(m_cache.size() == (size_t)GetSize().GetWidth());
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
	change_window(TimeSpan{t1, t2});
	e.Skip();
}

void WaveBar::OnMotion(wxMouseEvent &e)
{
	if (m_sel_start >= 0)
	{
		auto pos = e.GetPosition();
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
}

void WaveBar::SetTimeSelection(TimeSpan win)
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
	change_window(TimeSpan{t1, t2});
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
	change_window(TimeSpan{t1, t2});
}

} // namespace phonometrica
