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
 * Created: 28/03/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Preference dialog for formant tracking.                                                                    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FORMANT_SETTINGS_HPP
#define PHONOMETRICA_FORMANT_SETTINGS_HPP

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

class FormantSettings final : public PreferencesDialog
{
public:

	FormantSettings(wxWindow *parent);

private:

	void DoReset() override;

	bool DoOk() override;

	wxPanel *MakeGeneralPanel();

	void DisplayValues();

	wxTextCtrl *nformant_ctrl, *window_ctrl, *npole_ctrl, *max_freq_ctrl, *step_ctrl;
};

} // namespace phonometrica

#endif // PHONOMETRICA_FORMANT_SETTINGS_HPP
