//
// Created by Julien Eychenne on 19/02/2021.
//

#ifndef PHONOMETRICA_TOGGLE_BUTTON_HPP
#define PHONOMETRICA_TOGGLE_BUTTON_HPP

#include <wx/bmpbuttn.h>

namespace phonometrica {

class ToggleButton : public wxBitmapButton
{
public:

	ToggleButton(wxWindow *parent, wxWindowID id, const wxBitmap &bmp, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);


	bool IsChecked() const { return m_state; }

	void Check(bool value);

	void Toggle();

	void RestoreBackgroundColour();

protected:

	wxColor m_color_on, m_color_off;

	bool m_state = false;

};

} // namespace phonometrica

#endif // PHONOMETRICA_TOGGLE_BUTTON_HPP
