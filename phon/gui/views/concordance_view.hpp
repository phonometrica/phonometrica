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
 * Purpose: Display a concordance (i.e. the result of a query).                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_VIEW_HPP
#define PHONOMETRICA_CONCORDANCE_VIEW_HPP

#include <wx/stattext.h>
#include <phon/gui/tool_bar.hpp>
#include <phon/gui/views/view.hpp>
#include <phon/gui/event_editor.hpp>
#include <phon/gui/ctrl/concordance_controller.hpp>
#include <phon/application/audio_player.hpp>

namespace phonometrica {

class ConcordanceView : public View
{
public:

	ConcordanceView(wxWindow *parent, AutoConcordance conc);

	bool IsModified() const override;

	void DiscardChanges() override;

	wxString GetLabel() const override;

	String GetPath() const override;

	void Save() override;

	void Escape() override;

protected:

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

	void OnEditEvent(wxCommandEvent &);

	void EditCurrentEvent();

	void UpdateView() override;

	void UpdateCountLabel();

	void StopPlayer();

	void ResetPlayer();

	void OnDoubleClick(wxGridEvent &);

	void DeleteEventEditor();

	Match * GetSelectedMatch();

	wxGrid *m_grid;

	wxStaticText *count_label;

	ToolBar *m_toolbar;

	wxButton *m_save_tool, *m_play_tool, *m_col_tool;

	AudioPlayer *player = nullptr;

	EventEditor *event_editor = nullptr;

	AutoConcordance m_conc;

	bool m_show_file_info = true, m_show_metadata = false;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_VIEW_HPP
