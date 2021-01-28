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
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/menu.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>
#include <phon/gui/project_manager.hpp>
#include <phon/include/icons.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

struct ItemData final : public wxTreeItemData
{
	ItemData(VNode *n) :
			wxTreeItemData(), node(n) { }

	~ItemData() override = default;

	VNode *node = nullptr;
};

ProjectManager::ProjectManager(Runtime &rt, wxWindow *parent) :
	wxPanel(parent), runtime(rt)
{
	m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT|wxTR_MULTIPLE|wxTR_NO_LINES|wxTR_DEFAULT_STYLE);
	m_label = new wxStaticText(this, wxID_ANY, _("Project"), wxDefaultPosition, wxDefaultSize);
	auto sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(m_label, 0, wxEXPAND|wxTOP|wxLEFT, 7);
	sizer->Add(m_tree, 10, wxEXPAND|wxTOP, 5);
	auto font = m_label->GetFont();
	font.MakeBold();
	m_label->SetFont(font);
	m_label->SetForegroundColour(wxColor(75, 75, 75));

	SetSizer(sizer);
	auto images = new wxImageList(16, 16);
	m_corpus_img = images->Add(wxBITMAP_PNG_FROM_DATA(corpus));
	m_queries_img = images->Add(wxBITMAP_PNG_FROM_DATA(search));
	m_datasets_img = images->Add(wxBITMAP_PNG_FROM_DATA(data));
	m_scripts_img = images->Add(wxBITMAP_PNG_FROM_DATA(console));
	m_bookmarks_img = images->Add(wxBITMAP_PNG_FROM_DATA(favorite));
	m_annot_img = images->Add(wxBITMAP_PNG_FROM_DATA(annotation));
#ifdef __WXMSW__
	m_folder_img = images->Add(wxBITMAP_PNG_FROM_DATA(folder));
#elif defined(__WXMAC__)
	m_folder_img = images->Add(wxBITMAP_PNG_FROM_DATA(folder_mac));
#else
	m_folder_img = images->Add(wxArtProvider::GetIcon(wxART_FOLDER));
#endif
	m_bookmark_img = images->Add(wxBITMAP_PNG_FROM_DATA(bookmark));
	m_sound_img = images->Add(wxBITMAP_PNG_FROM_DATA(sound));
	m_script_img = images->Add(wxBITMAP_PNG_FROM_DATA(script));
	m_document_img = images->Add(wxBITMAP_PNG_FROM_DATA(document));
	m_query_img = images->Add(wxBITMAP_PNG_FROM_DATA(query));
	m_dataset_img = images->Add(wxBITMAP_PNG_FROM_DATA(dataset));
	m_tree->SetImageList(images);
	m_root = m_tree->AddRoot(_("Project"));
#if PHON_LINUX
	auto col = m_label->GetBackgroundColour();
#else
	auto col = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);
#endif
	this->SetBackgroundColour(col);
	m_label->SetBackgroundColour(col);
	m_tree->SetBackgroundColour(col);

	m_root = m_tree->GetRootItem();
	Populate();

	m_tree->Bind(wxEVT_TREE_BEGIN_DRAG, &ProjectManager::OnDragItem, this);
	m_tree->Bind(wxEVT_TREE_END_DRAG, &ProjectManager::OnDropItem, this);
	Bind(wxEVT_TREE_SEL_CHANGED, &ProjectManager::OnItemSelected, this);
	Bind(wxEVT_TREE_ITEM_ACTIVATED, &ProjectManager::OnItemDoubleClicked, this);
	Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &ProjectManager::OnRightClick, this);
}

void ProjectManager::Populate()
{
	auto p = Project::get();
	m_corpus_item = m_tree->AppendItem(m_root, _("Corpus"), m_corpus_img, m_corpus_img, new ItemData(p->corpus().get()));
	m_query_item = m_tree->AppendItem(m_root, _("Queries"), m_queries_img, m_queries_img, new ItemData(p->queries().get()));
	m_data_item = m_tree->AppendItem(m_root, _("Datasets"), m_datasets_img, m_datasets_img, new ItemData(p->data().get()));
	m_script_item = m_tree->AppendItem(m_root, _("Scripts"), m_scripts_img, m_scripts_img, new ItemData(p->scripts().get()));
	m_bookmark_item = m_tree->AppendItem(m_root, _("Bookmarks"), m_bookmarks_img, m_bookmarks_img, new ItemData(p->bookmarks().get()));
}

void ProjectManager::OnProjectUpdated()
{
	ClearProject();
	UpdateProject();
}

void ProjectManager::UpdateProject()
{
	auto project = Project::get();
	if (project->empty()) {
		return;
	}

	UpdateLabel();

	FillFolder(m_corpus_item, *project->corpus());
	FillFolder(m_data_item, *project->data());
	FillFolder(m_query_item, *project->queries());
	FillFolder(m_script_item, *project->scripts());
	FillFolder(m_bookmark_item, *project->bookmarks());

	m_tree->Expand(m_corpus_item);
	m_tree->Expand(m_query_item);
	m_tree->Expand(m_data_item);
	m_tree->Expand(m_script_item);
	m_tree->Expand(m_bookmark_item);
}

void ProjectManager::ClearProject()
{
	SetExpansionFlag(m_corpus_item);
	SetExpansionFlag(m_query_item);
	SetExpansionFlag(m_data_item);
	SetExpansionFlag(m_script_item);
	SetExpansionFlag(m_bookmark_item);

	m_tree->DeleteChildren(m_corpus_item);
	m_tree->DeleteChildren(m_query_item);
	m_tree->DeleteChildren(m_data_item);
	m_tree->DeleteChildren(m_script_item);
	m_tree->DeleteChildren(m_bookmark_item);
}

void ProjectManager::FillFolder(wxTreeItemId item, VFolder &folder)
{
	for (int i = 1; i <= folder.size(); i++)
	{
		auto &node = folder.get(i);

		if (node->is_folder())
		{
			auto &subfolder = dynamic_cast<VFolder&>(*node);
			auto data = new ItemData(&subfolder);
			auto child = m_tree->AppendItem(item, node->label(), m_folder_img, m_folder_img, data);
			FillFolder(child, subfolder);
		}
		else if (node->is_bookmark())
		{
//			auto &bookmark = dynamic_cast<Bookmark&>(*node);
			auto data = new ItemData(node.get());
			m_tree->AppendItem(item, node->label(), m_bookmark_img, m_bookmark_img, data);
		}
		else
		{
			auto &vfile = dynamic_cast<VFile&>(*node);
			int img;

			if (vfile.is_annotation())
			{
				img = m_annot_img;
			}
			else if (vfile.is_sound())
			{
				img = m_sound_img;
			}
			else if (vfile.is_script())
			{
				img = m_script_img;
			}
			else if (vfile.is_dataset())
			{
				img = m_dataset_img;
			}
			else if (vfile.is_document())
			{
				img = m_document_img;
			}
			else
			{
				img = m_document_img;
			}
			auto data = new ItemData(node.get());
			m_tree->AppendItem(item, node->label(), img, img, data);
		}
	}

	if (folder.expanded()) {
		m_tree->Expand(item);
	}
}

void ProjectManager::OnItemSelected(wxTreeEvent &)
{
	wxArrayTreeItemIds items;
	m_tree->GetSelections(items);
	VFileList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(item));
		if (!data) return; // should never happen
		auto vnode = data->node;
		assert(vnode);

		if (vnode->is_file())
		{
			auto vf = downcast<VFile>(vnode->shared_from_this());
			files.append(std::move(vf));
		}
	}
	files_selected(std::move(files));
}

void ProjectManager::OnItemDoubleClicked(wxTreeEvent &e)
{
	auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(e.GetItem()));
	if (!data) return; // should never happen
	auto vnode = data->node;
	auto id = data->GetId();
	assert(vnode);

	if (vnode->is_folder())
	{
		if (m_tree->IsExpanded(id))
		{
			m_tree->Collapse(id);
		}
		else
		{
			m_tree->Expand(id);
		}
	}
	else
	{
		auto vf = downcast<VFile>(vnode->shared_from_this());
		view_file(vf);
	}
}

void ProjectManager::OnRightClick(wxTreeEvent &)
{
	auto items = GetSelectedItems();
	auto menu = new wxMenu;
	auto project = Project::get();

	if (items.size() == 1)
	{
		auto &item = items.front();

		if (item->is_folder())
		{
			auto folder = downcast<VFolder>(item);
			wxArrayTreeItemIds ids;
			m_tree->GetSelections(ids);
			auto tree_item = ids.front();

			auto expand_id = wxNewId();
			menu->Append(expand_id, _("Expand content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnExpandDirectory, this, expand_id);
			auto collapse_id = wxNewId();
			menu->Append(collapse_id, _("Collapse content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnCollapseDirectory, this, collapse_id);
			menu->AppendSeparator();

			if (GetParentDirectory(tree_item) != m_bookmark_item)
			{
				auto add_files_id = wxNewId();
				menu->Append(add_files_id, _("Add files to directory..."));
				Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnAddFilesToDirectory, this, add_files_id);
				menu->AppendSeparator();
			}

			auto subfolder_id = wxNewId();
			menu->Append(subfolder_id, _("Create subdirectory..."));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) { CreateSubdirectory(folder); }, subfolder_id);

			if (!project->is_root(folder.get()))
			{
				auto rename_id = wxNewId();
				menu->Append(rename_id, _("Rename directory..."));
				auto remove_id = wxNewId();
				menu->AppendSeparator();
				menu->Append(remove_id, _("Remove directory"));

				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) mutable { RemoveDirectory(folder); }, remove_id);
				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) { RenameDirectory(folder); }, rename_id);
			}
		}
		else
		{
			auto file = downcast<VFile>(item);
			auto view_id = wxNewId();
			menu->Append(view_id, _("View file"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,file](wxCommandEvent &) { view_file(file); }, view_id);

			auto remove_id = wxNewId();
			menu->Append(remove_id, _("Remove file"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,file](wxCommandEvent &) mutable { RemoveFile(file); }, remove_id);
		}
	}

	PopupMenu(menu);
}

VNodeList ProjectManager::GetSelectedItems() const
{
	wxArrayTreeItemIds items;
	m_tree->GetSelections(items);
	VNodeList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(item));
		if (!data) {
			wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
			return files;
		}
		auto vnode = data->node;
		files.append(vnode->shared_from_this());
	}

	return files;
}

std::shared_ptr<VFolder> ProjectManager::GetSelectedFolder() const
{
	wxArrayTreeItemIds ids;
	m_tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(id));
	if (!data) {
		wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
		return nullptr;
	}
	auto vnode = data->node;

	return downcast<VFolder>(vnode->shared_from_this());
}

std::shared_ptr<VFile> ProjectManager::GetSelectedFile() const
{
	wxArrayTreeItemIds ids;
	m_tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(id));
	if (!data) {
		wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
		return nullptr;
	}
	auto vnode = data->node;

	return downcast<VFile>(vnode->shared_from_this());
}

void ProjectManager::RemoveDirectory(std::shared_ptr<VFolder> &folder)
{
	Project::get()->remove(folder);
	Project::updated();
}

void ProjectManager::RemoveFile(std::shared_ptr<VFile> &file)
{
	Project::get()->remove(file);
	Project::updated();
}

void ProjectManager::RenameDirectory(const std::shared_ptr<VFolder> &folder)
{
	String name = wxGetTextFromUser(_("New directory name:"), _("Rename directory..."));

	if (!name.empty())
	{
		folder->set_label(name);
		Project::updated();
	}
}

void ProjectManager::CreateSubdirectory(const std::shared_ptr<VFolder> &folder)
{
	String name = wxGetTextFromUser(_("Directory name:"), _("New directory..."));

	if (!name.empty())
	{
		folder->add_subfolder(name);
		Project::updated();
	}
}

void ProjectManager::UpdateLabel()
{
	auto label = Project::get()->label();
	if (Project::get()->modified()) {
		label.append('*');
	}
	m_label->SetLabel(label);
}

void ProjectManager::OnExpandDirectory(wxCommandEvent &e)
{
	wxArrayTreeItemIds ids;
	m_tree->GetSelections(ids);
	auto id = ids.front();
	ExpandNode(id);
}

void ProjectManager::OnCollapseDirectory(wxCommandEvent &)
{
	wxArrayTreeItemIds ids;
	m_tree->GetSelections(ids);
	auto id = ids.front();
	CollapseNode(id);
}

void ProjectManager::ExpandNode(wxTreeItemId node)
{
	m_tree->Expand(node);
	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_tree->GetFirstChild(node, cookie);

	while (child.IsOk())
	{
		ExpandNode(child);
		child = m_tree->GetNextChild(node, cookie);
	}
}

void ProjectManager::CollapseNode(wxTreeItemId node)
{
	m_tree->Collapse(node);
	wxTreeItemIdValue cookie;
	wxTreeItemId child = m_tree->GetFirstChild(node, cookie);

	while (child.IsOk())
	{
		CollapseNode(child);
		child = m_tree->GetNextChild(node, cookie);
	}
}

void ProjectManager::SetExpansionFlag(wxTreeItemId node)
{
	auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(node));
	assert(data);
	auto vnode = data->node;
	assert(node);

	if (vnode->is_folder())
	{
		auto vfolder = dynamic_cast<VFolder*>(vnode);
		vfolder->set_expanded(m_tree->IsExpanded(node));
		wxTreeItemIdValue cookie;
		wxTreeItemId child = m_tree->GetFirstChild(node, cookie);

		while (child.IsOk())
		{
			SetExpansionFlag(child);
			child = m_tree->GetNextChild(node, cookie);
		}
	}
}

void ProjectManager::OnAddFilesToDirectory(wxCommandEvent &)
{
	wxFileDialog dlg(this, _("Add files to directory..."), "", "", "Phonometrica files (*.*)|*.*",
				  wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST);

	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	wxArrayString paths;
	dlg.GetPaths(paths);
	auto project = Project::get();
	auto folder = GetSelectedFolder();

	// Only allow adding files of a relevant type. Other files are ignored.
	FileType type = FileType::Any;
	auto root = folder->toplevel();

	if (root == project->corpus().get()) {
		type = FileType::CorpusFile;
	}
	else if (root == project->scripts().get()) {
		type = FileType::Script;
	}
	else if (root == project->data().get()) {
		type = FileType::Dataset;
	}
	else if (root == project->queries().get()) {
		type = FileType::Query;
	}

	for (auto &path : paths)
	{
		project->add_file(path, folder, type);
	}
	folder->set_expanded(true);
	CheckProjectImport();

	// Files are added silently, so we need to explicitly modify the project
	project->modify();
	Project::updated();
}

wxTreeItemId ProjectManager::GetParentDirectory(wxTreeItemId item) const
{
	while (true)
	{
		auto tmp = m_tree->GetItemParent(item);
		if (tmp == m_root) {
			return item;
		}
		item = tmp;
	}
}

void ProjectManager::OnDragItem(wxTreeEvent &e)
{
	auto items = GetSelectedItems();

	for (auto &item : items)
	{
		if (Project::get()->is_root(item->toplevel())) {
			return;
		}
	}

	m_dragged_items.Clear();
	m_tree->GetSelections(m_dragged_items);
	e.Allow();
}

void ProjectManager::OnDropItem(wxTreeEvent &e)
{

}

void ProjectManager::CheckProjectImport()
{
	if (Project::get()->import_flag()) {
		wxMessageBox(_("Some files were ignored or already present in the corpus."), _("Files ignored"), wxICON_INFORMATION);
	}
	Project::get()->clear_import_flag();
}

} // namespace phonometrica