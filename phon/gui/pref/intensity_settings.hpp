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
 * Purpose: .                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_INTENSITY_SETTINGS_HPP
#define PHONOMETRICA_INTENSITY_SETTINGS_HPP

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

class IntensitySettings final : public PreferencesDialog
{
public:

	IntensitySettings(wxWindow *parent);

private:

	void DoReset() override;

	bool DoOk() override;

	wxPanel * MakeGeneralPanel();

	void DisplayValues();

	wxTextCtrl *min_ctrl, *max_ctrl, *step_ctrl;
};

} // namespace phonometrica



#endif // PHONOMETRICA_INTENSITY_SETTINGS_HPP
