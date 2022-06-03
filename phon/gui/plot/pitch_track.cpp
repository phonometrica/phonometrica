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
 * Created: 12/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/plot/pitch_track.hpp>
#include <phon/application/settings.hpp>
#include <phon/third_party/swipe/swipe.h>

std::vector<double>
rapt(phonometrica::Array<double> &data, double sample_freq, int frame_shift, double minF0, double maxF0, double voice_bias, int otype);

std::vector<double>
rapt2(phonometrica::Array<double> &input, double sample_rate, double time_step, double min_f0, double max_f0);

namespace phonometrica {

PitchTrack::PitchTrack(wxWindow *parent, const Handle<Sound> &snd, int channel) :
	SoundPlot(parent, snd, channel)
{
	try
	{
		ReadSettings();
	}
	catch (...)
	{
		Settings::reset_pitch_tracking();
		ReadSettings();
	}
}

void PitchTrack::DrawYAxis(PaintDC &dc, const wxRect &rect)
{
	auto top = wxString::Format("%d Hz", int(max_pitch));
	auto bottom = wxString::Format("%d Hz", int(min_pitch));

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

void PitchTrack::UpdateCache()
{
	DrawBitmap();
	m_cached_size = GetSize();
}

void PitchTrack::ReadSettings()
{
    String category("pitch_tracking");
    min_pitch = Settings::get_number(category, "minimum_pitch");
    max_pitch = Settings::get_number(category, "maximum_pitch");
    time_step = Settings::get_number(category, "time_step");
    voicing_threshold = Settings::get_number(category, "voicing_threshold");
}

void PitchTrack::DrawBitmap()
{
	wxMemoryDC dc;
	wxBitmap bmp(GetSize());
	dc.SelectObject(bmp);
	dc.SetBackground(*wxWHITE);
	dc.Clear();
	auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
	if (!gc) return;
	gc->SetPen(wxPen(*wxBLUE, 2));
	wxGraphicsPath path = gc->CreatePath();

	try
	{
		m_pitch.clear(); // in case CalculatePitch() throws
		m_pitch = CalculatePitch();
		double t = m_window.first;
		bool previous = false;

		for (auto f : m_pitch)
		{
//			t += time_step;

			if (std::isfinite(f))
			{
				auto x = TimeToXPos(t);
				auto y = PitchToYPos(f);
				if (previous) {
					path.AddLineToPoint(x, y);
					PHON_LOG("Pitch: add line x = %f, f0 = %f\n", x, f);
				}
				else {
					path.MoveToPoint(x, y);
					PHON_LOG("Pitch: move to point x = %f, f0 = %f\n", x, f);
				}
				previous = true;
			}
			else
			{
				previous = false;
			}
			t += time_step;
		}

		gc->StrokePath(path);
	}
	catch (std::exception &e)
	{
		wxString msg = e.what();
		auto sz = dc.GetTextExtent(msg);
		auto x = double(GetWidth()) / 2 - double(sz.x) / 2;
		auto y = double(GetHeight()) / 2 - double(sz.y) / 2;
		gc->SetFont(gc->CreateFont(dc.GetFont()));
		gc->DrawText(msg, x, y);
	}

	dc.SelectObject(wxNullBitmap);
	m_cached_bmp = bmp;
}

std::vector<double> PitchTrack::CalculatePitch()
{
	if (GetWindowDuration() <= time_step * 2) {
		throw error("Zoom out to see pitch");
	}
	if (GetWindowDuration() / time_step > GetWidth() * 2) {
		throw error("Zoom in to see pitch");
	}
	auto first_sample = m_sound->time_to_frame(m_window.first);
	auto last_sample = m_sound->time_to_frame(m_window.second);
	auto input = m_sound->get_channel(m_channel, first_sample, last_sample);
	auto sample_rate = m_sound->sample_rate();

#if 1
	vector vec;
	vec.x = (int) input.size();
	vec.v = input.data();

	auto tmp = swipe(vec, sample_rate, min_pitch, max_pitch, voicing_threshold, time_step);
	std::vector<double> result(tmp.v, tmp.v + tmp.x);
	utils::free(tmp.v);

	return result;
#else
//	auto frame_shift = int(ceil(sample_rate*time_step));
//	return rapt(input, sample_rate, frame_shift, 80, 250, 0, 1);
	return rapt2(input, sample_rate, time_step, 70, 500);
#endif
}

double PitchTrack::YPosToHertz(int y) const
{
	auto height = GetHeight();
	return (double(max_pitch - min_pitch) * (height - y)) / height + min_pitch;
}

double PitchTrack::PitchToYPos(double hz) const
{
	auto height = GetHeight();
    return height - ((hz - min_pitch) * height / (max_pitch - min_pitch));
}

} // namespace phonometrica
