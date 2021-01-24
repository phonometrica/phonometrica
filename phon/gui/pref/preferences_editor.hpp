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
 * Created: 20/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Main preferences dialog.                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PREFERENCES_EDITOR_HPP
#define PHONOMETRICA_PREFERENCES_EDITOR_HPP

#include <wx/checkbox.h>
#include <wx/simplebook.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

class PreferencesEditor final : public PreferencesDialog
{
public:

	explicit PreferencesEditor(wxWindow *parent);

private:

	void DoOk() override;
	void DoReset() override;

	wxPanel *MakeGeneralPanel();

	wxPanel *MakeSoundPanel();

	wxSpinCtrl *m_match_window_ctrl;

	wxCheckBox *m_autosave_checkbox, *m_autoload_checkbox;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PREFERENCES_EDITOR_HPP
