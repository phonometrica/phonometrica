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
 * Created: 11/06/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: dialog to add a layer.                                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_NEW_LAYER_DIALOG_HPP
#define PHONOMETRICA_NEW_LAYER_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/spinctrl.h>

namespace phonometrica {

class NewLayerDialog final : public wxDialog
{
public:

	NewLayerDialog(wxWindow *parent, intptr_t nlayer);

	wxString GetLayerLabel() const;

	bool HasInstants() const;

	int GetIndex() const;

private:

	wxTextCtrl *m_label_ctrl;

	wxChoice *m_type_choice;

	wxSpinCtrl *m_index_spin;
};

} // namespace phonometrica



#endif // PHONOMETRICA_NEW_LAYER_DIALOG_HPP
