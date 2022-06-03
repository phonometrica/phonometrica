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
 * Created: 03/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/checkbox.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/property_ctrl.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

static const int RANGE_OP = 7;

PropertyCtrl::PropertyCtrl(wxWindow *parent, const String &category, const std::type_info &type) :
		wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 200)), type(type), category(category)
{
    auto sizer = new VBoxSizer;
    auto txt = new wxStaticText(this, wxID_ANY, category);
    auto font = txt->GetFont();
    font.MakeBold();
    txt->SetFont(font);
    auto head_sizer = new HBoxSizer;
    head_sizer->Add(txt, 0, wxLEFT|wxTOP, 10);
    head_sizer->AddStretchSpacer();
    sizer->Add(head_sizer, 0, wxEXPAND);

    if (type == typeid(bool))
    {
        wxArrayString values;
	    values.Add("any");
        values.Add("true");
	    values.Add("false");
	    choicelist = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, values);
        choicelist->SetSelection(0);
        sizer->Add(choicelist, 0, wxEXPAND | wxALL, 10);
        sizer->AddStretchSpacer();
        choicelist->Bind(wxEVT_CHOICE, [this](wxCommandEvent &e) { modified(); });
    }
    else if (type == typeid(double))
    {
    	wxArrayString values;
    	values.Add("any");
    	values.Add("equal");
    	values.Add("not equal");
    	values.Add("less than");
    	values.Add("less than or equal");
    	values.Add("greater than");
    	values.Add("greater than or equal");
    	values.Add("inclusive range");
    	values.Add("exclusive range");
	    choicelist = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, values);
		choicelist->SetSelection(0);

	    entry1 = new wxTextCtrl(this, wxID_ANY);
        entry2 = new wxTextCtrl(this, wxNewId());
	    entry2->Disable();
        sizer->Add(choicelist, 0, wxEXPAND | wxALL, 10);
        sizer->Add(entry1, 0, wxEXPAND | wxLEFT|wxRIGHT, 10);
        sizer->Add(entry2, 0, wxEXPAND | wxALL, 10);
		sizer->AddStretchSpacer();
		choicelist->Bind(wxEVT_CHOICE, [this](wxCommandEvent &e) { entry2->Enable(e.GetSelection() >= RANGE_OP); modified(); });
    }
    else
    {
    	auto cb = new wxCheckBox(this, wxID_ANY, wxString());
    	cb->SetToolTip(_("Check all values"));
    	head_sizer->Add(cb, 0, wxRIGHT|wxTOP, 10);
        wxArrayString values;

        for (auto &value : Property::get_values(category)) {
            values.Add(value);
        }
        checklist = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, values);
        checklist->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent &) { checklist->SetSelection(-1); });
        checklist->Bind(wxEVT_CHECKLISTBOX, [this](wxCommandEvent &) { modified(); });
        auto pt = checklist->GetFont().GetPointSize();
        auto font = Settings::get_mono_font();
        font.SetPointSize(pt);
        checklist->SetFont(font);
        sizer->Add(checklist, 1, wxEXPAND | wxALL, 10);
        sizer->AddSpacer(20);
        cb->Bind(wxEVT_CHECKBOX, &PropertyCtrl::OnCheckAllItems, this);
    }

    this->SetSizer(sizer);
}

const std::type_info &PropertyCtrl::GetType() const
{
    return type;
}

bool PropertyCtrl::GetBoolean() const
{
    switch (choicelist->GetSelection())
    {
        case 1:
            return true;
    	case 2:
            return false;
    	default:
    		assert(false);
    }

    return false;
}

std::pair<double, double> PropertyCtrl::GetNumericValue() const
{
    std::pair<double,double> result;
	double tmp;

    if (!entry1->GetValue().ToCDouble(&tmp))
    {
    	throw error("Invalid number in numeric property");
    }
    result.first = tmp;

    if (choicelist->GetSelection() >= RANGE_OP)
    {
    	if (!entry2->GetValue().ToCDouble(&tmp))
    	{
    		throw error("Invalid number in numeric property");
    	}
    	result.second = tmp;
    }

    return result;
}

Array<String> PropertyCtrl::GetTextValues() const
{
    wxArrayInt items;
    Array<String> values;
    int count = checklist->GetCheckedItems(items);

    for (int i = 0; i < count; i++)
    {
        int id = items[i];
        values.push_back(checklist->GetString(id));
    }

    return values;
}

bool PropertyCtrl::HasSelection() const
{
	if (type == typeid(String))
	{
		for (int i = 0; i < (int)checklist->GetCount(); i++)
		{
			 if (checklist->IsChecked(i)) {
			 	return true;
			 }
		}
		return false;
	}
	else // num or bool
	{
		return choicelist->GetSelection() > 0;
	}
}

void PropertyCtrl::OnCheckAllItems(wxCommandEvent &e)
{
	for (unsigned int i = 0; i < checklist->GetCount(); i++)
	{
		checklist->Check(i, e.IsChecked());
	}
	modified();
}

const String &PropertyCtrl::GetCategory() const
{
	return category;
}

int PropertyCtrl::GetOperator() const
{
	return choicelist->GetSelection();
}

void PropertyCtrl::CheckValues(const Array<String> &values)
{
	for (auto &v : values)
	{
		wxString value = v;
		for (int i = 0; i < (int)checklist->GetCount(); i++)
		{
			if (checklist->GetString(i) == value)
			{
				checklist->Check(i);
				break;
			}
		}
	}
}

void PropertyCtrl::SetNumericValue(int op, std::pair<double, double> value)
{
	choicelist->SetSelection(op);
	entry1->ChangeValue(wxString::FromCDouble(value.first));
	if (op >= RANGE_OP)
	{
		entry2->Enable();
		entry2->ChangeValue(wxString::FromCDouble(value.second));
	}
}

void PropertyCtrl::SetBoolean(bool value)
{
	choicelist->SetSelection(value ? 1 : 2);
}


} // namespace phonometrica