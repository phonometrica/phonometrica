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
 * Created: 14/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Dialog for concordance joints.                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_JOIN_DIALOG_HPP
#define PHONOMETRICA_CONCORDANCE_JOIN_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

class ConcordanceJoinDialog final : public wxDialog
{
public:

	ConcordanceJoinDialog(wxWindow *parent, const wxString &title);

	wxString GetLabel() const;

	AutoConcordance GetConcordance() const;

private:

	wxTextCtrl *m_text;

	wxChoice *m_choice;

	Array<AutoConcordance> m_items;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_JOIN_DIALOG_HPP
