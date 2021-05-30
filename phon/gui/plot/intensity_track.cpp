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
 * Created: 25/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/intensity_track.hpp>
#include <phon/application/settings.hpp>

#ifdef __WXMSW__
#undef DrawText
#endif

namespace phonometrica {

IntensityTrack::IntensityTrack(wxWindow *parent, const Handle<Sound> &snd, int channel)
		: SoundPlot(parent, snd, channel)
{
	try
	{
		ReadSettings();
	}
	catch (...)
	{
		Settings::reset_intensity();
		ReadSettings();
	}
}

void IntensityTrack::ReadSettings()
{
	String cat("intensity");
	min_dB = Settings::get_number(cat, "minimum_intensity");
	max_dB = Settings::get_number(cat, "maximum_intensity");
	time_step = Settings::get_number(cat, "time_step");
}

void IntensityTrack::OnMotion(wxMouseEvent &e)
{
	SoundPlot::OnMotion(e);
	if (m_track_mouse)
	{
		double dB = YPosToIntensity(e.GetPosition().y);
		auto msg = wxString::Format(_("Intensity at cursor: %.2f dB"), dB);
		update_status(msg);
	}
}

double IntensityTrack::YPosToIntensity(int y) const
{
	auto height = double(GetSize().GetHeight());
	return (double(max_dB - min_dB) * (height - y)) / height + min_dB;
}

double IntensityTrack::IntensityToYPos(double dB) const
{
	auto height = double(GetSize().GetHeight());
	return height - ((dB - min_dB) * height / (max_dB - min_dB));
}

void IntensityTrack::DrawYAxis(PaintDC &dc, const wxRect &rect)
{
	auto top = wxString::Format("%d db", int(max_dB));
	auto bottom = wxString::Format("%d db", int(min_dB));

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

void IntensityTrack::UpdateCache()
{
	DrawBitmap();
	m_cached_size = GetSize();
}

void IntensityTrack::DrawBitmap()
{
	auto db_data = CalculateIntensity();
	wxMemoryDC dc;
	wxBitmap bmp(GetSize());
	dc.SelectObject(bmp);
	dc.SetBackground(*wxWHITE);
	dc.Clear();
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	gc->SetPen(wxPen(*wxGREEN, 2));
	wxGraphicsPath path = gc->CreatePath();

	double t = m_window.first + time_step;// / 2;
	bool previous = false; // no intensity before

	for (auto dB : db_data)
	{
		if (!std::isfinite(dB))
		{
			previous = false;
			continue;
		}
		else if (previous)
		{
			auto x = TimeToXPos(t);
			auto y = IntensityToYPos(dB);
			path.AddLineToPoint(x, y);
		}
		else
		{
			auto x = TimeToXPos(t);
			auto y = IntensityToYPos(dB);
			path.MoveToPoint(x, y);
		}

		previous = true;
		t += time_step;
	}

	gc->StrokePath(path);
	dc.SelectObject(wxNullBitmap);
	m_cached_bmp = bmp;
}

Array<double> IntensityTrack::CalculateIntensity()
{
    auto start_pos = m_sound->time_to_frame(m_window.first);
    auto end_pos = m_sound->time_to_frame(m_window.second);

    return m_sound->get_intensity(m_channel, start_pos, end_pos, time_step);
}
} // namespace phonometrica
