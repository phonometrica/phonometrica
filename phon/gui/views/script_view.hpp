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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: View for scripts.                                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SCRIPT_VIEW_HPP
#define PHONOMETRICA_SCRIPT_VIEW_HPP

#include <wx/toolbar.h>
#include <phon/gui/views/view.hpp>
#include <phon/gui/script_ctrl.hpp>
#include <phon/runtime.hpp>
#include <phon/application/script.hpp>

namespace phonometrica {

class ScriptView final : public View
{
public:

	ScriptView(Runtime &rt, const AutoScript &script, wxWindow *parent);

	void Save() override;

	void Run() override;

	void AdjustFontSize() override;

	String path() const override;

	bool IsModified() const override;

	void DiscardChanges() override;

	wxString GetLabel() const override;

private:

	void SetupUi();

	void OnModification();

	void OnCommentSelection(wxCommandEvent &);

	void OnUncommentSelection(wxCommandEvent &);

	void OnIndentSelection(wxCommandEvent &);

	void OnUnindentSelection(wxCommandEvent &);

	void AddStartCharacter(const wxString &s);

	void RemoveStartCharacter(const wxString &s);

	void OnOpenHelp(wxCommandEvent &);

	void OnViewBytecode(wxCommandEvent &);

	ScriptControl *m_ctrl;

	wxToolBar *m_toolbar;

	wxToolBarToolBase *m_save_tool;

	AutoScript m_script;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SCRIPT_VIEW_HPP
