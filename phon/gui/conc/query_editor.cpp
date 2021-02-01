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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <phon/gui/conc/query_editor.hpp>
#include <phon/runtime.hpp>

namespace phonometrica {

intptr_t QueryEditor::id = 0;


QueryEditor::QueryEditor(wxWindow *parent, const wxString &title) :
	wxDialog(parent, wxID_ANY, title)
{

}

QueryEditor::QueryEditor(wxWindow *parent, const wxString &title, const AutoQuery &query) :
	wxDialog(parent, wxID_ANY, title), m_query(query)
{

}

void QueryEditor::SetHeader()
{

}

void QueryEditor::SetMetadata()
{

}

wxBoxSizer *QueryEditor::MakeButtons(wxWindow *parent)
{
	auto sizer = new wxBoxSizer(wxHORIZONTAL);
	auto save_btn = new wxButton(parent, wxID_ANY, _("Save"));
	save_btn->SetToolTip(_("Save query"));
	auto cancel_btn = new wxButton(parent, wxID_ANY, _("Cancel"));
	auto ok_btn = new wxButton(parent, wxID_ANY, _("OK"));
	sizer->Add(save_btn);
	sizer->AddStretchSpacer();
	sizer->Add(cancel_btn);
	sizer->AddSpacer(5);
	sizer->Add(ok_btn);

	return sizer;
}

void QueryEditor::Prepare()
{
	// Since we can't call virtual functions in the constructor, we setup the UI after the object is set up.
	this->SetSize(1000, 700);
	this->SetPosition(wxPoint(100, 70));

	auto scrolled_window = new wxScrolledWindow(this);
	auto scrolled_sizer = new wxBoxSizer(wxVERTICAL);

	SetHeader();
	SetSearchPanel();
	SetMetadata();
	auto buttons = MakeButtons(scrolled_window);
	scrolled_sizer->Add(buttons, 0, wxEXPAND|wxALL, 10);

	scrolled_window->SetSizer(scrolled_sizer);
	auto main_sizer = new wxBoxSizer(wxVERTICAL);
	scrolled_window->SetScrollRate(5, 5);
	scrolled_window->FitInside();
	scrolled_window->SetVirtualSize(1000, 600);
	main_sizer->Add(scrolled_window, 1, wxEXPAND|wxALL, 0);
	SetSizer(main_sizer);
	prepared = true;
}

void QueryEditor::Execute()
{
	if (!prepared) {
		throw error("Internal error: you must call Prepare() before executing a query");
	}

}
} // namespace phonometrica
