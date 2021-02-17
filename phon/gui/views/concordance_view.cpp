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
#include <phon/gui/conc/concordance_join_dialog.hpp>
#include <phon/application/cmd/delete_match_command.hpp>
#include <phon/application/settings.hpp>
#include <phon/include/icons.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/praat.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

ConcordanceView::ConcordanceView(wxWindow *parent, Handle<Concordance> conc) :
	View(parent), m_conc(std::move(conc))
{
#define ICN(x) wxBITMAP_PNG_FROM_DATA(x)

	auto sizer = new VBoxSizer;

	m_toolbar = new ToolBar(this);

	auto save_icon = ICN(save);
	m_save_tool = m_toolbar->AddButton(save_icon, _("Save concordance... (" CTRL_KEY "S)"));
	auto csv_tool = m_toolbar->AddButton(ICN(export_csv), _("Export concordance to CSV..."));

	auto play_icon = ICN(play);
	m_play_tool = m_toolbar->AddButton(play_icon, _("Play selection"));
	auto stop_tool = m_toolbar->AddButton(ICN(stop), _("Stop playing"));

	auto praat_tool = m_toolbar->AddButton(ICN(praat), _("Open selection in Praat"));
	auto view_tool = m_toolbar->AddButton(ICN(eye), _("Open match in annotation"));

	auto del_row_tool = m_toolbar->AddButton(ICN(delete_row), _("Delete selected row(s)"));
	auto edit_row_tool = m_toolbar->AddButton(ICN(edit_row), _("Edit selected event"));

	auto union_tool = m_toolbar->AddButton(ICN(query_union), _("Unite concordance... (matches in A or B)"));
	auto intersection_tool = m_toolbar->AddButton(ICN(query_intersection), _("Intersect concordance... (matches in A and B)"));
	auto complement_tool = m_toolbar->AddButton(ICN(query_complement), _("Get complement of concordance... (matches in B not A)"));

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
	edit_row_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnEditEvent, this);
	bookmark_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnBookmarkMatch, this);
	help_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnHelp, this);
	union_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnUnion, this);
	intersection_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnIntersection, this);
	complement_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ConcordanceView::OnComplement, this);
	m_col_tool->Bind(wxEVT_LEFT_DOWN, &ConcordanceView::OnColumnButtonClicked, this);
	m_grid->Bind(wxEVT_KEY_DOWN, &ConcordanceView::OnKeyDown, this);
	m_grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &ConcordanceView::OnDoubleClick, this);
	m_grid->Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &ConcordanceView::OnRightClick, this);
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
	auto sel = m_grid->GetSelectedRows();
	if (sel.size() != 1)
	{
		if (sel.empty())
		{
			wxMessageBox(_("You must first select a match"), _("Error"), wxICON_INFORMATION);
		}
		else
		{
			wxMessageBox(_("You can only play one match at a time"), _("Error"), wxICON_INFORMATION);
		}
		return;
	}

	PlayMatch(sel.front());
}

void ConcordanceView::OnStopPlaying(wxCommandEvent &)
{
	StopPlayer();
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
			if (sel.size() == 1) {
				PlayMatch(sel.front());
			}
		} break;
		case WXK_RETURN:
		case WXK_NUMPAD_ENTER:
		{
			EditCurrentEvent();
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
	auto &match = m_conc->get_match(row + 1);
	auto sound = match.annotation()->sound();
	if (!sound) return;
	StopPlayer();
	player = new AudioPlayer(sound->light_data());
	player->done.connect(&ConcordanceView::ResetPlayer, this);
	double from = match.get_start_time(1);
	double to = match.get_end_time(1);
	player->play(from, to);
}

void ConcordanceView::OnViewMatch(wxCommandEvent &)
{
	wxMessageBox(_("Not implemented yet!"), _(""), wxICON_INFORMATION);
}

void ConcordanceView::OnBookmarkMatch(wxCommandEvent &)
{
	wxMessageBox(_("Not implemented yet!"), _(""), wxICON_INFORMATION);
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
	auto sel = m_grid->GetSelectedRows();
	if (sel.empty()) {
		wxMessageBox(_("No row selected"), _("Empty selection"), wxICON_INFORMATION);
		return;
	}
	int shift = 0;
	int first_row = sel.front();
	int row = first_row + 1 - shift++;
	auto cmd = std::make_unique<DeleteMatchCommand>(m_conc, row);

	for (int i = 1; i < (int)sel.size(); i++)
	{
		row = i + 1 - shift++;
		cmd->append(std::make_unique<DeleteMatchCommand>(m_conc, row));

	}
	command_processor.submit(std::move(cmd));
	m_grid->ClearSelection();
	m_grid->SelectRow(first_row);
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

void ConcordanceView::StopPlayer()
{
	if (player)
	{
		player->stop();
		player = nullptr;
	}
}

void ConcordanceView::ResetPlayer()
{
	player = nullptr;
}

void ConcordanceView::OnDoubleClick(wxGridEvent &)
{
	wxCommandEvent dummy;
	OnPlaySelection(dummy);
}

void ConcordanceView::OnEditEvent(wxCommandEvent &)
{
	EditCurrentEvent();
}

void ConcordanceView::EditCurrentEvent()
{
	DeleteEventEditor();

	auto match = GetSelectedMatch();
	if (!match) {
		return;
	}

	// Find first cell that contains a match on the selected the row so that we can position the editor there.
	wxSize size(500, 60);
	int j;
	for (j = 1; j <= m_conc->column_count(); j++) {
		if (m_conc->is_target(j)) break;
	}
	auto sel = m_grid->GetSelectedRows();
	auto rect = m_grid->CellToRect(sel.front(), j);
	auto pos = rect.GetPosition();
	pos.x -= size.GetWidth() / 2;
	pos.y += size.GetHeight() / 2;

	auto offset = match->get_offset(1);
	auto len = match->get_value(1).size();
	edited_match = m_grid->GetSelectedRows().front() + 1; // index in base 1
	event_editor = new EventEditor(this, match->annotation(), match->get_event(1), offset, len, pos, size);
	event_editor->done.connect(&ConcordanceView::EndMatchEditing, this);
}

void ConcordanceView::Escape()
{
	DeleteEventEditor();
	StopPlayer();
}

void ConcordanceView::DeleteEventEditor()
{
	if (event_editor)
	{
		delete event_editor;
		event_editor = nullptr;
		edited_match = 0;
		m_grid->SetFocus();
	}
}

Match * ConcordanceView::GetSelectedMatch()
{
	auto sel = m_grid->GetSelectedRows();
	if (sel.size() != 1) {
		wxMessageBox(_("You must select exactly one row!"), _("Selection too large"), wxICON_INFORMATION);
		return nullptr;
	}

	return &m_conc->get_match(sel.front() + 1);
}

void ConcordanceView::OnUnion(wxCommandEvent &)
{
	ConcordanceJoinDialog dlg(this, _("Unite concordances..."));
	if (dlg.ShowModal() != wxID_OK) {
		return;
	}

	String label = dlg.GetLabel();
	auto other = dlg.GetConcordance();
	other->open();

	try
	{
		auto result = m_conc->unite(*other, label);
		file_created(result);
		Project::updated();
	}
	catch (std::exception &e)
	{
		auto msg = wxString::Format(_("Concordance union failed: %s"), e.what());
		wxMessageBox(msg, _("Error"), wxICON_ERROR);
	}
}

void ConcordanceView::OnIntersection(wxCommandEvent &)
{
	ConcordanceJoinDialog dlg(this, _("Intersect concordances..."));
	if (dlg.ShowModal() != wxID_OK) {
		return;
	}

	String label = dlg.GetLabel();
	auto other = dlg.GetConcordance();
	other->open();

	try
	{
		auto result = m_conc->intersect(*other, label);
		file_created(result);
		Project::updated();
	}
	catch (std::exception &e)
	{
		auto msg = wxString::Format(_("Concordance union failed: %s"), e.what());
		wxMessageBox(msg, _("Error"), wxICON_ERROR);
	}
}

void ConcordanceView::OnComplement(wxCommandEvent &)
{
	ConcordanceJoinDialog dlg(this, _("Get complement of concordance..."));
	if (dlg.ShowModal() != wxID_OK) {
		return;
	}

	String label = dlg.GetLabel();
	auto other = dlg.GetConcordance();
	other->open();

	try
	{
		auto result = m_conc->complement(*other, label);
		file_created(result);
		Project::updated();
	}
	catch (std::exception &e)
	{
		auto msg = wxString::Format(_("Concordance complement failed: %s"), e.what());
		wxMessageBox(msg, _("Error"), wxICON_ERROR);
	}
}

void ConcordanceView::OnRightClick(wxGridEvent &e)
{
	m_grid->ClearSelection();
	m_grid->SelectRow(e.GetRow());
	auto match = GetSelectedMatch();
	auto menu = new wxMenu;

	if (match->annotation()->has_sound())
	{
		auto play_item = menu->Append(wxNewId(), _("Play match"));
		auto view_item = menu->Append(wxNewId(), _("View match in annotation"));
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnPlaySelection, this, play_item->GetId());
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnViewMatch, this, view_item->GetId());
	}
	if (match->annotation()->is_textgrid())
	{
		auto praat_item = menu->Append(wxNewId(), _("Open match in Praat"));
		Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnOpenInPraat, this, praat_item->GetId());
	}
	menu->AppendSeparator();
	auto edit_item = menu->Append(wxNewId(), _("Edit matched event"));
	auto remove_item = menu->Append(wxNewId(), _("Remove match"));
	menu->AppendSeparator();
	auto bookmark_item = menu->Append(wxNewId(), _("Bookmark match"));

	Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnEditEvent, this, edit_item->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnDeleteRows, this, remove_item->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ConcordanceView::OnBookmarkMatch, this, bookmark_item->GetId());
	auto pos = e.GetPosition();
	pos.y += 60;
	PopupMenu(menu, pos);
}

void ConcordanceView::EndMatchEditing()
{
	// This is a slot connected to EventEditor::done.

	if (!m_conc->update_match(edited_match))
	{
		wxMessageBox(_("The current match is no longer valid and will be deleted"),
			   _("Invalid match"), wxICON_WARNING);
		DeleteRow(edited_match, true);
	}

	DeleteEventEditor();
}

void ConcordanceView::DeleteRow(intptr_t i, bool update)
{
	auto cmd = std::make_unique<DeleteMatchCommand>(m_conc, i);
	command_processor.submit(std::move(cmd));
	if (update) {
		UpdateView();
	}
}

} // namespace phonometrica
