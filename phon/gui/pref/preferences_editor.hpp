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

#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/simplebook.h>
#include <wx/combobox.h>
#include <wx/fontpicker.h>
#include <phon/gui/pref/preferences_dialog.hpp>

namespace phonometrica {

class PreferencesEditor final : public PreferencesDialog
{
public:

	explicit PreferencesEditor(wxWindow *parent);

private:

	bool DoOk() override;

	void DoReset() override;

	wxPanel *MakeGeneralPanel();

	wxPanel *MakeAppearancePanel();

	wxFontPickerCtrl *m_font_picker;

	wxCheckBox *m_autosave_checkbox, *m_autoload_checkbox, *m_autohints_checkbox, *m_restore_views_checkbox;

	wxCheckBox *m_empty_conc_checkbox;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PREFERENCES_EDITOR_HPP
