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

#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <phon/gui/sizer.hpp>
#include <phon/gui/dialog.hpp>
#include <phon/gui/views/concordance_view.hpp>
#include <phon/application/cmd/delete_match_command.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/icons.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/praat.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

ConcordanceView::ConcordanceView(wxWindow *parent, AutoConcordance conc) :
	View(parent), m_conc(std::move(conc))
{
#define ICN(x) wxBITMAP_PNG_FROM_DATA(x)

	auto sizer = new VBoxSizer;

	m_toolbar = new ToolBar(this);

	auto save_icon = ICN(save);
	m_save_tool = m_toolbar->AddButton(save_icon, _("Save concordance... (" CTRL_KEY "S)"));
	auto csv_tool = m_toolbar->AddButton(ICN(export_csv), _("Export concordance to CSV..."));
	m_toolbar->AddSeparator();

	auto play_icon = ICN(play);
	m_play_tool = m_toolbar->AddButton(play_icon, _("Play selection"));

	auto stop_tool = m_toolbar->AddButton(ICN(stop), _("Stop playing"));
	auto praat_tool = m_toolbar->AddButton(ICN(praat), _("Open selection in Praat"));
	m_toolbar->AddSeparator();

	auto view_tool = m_toolbar->AddButton(ICN(eye), _("Open match in annotation"));

	auto del_row_tool = m_toolbar->AddButton(ICN(delete_row), _("Delete selected row(s)"));
	m_col_tool = m_toolbar->AddMenuButton(ICN(select_column_dropdown), _("Show/hide columns"));

	auto bookmark_tool = m_toolbar->AddButton(ICN(favorite24), _("Bookmark match"));
	m_toolbar->AddStretchableSpace();
	auto help_tool = m_toolbar->AddHelpButton();

	m_grid = new wxGrid(this, wxID_ANY);
	auto ctrl = new ConcordanceController(m_conc);
	count_label = new wxStaticText(this, wxID_ANY, wxString());
	// FIXME: on Linux, the label is truncated after the number if it is bold, but it is displayed correctly if the
	//  user clicks on another file in the project manager (which must trigger a refresh).
#ifndef __WXGTK__
	auto font = count_label->GetFont();
	font.MakeBold();
	count_label->SetFont(font);
#endif
	UpdateCountLabel();

	auto pt_size = m_grid->GetFont().GetPointSize();
	auto mono_font = Settings::get_mono_font();
	mono_font.SetPointSize(pt_size);
	m_grid->SetDefaultCellFont(mono_font);
	m_grid->EnableEditing(false);

	mono_font.MakeBold();
	auto prov = new ConcCellAttrProvider(*m_conc, mono_font);
	ctrl->SetAttrProvider(prov);
	m_grid->SetTable(ctrl, true);
	m_grid->AutoSizeColumns();
	m_grid->SetDefaultRowSize(30);
	m_grid->DisableDragRowSize();
	m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	m_grid->SetCellHighlightPenWidth(0);
	m_grid->SetCellHighlightROPenWidth(0);

	auto label_sizer = new HBoxSizer;
	label_sizer->Add(count_label, 0, wxEXPAND);
	label_sizer->AddStretchSpacer();
	sizer->Add(m_toolbar, 0, wxEXPAND|wxTOP, 10);
	sizer->Add(label_sizer, 0, wxEXPAND|wxALL, 10);
	sizer->Add(m_grid, 1, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
	SetSizer(sizer);

	ShowFileInfo();
	ShowMetadata();

	m_save_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnSave, this);
	m_play_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnPlaySelection, this);
	stop_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnStopPlaying, this);
	praat_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnOpenInPraat, this);
	csv_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnExportToCsv, this);
	view_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnViewMatch, this);
	del_row_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnDeleteRows, this);
	bookmark_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnBookmarkMatch, this);
	help_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnHelp, this);
	m_col_tool->Bind(wxEVT_LEFT_DOWN, &ConcordanceView::OnColumnButtonClicked, this);
	m_grid->Bind(wxEVT_KEY_DOWN, &ConcordanceView::OnKeyDown, this);
#undef ICN
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

void ConcordanceView::OnSave(wxCommandEvent &)
{
	Save();
}

void ConcordanceView::OnPlaySelection(wxCommandEvent &)
{

}

void ConcordanceView::OnStopPlaying(wxCommandEvent &)
{

}

void ConcordanceView::OnOpenInPraat(wxCommandEvent &)
{
	auto rows = m_grid->GetSelectedRows();
	for (auto row : rows)
	{
		OpenInPraat(row);
	}

}

void ConcordanceView::OpenInPraat(int row)
{
	auto &match = m_conc->get_match(row+1);
	auto annot = match.annotation().get();

	if (!annot->is_textgrid())
	{
		wxMessageBox(_("Cannot open file in Praat because it is not a TextGrid!"), _("Type error"), wxICON_ERROR);
		return;
	}

	String sound_path;

	if (annot->has_sound()) {
		sound_path = annot->sound()->path();
	}

	try
	{
		auto target = match.reference_target();
		assert(target);
		auto interval = annot->get_event_index(target->layer, target->start_time());
		praat::open_interval(target->layer, interval, annot->path(), sound_path);
	}
	catch (std::exception &e)
	{
		wxString msg = _("Cannot open interval in Praat");
		msg.Append(wxString::FromUTF8(e.what()));
		wxMessageBox(msg, _("System error"), wxICON_ERROR);
	}
}

void ConcordanceView::OnKeyDown(wxKeyEvent &e)
{
	switch (e.GetKeyCode())
	{
		case WXK_UP:
		{
			auto sel = m_grid->GetSelectedRows();
			if (!sel.IsEmpty() && sel.front() > 0)
			{
				m_grid->SelectRow(sel.front() - 1);
			}
		} break;
		case WXK_DOWN:
		{
			auto sel = m_grid->GetSelectedRows();
			if (!sel.IsEmpty() && sel.front() < m_grid->GetNumberRows() - 1)
			{
				m_grid->SelectRow(sel.front() + 1);
			}

		} break;
		case WXK_SPACE:
		{
			auto sel = m_grid->GetSelectedRows();
			if (!sel.IsEmpty()) {
				PlayMatch(sel.front());
			}
		} break;
		default:
			e.Skip();
			break;
	}
}

void ConcordanceView::OnExportToCsv(wxCommandEvent &)
{
	String name = m_conc->label();
	name.append(".csv");

	FileDialog dlg(this, _("Save concordance as.."), name, "CSV file (*.csv)|*.csv", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}

	String path = dlg.GetPath();
	if (!path.ends_with(".csv")) path.append(".csv");
	m_conc->to_csv(path, "\t");
	AskImportFile(path);
}

void ConcordanceView::PlayMatch(int row)
{
	row++;

}

void ConcordanceView::OnViewMatch(wxCommandEvent &)
{

}

void ConcordanceView::OnBookmarkMatch(wxCommandEvent &)
{

}

void ConcordanceView::OnColumnButtonClicked(wxMouseEvent &)
{
	auto menu = new wxMenu;
	auto info_tool = menu->AppendCheckItem(wxID_ANY, _("Show file information"));
	auto meta_tool = menu->AppendCheckItem(wxID_ANY, _("Show metadata"));
	info_tool->Check(m_show_file_info);
	meta_tool->Check(m_show_metadata);
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &) { m_show_file_info = !m_show_file_info; ShowFileInfo(); }, info_tool->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [this](wxCommandEvent &) { m_show_metadata = !m_show_metadata; ShowMetadata(); }, meta_tool->GetId());

	m_toolbar->ShowMenu(m_col_tool, menu);
}

void ConcordanceView::ShowFileInfo()
{
	for (int i = 1; i <= m_conc->column_count(); i++)
	{
		if (m_conc->is_file_info_column(i))
		{
			if (m_show_file_info)
			{
				m_grid->ShowCol(i - 1);
			}
			else
			{
				m_grid->HideCol(i - 1);
			}
		}
		else
		{
			break; // file info columns are at the beginning
		}
	}
}

void ConcordanceView::ShowMetadata()
{
	for (int i = 1; i <= m_conc->column_count(); i++)
	{
		if (m_conc->is_metadata_column(i))
		{
			if (m_show_metadata)
			{
				m_grid->ShowCol(i - 1);
			}
			else
			{
				m_grid->HideCol(i - 1);
			}
		}
	}
}

void ConcordanceView::Save()
{
	if (!m_conc->has_path())
	{
		auto label = m_conc->label();
		label.append(PHON_EXT_CONCORDANCE);
		FileDialog dlg(this, _("Save concordance as..."), label, "Phonometrica concordance (*.phon-conc)|*.phon-conc",
				 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) {
			return;
		}
		auto path = dlg.GetPath();
		if (!path.ends_with(PHON_EXT_CONCORDANCE)) {
			path.append(PHON_EXT_CONCORDANCE);
		}
		m_conc->set_path(path, true);
		m_conc->modify();
		Project::get()->modify();
	}
	if (!m_conc->parent())
	{
		auto folder = Project::get()->data().get();
		folder->append(m_conc);
		Project::updated();
	}
	m_conc->save();
	m_save_tool->Disable();
	UpdateTitle();
}

void ConcordanceView::OnHelp(wxCommandEvent &)
{
	auto url = Settings::get_documentation_page("query.html");
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
}

void ConcordanceView::OnDeleteRows(wxCommandEvent &)
{
	int shift = 0;

	for (int i : m_grid->GetSelectedRows())
	{
		int row = i + 1 - shift++;
		auto cmd = std::make_unique<DeleteMatchCommand>(m_conc, row);
		command_processor.submit(std::move(cmd));
	}
	UpdateView();
}

void ConcordanceView::UpdateView()
{
	View::UpdateView();
	UpdateCountLabel();
	m_grid->Refresh();
	m_save_tool->Enable();
}

void ConcordanceView::UpdateCountLabel()
{
	auto label = wxString::Format(_("%d matches"), (int)m_conc->row_count());
	count_label->SetLabel(label);
}
} // namespace phonometrica
