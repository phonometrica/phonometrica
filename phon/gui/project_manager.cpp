/***********************************************************************************************************************
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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/settings.h>
#include <phon/gui/project_manager.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

ProjectManager::ProjectManager(Runtime &rt, wxWindow *parent) :
	wxPanel(parent), runtime(rt)
{
	m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT|wxTR_MULTIPLE|wxTR_NO_LINES|wxTR_DEFAULT_STYLE);
	//m_tree = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	//m_tree->AppendColumn("Project");

	m_label = new wxStaticText(this, wxID_ANY, _("Project"), wxDefaultPosition, wxDefaultSize);
	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_label, 0, wxEXPAND|wxTOP|wxLEFT, 7);
	sizer->Add(m_tree, 10, wxEXPAND|wxTOP, 5);
	auto font = m_label->GetFont();
	font.MakeBold();
	m_label->SetFont(font);
	m_label->SetForegroundColour(wxColor(75, 75, 75));

	SetSizer(sizer);
	auto images = new wxImageList;
	m_corpus_img = images->Add(wxBitmap(Settings::get_icon_path("corpus.png"), wxBITMAP_TYPE_PNG));
	m_query_img = images->Add(wxBitmap(Settings::get_icon_path("search.png"), wxBITMAP_TYPE_PNG));
	m_data_img = images->Add(wxBitmap(Settings::get_icon_path("data.png"), wxBITMAP_TYPE_PNG));
	m_script_img = images->Add(wxBitmap(Settings::get_icon_path("console.png"), wxBITMAP_TYPE_PNG));
	m_bookmark_img = images->Add(wxBitmap(Settings::get_icon_path("favorite.png"), wxBITMAP_TYPE_PNG));
	m_annot_img = images->Add(wxBitmap(Settings::get_icon_path("annotation.png"), wxBITMAP_TYPE_PNG));
	m_tree->SetImageList(images);
	m_root = m_tree->AddRoot(_("Untitled project"));
	m_tree->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));

	m_root = m_tree->GetRootItem();
	Populate();
}

void ProjectManager::Populate()
{
	auto c = m_tree->AppendItem(m_root, _("Corpus"), m_corpus_img, m_corpus_img);
	m_tree->AppendItem(c, "Test", m_annot_img, m_annot_img);
	m_tree->AppendItem(m_root, _("Queries"), m_query_img, m_query_img);
	m_tree->AppendItem(m_root, _("Datasets"), m_data_img, m_data_img);
	m_tree->AppendItem(m_root, _("Scripts"), m_script_img, m_script_img);
	m_tree->AppendItem(m_root, _("Bookmarks"), m_bookmark_img, m_bookmark_img);

}
} // namespace phonometrica