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
 * Created: 02/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: A checklistbox that provides a tooltip for each item.                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CHECK_LIST_BOX_HPP
#define PHONOMETRICA_CHECK_LIST_BOX_HPP

#include <wx/checklst.h>
#include <phon/string.hpp>

namespace phonometrica {

class CheckListBox : public wxCheckListBox
{
public:

	CheckListBox(wxWindow *parent, const wxArrayString &choices, wxArrayString &tooltips);

	const wxString &GetToolTip(size_t i) const;

	String GetJsonValue(size_t i) const;

	void CheckAll(bool value);

protected:

	void OnMouseOver(wxMouseEvent &e);

	wxArrayString m_tooltips;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CHECK_LIST_BOX_HPP
