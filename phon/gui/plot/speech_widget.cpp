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

#include <phon/gui/plot/speech_widget.hpp>

namespace phonometrica {

SpeechWidget::SpeechWidget(wxWindow *parent) :
	wxWindow(parent, wxID_ANY),  m_cached_size(wxDefaultSize)
{
	SetBackgroundColour(*wxWHITE);
}

void SpeechWidget::SetTimeWindow(TimeWindow win)
{
	m_window = win;
	InvalidateCache();
	Refresh();
}

double SpeechWidget::XPosToTime(double x) const
{
    return m_window.first + (x * GetWindowDuration() / GetWidth());
}

double SpeechWidget::TimeToXPos(double t) const
{
    return (t - m_window.first) * GetWidth() / GetWindowDuration();
}

TimeWindow SpeechWidget::GetTimeWindow() const
{
	return m_window;
}

double SpeechWidget::ClipTime(double t) const
{
	if (t < 0) {
		t = 0;
	}
	else if (t > GetSoundDuration()) {
		t = GetSoundDuration();
	}

	return t;
}

void SpeechWidget::ZoomIn()
{
	SetTimeWindow(ComputeZoomIn());
}

void SpeechWidget::ZoomOut()
{
	SetTimeWindow(ComputeZoomOut());
}

void SpeechWidget::ViewAll()
{
	TimeWindow win{0, GetSoundDuration()};
	SetTimeWindow(win);
}

void SpeechWidget::MoveForward()
{
	// Slide by 10%
	if (m_window.second < GetSoundDuration())
	{
		auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
		auto t1 = ClipTime(m_window.first + delta);
		auto t2 = ClipTime(m_window.second + delta);
		SetTimeWindow(TimeWindow{t1, t2});
	}
}

void SpeechWidget::MoveBackward()
{
	// Slide by 10%
	if (m_window.first >= 0)
	{
		auto delta = std::max<double>(GetWindowDuration() / 10, 0.001);
		auto t1 = ClipTime(m_window.first - delta);
		auto t2 = ClipTime(m_window.second - delta);
		SetTimeWindow(TimeWindow{t1, t2});
	}
}

TimeWindow SpeechWidget::ComputeZoomIn() const
{
	// Zoom in by 25% on each side
	auto zoom = GetWindowDuration() / 4;
	auto t1 = ClipTime(m_window.first + zoom);
	auto t2 = ClipTime(m_window.second - zoom);

	return TimeWindow{t1, t2};
}

TimeWindow SpeechWidget::ComputeZoomOut() const
{
	// Zoom out by 50%
	auto zoom = GetWindowDuration() / 2;
	auto t1 = ClipTime(m_window.first - zoom);
	auto t2 = ClipTime(m_window.second + zoom);

	return TimeWindow{t1, t2};
}

bool SpeechWidget::HasValidCache() const
{
	return m_cached_size == GetSize();
}

void SpeechWidget::InvalidateCache()
{
	// Don't clear the cache here, it will be done in UpdateCache() once the invalid size is detected.
	m_cached_size = wxDefaultSize;
}


} // namespace phonometrica
