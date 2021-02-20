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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/waveform.hpp>

namespace phonometrica {

Waveform::Waveform(wxWindow *parent, const Handle<Sound> &snd, int channel) :
	SoundPlot(parent, snd), m_cached_size(wxDefaultSize), m_channel(channel)
{
	Bind(wxEVT_PAINT, &Waveform::OnPaint, this);
}

void Waveform::ClearCache()
{
	m_cache.clear();
	m_cached_size = wxDefaultSize;
}

void Waveform::OnPaint(wxPaintEvent &)
{
	UpdateCache();
	wxPaintDC dc(this);
	Render(dc);
}

void Waveform::Render(wxPaintDC &dc)
{
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	auto height = GetSize().GetHeight();
	auto width = GetSize().GetWidth();

	// Draw zero-crossing line.
	dc.SetPen(wxPen(*wxBLUE, 1, wxPENSTYLE_DOT));
	dc.DrawLine(0, height/2, width, height/2);

	if (m_cache.size() >= width)
	{
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
	}
	else
	{
#if 0
		// If we have fewer samples than pixels, we spread the samples over the screen
        auto line_pen = painter.pen();
        QPen point_pen;
        point_pen.setWidth(line_pen.width() * 3);
        auto pixels_per_frame = (double)this->width() / sample_count;
        double previous_x = 0;
        double previous_y = 0;
        double x = 0;

        // Add one more pixel if possible to reach the right edge of the screen
        intptr_t last;
        if (last_sample < m_data->size())
        {
            last = last_sample + 1;
        }
        else
        {
            last = last_sample;
            pixels_per_frame = (double)this->width() / (this->windowDuration() - 1);
        }

        bool display_points = (pixels_per_frame >= 10);

        for (intptr_t i = first_sample; i <= last; i++)
        {
            auto sample = m_data->sample(i);
            auto y = sampleToHeight(sample);

            if (x != 0)
            {
                QPointF p1(previous_x, previous_y);
                QPointF p2(x, y);
                if (display_points)
                {
                    painter.setPen(point_pen);
                    painter.drawPoint(p2);
                    painter.setPen(line_pen);
                }
                painter.drawLine(p1, p2);
            }
            previous_x = x;
            previous_y = y;
            x += pixels_per_frame;
        }
#endif
	}

//	if (!HasSelection()) {
//		return;
//	}
//	path.MoveToPoint(m_sel.from, 0.0);
//	path.AddLineToPoint(m_sel.to, 0.0);
//	path.AddLineToPoint(m_sel.to, height);
//	path.AddLineToPoint(m_sel.from, height);
//	path.AddLineToPoint(m_sel.from, 0.0);
//	wxBrush brush;
//	brush.SetColour(WAVEBAR_SEL_COLOUR);
//	gc->SetBrush(brush);
//	gc->FillPath(path);
}

void Waveform::UpdateCache()
{
	if (GetSize() == m_cached_size) {
		return;
	}
	// If the number of samples to display is greater than the number of pixels,
	// map several frames to one pixel. We find the maximum and minimum amplitudes
	// over the range of frames, and we draw a line from the previous minimum to the
	// current maximum, and from the current maximum to the current mimimum.
	auto first_sample = m_sound->time_to_frame(m_window.from);
	auto last_sample = m_sound->time_to_frame(m_window.to);
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
	auto width = GetSize().GetWidth();
	auto raw_data = data.data();

	if (sample_count >= width)
	{
		assert(m_window.to <= m_sound->duration());

		// Subtract 1 to width so that the last pixel is assigned the left-over frames.
		auto frames_per_pixel = sample_count / (width - 1);

		auto maximum = -(std::numeric_limits<double>::max)();
		auto minimum = (std::numeric_limits<double>::max)();
		intptr_t min_index = (std::numeric_limits<intptr_t>::min)();
		intptr_t max_index = (std::numeric_limits<intptr_t>::min)();

		for (size_t i = 0; i < sample_count; i++)
		{
			auto sample = *raw_data++;

			if (sample < minimum)
			{
				minimum = sample;
				min_index = i;
			}
			if (sample > maximum)
			{
				maximum = sample;
				max_index = i;
			}

			if (i % frames_per_pixel == 0)
			{
				double y1 = SampleToHeight(maximum);
				double y2 = SampleToHeight(minimum);
				m_cache.emplace_back(y1, y2);

				// reset values
				maximum = -(std::numeric_limits<double>::max)();
				minimum = (std::numeric_limits<double>::max)();
				min_index = max_index = (std::numeric_limits<intptr_t>::min)();
			}
		}

		double y1 = SampleToHeight(maximum);
		double y2 = SampleToHeight(minimum);
		if (min_index < max_index) {
			std::swap(y1, y2);
		}
		m_cache.emplace_back(y1, y2);
	}
	m_cached_size = GetSize();
}

double Waveform::SampleToHeight(double s) const
{
	const double H = (double)GetSize().GetHeight() / 2;
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

} // namespace phonometrica
