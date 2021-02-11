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
 * Created: 10/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/toolbar.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/views/concordance_view.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

ConcordanceView::ConcordanceView(wxWindow *parent, AutoConcordance conc) :
	View(parent), m_conc(std::move(conc))
{
	auto sizer = new VBoxSizer;
	m_grid = new wxGrid(this, wxID_ANY);
	auto ctrl = new ConcordanceController(m_conc);
	auto label = wxString::Format("%d matches", (int)m_conc->row_count());
	count_label = new wxStaticText(this, wxID_ANY, "label");
	auto font = count_label->GetFont();
	font.MakeBold();
	count_label->SetFont(font);

	auto pt_size = m_grid->GetFont().GetPointSize();
	auto mono_font = Settings::get_mono_font();
	mono_font.SetPointSize(pt_size);
	m_grid->SetDefaultCellFont(mono_font);

	mono_font.MakeBold();
	auto prov = new ConcCellAttrProvider(mono_font);
	ctrl->SetAttrProvider(prov);
	m_grid->SetTable(ctrl, true);
	m_grid->AutoSizeColumns();

	auto label_sizer = new HBoxSizer;
	label_sizer->Add(count_label, 0, wxEXPAND);
	label_sizer->AddStretchSpacer();
	sizer->Add(label_sizer, 0, wxEXPAND|wxALL, 10);
	sizer->Add(m_grid, 1, wxEXPAND|wxALL, 10);
	SetSizer(sizer);
}

bool ConcordanceView::IsModified() const
{
	return m_conc->modified();
}

void ConcordanceView::DiscardChanges()
{
	m_conc->discard_changes();
}

wxString ConcordanceView::GetLabel() const
{
	return m_conc->label();
}

String ConcordanceView::GetPath() const
{
	return m_conc->path();
}
} // namespace phonometrica
