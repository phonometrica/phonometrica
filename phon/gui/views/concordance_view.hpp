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
#include <wx/toolbar.h>
#include <phon/gui/views/view.hpp>
#include <phon/gui/ctrl/concordance_controller.hpp>

namespace phonometrica {

class ConcordanceView : public View
{
public:

	ConcordanceView(wxWindow *parent, AutoConcordance conc);

	virtual bool IsModified() const override;

	virtual void DiscardChanges() override;

	virtual wxString GetLabel() const override;

	virtual String GetPath() const override;

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

	void OnColumnButtonClicked(wxCommandEvent &);

	void PlayMatch(int row);

	wxGrid *m_grid;

	wxStaticText *count_label;

	wxToolBar *m_toolbar;

	wxToolBarToolBase *m_save_tool, *m_play_tool, *m_col_tool;

	AutoConcordance m_conc;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_VIEW_HPP
