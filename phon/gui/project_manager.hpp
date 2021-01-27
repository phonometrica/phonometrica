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
//#include <wx/treelist.h>
#include <wx/imaglist.h>
#include <phon/runtime.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class ProjectManager final : public wxPanel
{
public:

	ProjectManager(Runtime &rt, wxWindow *parent);

	void OnProjectUpdated();

//	void script_selected(std::shared_ptr<Script>);
//
	Signal<VFileList> files_selected;
//
//	void no_selection();
//
	Signal<const std::shared_ptr<VFile> &> view_file;

//	void view_annotation(AutoAnnotation annot, intptr_t layer, double from, double to);

private:

	void Populate();

	void ClearProject();

	void UpdateProject();

	void FillFolder(wxTreeItemId &item, VFolder &folder);

	void OnItemSelected(wxTreeEvent &);

	void OnItemDoubleClicked(wxTreeEvent &);

	void OnRightClick(wxTreeEvent &);

	VNodeList GetSelectedItems() const;

	wxTreeCtrl *m_tree = nullptr;

	wxStaticText *m_label = nullptr;

	wxTreeItemId m_root, m_corpus_item, m_query_item, m_script_item, m_data_item, m_bookmark_item;

	int m_corpus_img, m_queries_img, m_datasets_img, m_scripts_img, m_bookmarks_img;
	int m_annot_img, m_folder_img, m_bookmark_img, m_sound_img, m_document_img, m_query_img, m_script_img, m_dataset_img;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_PROJECT_MANAGER_HPP
