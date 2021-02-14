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

#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/srchctrl.h>
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

	static void CheckProjectImport();

	Signal<VFileList> files_selected;

	Signal<const std::shared_ptr<VFile> &> view_file;

	Signal<const String &> execute_script;

	Signal<const AutoQuery &> edit_query;

	Signal<VFolder*> new_script;

private:

	void Populate();

	void ClearProject(bool set_flag);

	void UpdateProject();

	void FillFolder(wxTreeItemId item, VFolder &folder);

	void OnItemSelected(wxTreeEvent &);

	void OnItemDoubleClicked(wxTreeEvent &);

	void OnRightClick(wxTreeEvent &);

	void OnMiddleClick(wxTreeEvent &);

	void RemoveDirectory(std::shared_ptr<VFolder> &folder);

	void RemoveFile(std::shared_ptr<VFile> &file);

	void RemoveFiles(VNodeList files);

	void RenameDirectory(const std::shared_ptr<VFolder> &folder);

	void CreateSubdirectory(const std::shared_ptr<VFolder> &folder);

	void OnExpandDirectory(wxCommandEvent &);

	void OnAddFilesToDirectory(wxCommandEvent &);

	void OnCollapseDirectory(wxCommandEvent &);

	VNodeList GetSelectedItems() const;

	wxTreeItemId GetSelectedId() const;

	std::shared_ptr<VFolder> GetSelectedFolder() const;

	std::shared_ptr<VFile> GetSelectedFile() const;

	void ExpandNode(wxTreeItemId node);

	void CollapseNode(wxTreeItemId node);

	void SetExpansionFlag(wxTreeItemId node);

	wxTreeItemId GetParentDirectory(wxTreeItemId item) const;

	void OnDragItem(wxTreeEvent &);

	void OnDropItem(wxTreeEvent &);

	void ConvertAnnotationToTextGrid(const AutoAnnotation &annot);

	void ConvertTextGridToAnnotation(const AutoAnnotation &annot);

	void OpenAnnotationInPraat(const AutoAnnotation &annot);

	void AskImportFile(const String &path);

	void CopyTextToClipboard(const wxString &text);

	void RemoveItems(const VNodeList &items);

	void SetScriptingFunctions();

	void RenameQuery(const AutoQuery &query);

	void DuplicateQuery(const AutoQuery &query);

	void SaveFile(const std::shared_ptr<VFile> &file);

	void OnQuickSearch(wxCommandEvent &e);

	void OnProjectContextMenu(wxMouseEvent &e);

	void OnRenameProject(wxCommandEvent &);

	void OnKeyDown(wxKeyEvent &e);

#ifdef __WXMSW__
	void OnShowToolTip(wxTreeEvent &e);
#endif

	wxTreeCtrl *tree = nullptr;

	wxSearchCtrl *search_ctrl = nullptr;

	wxStaticText *main_label = nullptr;

	wxTreeItemId root, corpus_item, query_item, script_item, data_item, bookmark_item;

	wxButton *menu_btn;

	int corpus_img, queries_img, datasets_img, scripts_img, bookmarks_img;
	int annot_img, textgrid_img, folder_img, bookmark_img, sound_img, document_img, query_img, script_img, conc_img, csv_img;

	VNodeList dragged_files;

	wxFont mono_font;

	String search_string; // used for quick search

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PROJECT_MANAGER_HPP
