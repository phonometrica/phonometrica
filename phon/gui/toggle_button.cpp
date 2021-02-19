//
// Created by Julien Eychenne on 19/02/2021.
//

#include <phon/gui/toggle_button.hpp>

namespace phonometrica {

ToggleButton::ToggleButton(wxWindow *parent, wxWindowID id, const wxBitmap &bmp, const wxPoint &pos, const wxSize &size) :
	wxBitmapButton(parent, id, bmp, pos, size, wxBORDER_NONE)
{
	m_color_off = GetBackgroundColour();
	double factor = 0.65;
	auto r = m_color_off.Red();
	auto g = m_color_off.Green();
	auto b = m_color_off.Blue();
	r = (unsigned char)(r + factor * (255 - r));
	g = (unsigned char)(g + factor * (255 - g));
	b = (unsigned char)(b + factor * (255 - b));
	m_color_on = wxColor(r,g,b);

}

void ToggleButton::Check(bool value)
{
	m_state = value;
	RestoreBackgroundColour();
}

void ToggleButton::Toggle()
{
	Check(!IsChecked());
}

void ToggleButton::RestoreBackgroundColour()
{
	if (m_state)
		SetBackgroundColour(m_color_on);
	else
		SetBackgroundColour(m_color_off);

	Refresh();
}
} // namespace phonometrica