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
 * Created: 27/03/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Settings for spectrograms.                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP
#define PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP

#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

class SpectrogramSettings final : public PreferencesDialog
{
public:

	SpectrogramSettings(wxWindow *parent);

private:

	void DoReset() override;

	bool DoOk() override;

	void OnEnableCustomBand(wxCommandEvent &);

	void OnDisableCustomBand(wxCommandEvent &);

	void EnableCustomBand(bool value);

	void OnDynamicRangeChanged(wxCommandEvent &);

	void SetDynamicRangeLabel(int value);

	wxPanel *MakeGeneralPanel();

	void DisplayValues();

	wxRadioButton *wide_btn, *narrow_btn, *custom_btn;

	wxTextCtrl *winlen_ctrl, *bandwidth_ctrl, *preemph_ctrl;

	wxChoice *window_choice;

	wxStaticText *dyn_range_label;

	wxSlider *dyn_range_slider;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SPECTROGRAM_SETTINGS_HPP
