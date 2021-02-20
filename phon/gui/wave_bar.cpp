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
	m_sound(snd)
{
	SetBackgroundColour(*wxWHITE);
	Bind(wxEVT_PAINT, &WaveBar::OnPaint, this);
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
}

void WaveBar::OnPaint(wxPaintEvent &)
{
	wxPaintDC dc(this);
	Render(dc);
}

void WaveBar::Render(wxPaintDC &dc)
{
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
	MakePath(path);
	gc->DrawPath(path);


//	QColor col(Qt::blue);
//	col = col.lighter(160);
//	col.setAlpha(60);
//
//	// If the user if making a selection backward, make sure that it is displayed properly
//	auto w = int(round(std::abs(to - from)));
//	auto x = int(round((from < to) ? from : to));
//
//	if (w <= 1)
//	{
//		QPen pen;
//		col.setAlpha(120);
//		pen.setColor(col);
//		painter.setPen(pen);
//		painter.drawLine(x, 0, x, height());
//	}
//	else
//	{
//		painter.fillRect(x, 0, w, height(), col);
//	}

}

void WaveBar::MakePath(wxGraphicsPath &path)
{
	int x = 0;
	auto sample_count = m_sound->channel_size();
	auto &data = m_sound->data();
	int nchannel = m_sound->nchannel();
	auto height = GetSize().GetHeight();
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

			if (x == 0)
			{
				path.MoveToPoint(0.0, y1);
			}
			else
			{
				path.AddLineToPoint(x, y1);
			}
			path.AddLineToPoint(x, y2);
			x++;

			// reset values
			maximum = (std::numeric_limits<double>::min)();
			minimum = (std::numeric_limits<double>::max)();
		}
	}

	auto y1 = SampleToYPos(maximum);
	auto y2 = SampleToYPos(minimum);
	path.AddLineToPoint(x, y1);
	path.AddLineToPoint(x, y2);
}

//
//double WaveBar::SampleToXPos(intptr_t s) const
//{
//    return s * ((double)GetSize().GetWidth() / m_sound->channel_size());
//}
//
//intptr_t WaveBar::XPosToSample(double x) const
//{
//    return (intptr_t) round(x * m_sound->channel_size() / GetSize().GetWidth()) + 1;
//}

double WaveBar::SampleToYPos(double s) const
{
    const int H = GetSize().GetHeight()/ 2;
    return H - s * (double(H) / raw_magnitude);
}

} // namespace phonometrica
