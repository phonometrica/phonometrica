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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: The viewer occupies the center of the main window. It is a notebook that can display views as tabs, like   *
 * in a web browser.                                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VIEWER_HPP
#define PHONOMETRICA_VIEWER_HPP

#include <wx/aui/auibook.h>
#include <phon/runtime.hpp>
#include <phon/gui/views/script_view.hpp>
#include <phon/gui/views/start_view.hpp>
#include <phon/application/annotation.hpp>
#include <phon/application/sound.hpp>
#include <phon/application/script.hpp>

namespace phonometrica {

class MainWindow;

class Viewer final : public wxAuiNotebook
{
public:

	Viewer(Runtime &rt, wxWindow *parent, MainWindow *win);

	void SetStartView();

	void NewScript();

	void NewScriptWithParent(Directory *parent);

	void CloseCurrentView();

	View *GetCurrentView() const;

	void ViewFile(const Handle<Document> &file);

	void AdjustFontSize();

	bool SaveViews(bool autosave);

	void CloseViews();

	View *GetView(size_t i) const { return dynamic_cast<View*>(GetPage(i)); }

	void UpdateCurrentView();

	void UpdateLabels();

	Handle<Sound> GetCurrentSound() const;

	Handle<Annotation> GetCurrentAnnotation() const;

	void OpenAnnotation(const Handle<Annotation> &annot, intptr_t layer, const AutoEvent &event);

	double GetWindowDuration() const;

	double GetSelectionDuration() const;

	Signal<> request_console;

	// Work around a bug on Windows, whereby the application goes to the background when a concordance is displayed.
	Signal<> wake_up;

private:

	void AddView(View *view, const wxString &title);

	void CloseView(int index, bool remove);

	void OnCloseView(wxAuiNotebookEvent &);

	void OnViewClosed(wxAuiNotebookEvent &);

	void NewScript(const Handle<Script> &script);

	void OnPageChanged(wxAuiNotebookEvent &);

	void OnMainCloseButtonClicked(wxAuiNotebookEvent &);

	bool OpenFile(const Handle<Document> &file);

	void OnEditEvent(const Handle<Annotation> &annot, const AutoEvent &event, const String &new_value);

	// Used to set bindings.
	MainWindow *main_window = nullptr;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_VIEWER_HPP
