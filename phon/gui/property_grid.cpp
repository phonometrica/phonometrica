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
 * Created: 26/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/textctrl.h>
#include <phon/gui/property_grid.hpp>

namespace phonometrica {

class CellEditor : public wxGridCellTextEditor
{
public:

	CellEditor() = default;

	wxTextCtrl *GetTextCtrl() const { return Text(); }
};


PropertyGrid::PropertyGrid(wxWindow *parent) : wxGrid(parent, wxID_ANY)
{
	CreateGrid(0, 3);
	HideRowLabels();
	SetColLabelValue(0, _("Type"));
	SetColLabelValue(1, _("Key"));
	SetColLabelValue(2, _("Value"));
	SetColLabelAlignment(wxALIGN_LEFT, wxALIGN_CENTRE);
	Bind(wxEVT_SIZE, &PropertyGrid::OnResize, this);
}

void PropertyGrid::OnResize(wxSizeEvent &)
{
	int w = GetClientSize().GetWidth();
//	SetDefaultColSize(w/3);
//	SetColSize(0, w/5);
//	w = w - w/5;
//	SetColSize(1, w/2);
//	SetColSize(2, w/2);
//
	SetColSize(0, w/3);
	SetColSize(1, w/3);
	SetColSize(2, w/3);
}

void PropertyGrid::AppendProperties(const std::set<Property> &properties)
{
	AppendRows((int)properties.size());
	int i = 0;

	for (auto &p : properties)
	{
		SetCellValue(i, 1, p.category());


		wxArrayString choices;
		for (auto &value : Property::get_values(p.category())) {
			choices.Add(value);
		}

		SetCellValue(i, 2, p.value());
		SetReadOnly(i, 1, true);
		wxString type;

		if (p.is_text())
		{
			type = "Text";
		}
		else if (p.is_boolean())
		{
			type = "Boolean";
		}
		else if (p.is_numeric())
		{
			type = "Number";
		}
		SetCellValue(i, 0, type);

		i++;
	}

}

void PropertyGrid::SetEditingMode(int row, bool value)
{
	wxColour color = value ? *wxRED : GetDefaultCellTextColour();
	auto font = GetDefaultCellFont();
	if (value) {
		font.MakeBold();
	}

	for (int col = 0; col <= 2; col++)
	{
		SetCellTextColour(row, col, color);
		SetCellFont(row, col, font);
	}
}
} // namespace phonometrica
