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
 * Purpose: Prepare a concordance (i.e. the result of a query).                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_VIEW_HPP
#define PHONOMETRICA_CONCORDANCE_VIEW_HPP

#include <wx/grid.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <phon/gui/tool_bar.hpp>
#include <phon/gui/views/view.hpp>
#include <phon/gui/event_editor.hpp>
#include <phon/application/audio_player.hpp>
#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

class ConcordanceView : public View
{
public:

	ConcordanceView(wxWindow *parent, Handle<Concordance> conc);

	bool IsModified() const override;

	void DiscardChanges() override;

	wxString GetLabel() const override;

	String GetPath() const override;

	void Save() override;

	void Escape() override;

	bool Finalize(bool autosave) override;

	void DeleteRow(int i);

	void RestoreRow(int i);

	void UpdateRow(intptr_t i);

	void DeleteRow(intptr_t i, bool update);

	void SelectRow(intptr_t i);

	void ClearSelection();

protected:

	void Undo() override;

	void OnSave(wxCommandEvent &);

	void OnPlaySelection(wxCommandEvent &);

	void OnStopPlaying(wxCommandEvent &);

	void OnOpenInPraat(wxCommandEvent &);

	void OpenInPraat(int row);

	void OnKeyDown(wxKeyEvent &e);

	void OnExportToCsv(wxCommandEvent &);

	void OnViewMatch(wxCommandEvent &);

	void OnBookmarkMatch(wxCommandEvent &);

	void OnHelp(wxCommandEvent &);

	void OnColumnButtonClicked(wxMouseEvent &);

	void ShowFileInfo();

	void ShowMetadata();

	void PlayMatch(int row);

	void OnDeleteRows(wxCommandEvent &);

	void DeleteSelectedRows();

	void OnEditEvent(wxCommandEvent &);

	void EditCurrentEvent();

	void UpdateView() override;

	void UpdateCountLabel();

	void StopPlayer();

	void OnDoubleClick(wxGridEvent &);

	void OnRightClick(wxGridEvent &);

	void DoEventEditing();

	Match * GetSelectedMatch();

	void OnUnion(wxCommandEvent &);

	void OnIntersection(wxCommandEvent &);

	void OnComplement(wxCommandEvent &);

	void OnRename(wxCommandEvent &);

	void FillGrid();

	int GetActiveTarget() const;

	wxGrid *m_grid;

	wxStaticText *count_label;

	ToolBar *m_toolbar;

	wxButton *m_save_tool, *m_play_tool, *m_col_tool;

	wxSpinCtrl *m_active_target;

	std::unique_ptr<AudioPlayer> player;

	std::unique_ptr<EventEditor> event_editor;

	Handle<Concordance> m_conc;

	intptr_t edited_match = -1; // index in base 1

	bool m_show_file_info = true, m_show_metadata = false;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_VIEW_HPP
