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

/*
 * This cell editor provides autocompletion for property values. Figuring out how to get it to work wasn't trivial due to
 * the lack of documentation, I'll lay out the solution here. We must first create a custom cell editor that derives from
 * wxGridCellTextEditor. When we create the row, we must install this custom editor on the value's cell. The key is to
 * override its Create() method, to get a pointer to its wxTextCtrl (which is a protected member) and to set autocompletion
 * choices there.
 */
class PropertyValueEditor final : public wxGridCellTextEditor
{
public:

	PropertyValueEditor(const String &cat) : category(cat) { }

	void OnTextEnter(wxCommandEvent &)
	{
		bool b = 1;
	}

	void Create(wxWindow *parent, wxWindowID id, wxEvtHandler *handler) override
	{
		wxGridCellTextEditor::Create(parent, id, handler);
		wxTextCtrl *txt = Text();
		assert(txt);
		wxArrayString choices;
		for (auto &value : Property::get_values(category)) {
			choices.Add(value);
		}
		txt->SetExtraStyle(wxTE_PROCESS_ENTER);
		txt->AutoComplete(choices);
		txt->Bind(wxEVT_TEXT_ENTER, &PropertyValueEditor::OnTextEnter, this);
	}

	String category;
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
	SetColSize(0, w/3);
	SetColSize(1, w/3);
	SetColSize(2, w/3);
}

void PropertyGrid::AppendProperties(const std::set<Property> &properties)
{
	AppendRows((int)properties.size());
	int row = 0;

	for (auto &p : properties)
	{
		wxString type;
		wxString value;

		if (p.is_text())
		{
			type = "Text";
			SetPropertyEditor(row, p.category());
			value = p.value();
		}
		else if (p.is_boolean())
		{
			type = "Boolean";
			value = p.boolean_value() ? "1" : "";
			SetCellRenderer(row, 2, new wxGridCellBoolRenderer);
			SetCellEditor(row, 2, new wxGridCellBoolEditor);
		}
		else if (p.is_numeric())
		{
			value = p.value();
		}
		SetCellValue(row, 0, type);
		SetCellValue(row, 1, p.category());
		SetCellValue(row, 2, value);
		SetReadOnly(row, 0, true);
		SetReadOnly(row, 1, true);

		row++;
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

void PropertyGrid::SetPropertyEditor(int row, const String &category)
{
	auto editor = new PropertyValueEditor(category);
	SetCellEditor(row, 2, editor);
}
} // namespace phonometrica
