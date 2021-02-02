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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/check_list_box.hpp>

namespace phonometrica {

CheckListBox::CheckListBox(wxWindow *parent, const wxArrayString &choices, wxArrayString &tooltips) :
	wxCheckListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices), m_tooltips(tooltips)
{
	Bind(wxEVT_MOTION, &CheckListBox::OnMouseOver, this);
}

void CheckListBox::OnMouseOver(wxMouseEvent &e)
{
	auto item = HitTest(e.GetPosition());
	if (item != wxNOT_FOUND)
	{
		SetToolTip(m_tooltips.at(item));
	}
	e.Skip();
}

} // namespace phonometrica
