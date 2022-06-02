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
 * purpose: The project manager is located on the left in the main window and displays the project's structure.        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROJECT_MANAGER_HPP
#define PHONOMETRICA_PROJECT_MANAGER_HPP

#include <wx/timer.h>
#include <wx/panel.h>
#include <wx/richtooltip.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/srchctrl.h>
#include <wx/activityindicator.h>
#include <wx/imaglist.h>
#include <wx/button.h>
#include <phon/runtime.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class ProjectManager final : public wxPanel
{
public:

	ProjectManager(Runtime &rt, wxWindow *parent);

	void OnProjectUpdated();

	void OnProjectClosed();

	void UpdateLabel();

	void StartActivity();

	void StopActivity();

	void Expand();

	static void CheckProjectImport();

	Signal<DocList> files_selected;

	Signal<const Handle<Bookmark> &> bookmark_selected;

	Signal<const Handle<Document> &> view_file;

	Signal<const String &> execute_script;

	Signal<const Handle<Query> &> edit_query;

	Signal<Directory*> new_script;

	Signal<> request_save;

private:

	void Populate();

	void ClearProject(bool set_flag);

	void UpdateProject();

	void FillFolder(wxTreeItemId item, Directory &folder);

	void OnItemSelected(wxTreeEvent &);

	void OnItemDoubleClicked(wxTreeEvent &);

	void OnRightClick(wxTreeEvent &);

	void OnMiddleClick(wxTreeEvent &);

	void OnMouseMiddleClick(wxMouseEvent &e);

	wxTreeItemId FindItem(wxPoint pos);

	wxTreeItemId FindItem(wxPoint pos, wxTreeItemId node);

	void RemoveDirectory(Handle<Directory> &folder);

	void RemoveBookmark(Handle<Bookmark> &bookmark);

	void RemoveFile(Handle<Document> &file);

	void RemoveFiles(ElementList files);

	void RenameDirectory(const Handle<Directory> &folder);

	void CreateSubdirectory(const Handle<Directory> &folder);

	void OnExpandDirectory(wxCommandEvent &);

	void OnAddFilesToDirectory(wxCommandEvent &);

	void OnCollapseDirectory(wxCommandEvent &);

	ElementList GetSelectedItems() const;

	wxTreeItemId GetSelectedId() const;

	Handle<Directory> GetSelectedFolder() const;

	Handle<Document> GetSelectedFile() const;

	void ExpandNode(wxTreeItemId node);

	void CollapseNode(wxTreeItemId node);

	void SetExpansionFlag(wxTreeItemId node);

	wxTreeItemId GetParentDirectory(wxTreeItemId item) const;

	void OnDragItem(wxTreeEvent &);

	void OnDropItem(wxTreeEvent &);

	void ConvertAnnotationToTextGrid(const Handle<Annotation> &annot);

	void ConvertTextGridToAnnotation(const Handle<Annotation> &annot);

	void OpenAnnotationInPraat(const Handle<Annotation> &annot);

	void AskImportFile(const String &path);

	void CopyTextToClipboard(const wxString &text);

	void RemoveItems(const ElementList &items);

	void SetScriptingFunctions();

	void RenameQuery(const Handle<Query> &query);

	void DuplicateQuery(const Handle<Query> &query);

	void RenameConcordance(const Handle<Concordance> &conc);

	void RenameBookmark(const Handle<Bookmark> &bookmark);

	void SaveFile(const Handle<Document> &file);

	void OnQuickSearch(wxCommandEvent &e);

	void OnProjectContextMenu(wxMouseEvent &e);

	void OnRenameProject(wxCommandEvent &);

	void OnKeyDown(wxKeyEvent &e);

	void OnShowToolTip(wxTreeEvent &e);

	void OnMouseMove(wxMouseEvent &);

	void OnTimerDone(wxTimerEvent &);

	void OnSummaryStatistics(wxCommandEvent &);

	void OnViewBookmark(wxCommandEvent &);

	void OpenBookmarkInPraat(const Handle<TimeStamp> &stamp);


	wxTreeCtrl *tree = nullptr;

	wxSearchCtrl *search_ctrl = nullptr;

	wxStaticText *main_label = nullptr;

	wxTreeItemId root, corpus_item, query_item, script_item, data_item, bookmark_item;

	wxButton *menu_btn;

	wxActivityIndicator *activity_indicator = nullptr;

	wxTimer timer;

	int corpus_img, queries_img, datasets_img, scripts_img, bookmarks_img;
	int annot_img, textgrid_img, folder_img, bookmark_img, sound_img, document_img, query_img, script_img, conc_img, csv_img;

	ElementList dragged_files;

	wxFont mono_font;

	String search_string; // used for quick search

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PROJECT_MANAGER_HPP
