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
#include <wx/clipbrd.h>
#include <phon/gui/dialog.hpp>
#include <phon/gui/project_manager.hpp>
#include <phon/include/icons.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/praat.hpp>
#include <phon/utils/file_system.hpp>

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
	tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT | wxTR_MULTIPLE | wxTR_NO_LINES | wxTR_DEFAULT_STYLE);
	mono_font = Settings::get_mono_font();
	mono_font.SetPointSize(tree->GetFont().GetPointSize());

	main_label = new wxStaticText(this, wxID_ANY, _("Project"), wxDefaultPosition, wxDefaultSize);
	auto font = main_label->GetFont();
	font.MakeBold();
	main_label->SetFont(font);
	main_label->SetForegroundColour(wxColor(75, 75, 75));

	search_ctrl = new wxSearchCtrl(this, wxID_ANY);
	search_ctrl->SetDescriptiveText(_("Search files"));
	search_ctrl->SetToolTip(_("Find files in your project based on their name or metadata"));

	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->Add(main_label, 0, wxALIGN_CENTER);
	hsizer->AddStretchSpacer();
	menu_btn = new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	menu_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(menu));
	menu_btn->SetMaxSize(wxSize(30, 30));
	menu_btn->SetToolTip(_("Show project menu"));
	hsizer->Add(menu_btn, 0, wxRIGHT, 10);


	auto sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXGTK__
	sizer->AddSpacer(2);
#endif
	sizer->Add(hsizer, 0, wxEXPAND | wxLEFT, 8);
	sizer->Add(tree, 10, wxEXPAND | wxTOP, 5);
#ifdef __WXMSW__
	int search_spacing = 2;
#else
    int search_spacing = 10;
#endif
	sizer->Add(search_ctrl, 0, wxEXPAND|wxALL, search_spacing);

	SetSizer(sizer);
	auto images = new wxImageList(16, 16);
	corpus_img = images->Add(wxBITMAP_PNG_FROM_DATA(corpus));
	queries_img = images->Add(wxBITMAP_PNG_FROM_DATA(search));
	datasets_img = images->Add(wxBITMAP_PNG_FROM_DATA(data));
	scripts_img = images->Add(wxBITMAP_PNG_FROM_DATA(console));
	bookmarks_img = images->Add(wxBITMAP_PNG_FROM_DATA(favorite));
	annot_img = images->Add(wxBITMAP_PNG_FROM_DATA(annotation));
#ifdef __WXMSW__
	folder_img = images->Add(wxBITMAP_PNG_FROM_DATA(folder));
#elif defined(__WXMAC__)
//	folder_img = images->Add(wxArtProvider::GetIcon(wxART_FOLDER, wxART_OTHER, wxSize(16, 16)));
	folder_img = images->Add(wxBITMAP_PNG_FROM_DATA(folder_mac));
#else
	folder_img = images->Add(wxArtProvider::GetIcon(wxART_FOLDER));
#endif
	bookmark_img = images->Add(wxBITMAP_PNG_FROM_DATA(bookmark));
	sound_img = images->Add(wxBITMAP_PNG_FROM_DATA(sound));
	script_img = images->Add(wxBITMAP_PNG_FROM_DATA(script));
	document_img = images->Add(wxBITMAP_PNG_FROM_DATA(document));
	query_img = images->Add(wxBITMAP_PNG_FROM_DATA(query));
	dataset_img = images->Add(wxBITMAP_PNG_FROM_DATA(dataset));
	tree->SetImageList(images);
	root = tree->AddRoot(_("Project"));
#if PHON_LINUX
	auto col = main_label->GetBackgroundColour();
#else
	auto col = wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK);
#endif
	this->SetBackgroundColour(col);
	main_label->SetBackgroundColour(col);
	tree->SetBackgroundColour(col);

	root = tree->GetRootItem();
	Populate();

	tree->Bind(wxEVT_TREE_BEGIN_DRAG, &ProjectManager::OnDragItem, this);
	tree->Bind(wxEVT_TREE_END_DRAG, &ProjectManager::OnDropItem, this);
	tree->Bind(wxEVT_KEY_DOWN, &ProjectManager::OnKeyDown, this);
	Bind(wxEVT_TREE_SEL_CHANGED, &ProjectManager::OnItemSelected, this);
	Bind(wxEVT_TREE_ITEM_ACTIVATED, &ProjectManager::OnItemDoubleClicked, this);
	Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &ProjectManager::OnRightClick, this);
	Bind(wxEVT_TREE_ITEM_MIDDLE_CLICK, &ProjectManager::OnMiddleClick, this);
	search_ctrl->Bind(wxEVT_TEXT, &ProjectManager::OnQuickSearch, this);
	menu_btn->Bind(wxEVT_LEFT_DOWN, &ProjectManager::OnProjectContextMenu, this);
#ifdef __WXMSW__
	Bind(wxEVT_TREE_ITEM_GETTOOLTIP, &ProjectManager::OnShowToolTip, this);
#endif

	SetScriptingFunctions();
}

void ProjectManager::Populate()
{
	auto p = Project::get();
	corpus_item = tree->AppendItem(root, _("Corpus"), corpus_img, corpus_img, new ItemData(p->corpus().get()));
	query_item = tree->AppendItem(root, _("Queries"), queries_img, queries_img, new ItemData(p->queries().get()));
	data_item = tree->AppendItem(root, _("Datasets"), datasets_img, datasets_img, new ItemData(p->data().get()));
	script_item = tree->AppendItem(root, _("Scripts"), scripts_img, scripts_img, new ItemData(p->scripts().get()));
	bookmark_item = tree->AppendItem(root, _("Bookmarks"), bookmarks_img, bookmarks_img, new ItemData(p->bookmarks().get()));
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
	main_label->SetToolTip(Project::get()->path());

	FillFolder(corpus_item, *project->corpus());
	FillFolder(data_item, *project->data());
	FillFolder(query_item, *project->queries());
	FillFolder(script_item, *project->scripts());
	FillFolder(bookmark_item, *project->bookmarks());

	tree->Expand(corpus_item);
	tree->Expand(query_item);
	tree->Expand(data_item);
	tree->Expand(script_item);
	tree->Expand(bookmark_item);
}

void ProjectManager::ClearProject()
{
	SetExpansionFlag(corpus_item);
	SetExpansionFlag(query_item);
	SetExpansionFlag(data_item);
	SetExpansionFlag(script_item);
	SetExpansionFlag(bookmark_item);

	tree->DeleteChildren(corpus_item);
	tree->DeleteChildren(query_item);
	tree->DeleteChildren(data_item);
	tree->DeleteChildren(script_item);
	tree->DeleteChildren(bookmark_item);
}

void ProjectManager::FillFolder(wxTreeItemId item, VFolder &folder)
{
	for (int i = 1; i <= folder.size(); i++)
	{
		wxTreeItemId child;
		auto &node = folder.get(i);

		// Dismiss files and folders that don't match the quick search string.
		if (!search_string.empty() && !node->quick_search(search_string)) {
			continue;
		}

		if (node->is_folder())
		{
			auto &subfolder = dynamic_cast<VFolder&>(*node);
			auto data = new ItemData(&subfolder);
			child = tree->AppendItem(item, node->label(), folder_img, folder_img, data);
			FillFolder(child, subfolder);
		}
		else if (node->is_bookmark())
		{
//			auto &bookmark = dynamic_cast<Bookmark&>(*node);
			auto data = new ItemData(node.get());
			child = tree->AppendItem(item, node->label(), bookmark_img, bookmark_img, data);
		}
		else
		{
			auto &vfile = dynamic_cast<VFile&>(*node);
			int img;

			if (vfile.is_annotation())
			{
				img = annot_img;
			}
			else if (vfile.is_sound())
			{
				img = sound_img;
			}
			else if (vfile.is_script())
			{
				img = script_img;
			}
			else if (vfile.is_dataset())
			{
				img = dataset_img;
			}
			else if (vfile.is_query())
			{
				img = query_img;
			}
			else
			{
				img = document_img;
			}
			auto data = new ItemData(node.get());
			auto label = node->label();
			if (node->modified()) label.append('*');
			child = tree->AppendItem(item, label, img, img, data);
		}
		tree->SetItemFont(child, mono_font);
	}

	if (folder.expanded()) {
		tree->Expand(item);
	}
}

void ProjectManager::OnItemSelected(wxTreeEvent &)
{
	wxArrayTreeItemIds items;
	tree->GetSelections(items);
	VFileList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(tree->GetItemData(item));
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
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(e.GetItem()));
	if (!data) return; // should never happen
	auto vnode = data->node;
	auto id = data->GetId();
	assert(vnode);

	if (vnode->is_folder())
	{
		if (tree->IsExpanded(id))
		{
			tree->Collapse(id);
		}
		else
		{
			tree->Expand(id);
		}
	}
	else
	{
		auto vf = downcast<VFile>(vnode->shared_from_this());

		if (vf->is_query())
		{
			edit_query(downcast<Query>(vf));
		}
		else
		{
			view_file(vf);
		}
	}
}

void ProjectManager::OnMiddleClick(wxTreeEvent &e)
{
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(e.GetItem()));
	if (!data) return; // should never happen
	auto vnode = data->node;
	auto id = data->GetId();
	assert(vnode);

	if (vnode->is_folder())
	{
		if (tree->IsExpanded(id))
		{
			CollapseNode(id);
		}
		else
		{
			ExpandNode(id);
		}
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
			tree->GetSelections(ids);
			auto tree_item = ids.front();

			auto expand_id = wxNewId();
			menu->Append(expand_id, _("Expand content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnExpandDirectory, this, expand_id);
			auto collapse_id = wxNewId();
			menu->Append(collapse_id, _("Collapse content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnCollapseDirectory, this, collapse_id);
			menu->AppendSeparator();

			if (folder->toplevel() == Project::get()->scripts().get())
			{
				auto script_id = wxNewId();
				menu->Append(script_id, _("New script"));
				menu->AppendSeparator();
				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) {
					wxArrayTreeItemIds sel;
					tree->GetSelections(sel);
					tree->Expand(sel.front());
					new_script(folder.get());
				}, script_id);
			}

			if (GetParentDirectory(tree_item) != bookmark_item)
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
				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) { RenameDirectory(folder); }, rename_id);

				menu->AppendSeparator();
				menu->Append(remove_id, _("Remove directory"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,folder](wxCommandEvent &) mutable { RemoveDirectory(folder); }, remove_id);
			}
		}
		else if (item->is_file())
		{
			auto file = downcast<VFile>(item);

			if (file->is_query())
			{
				auto query = downcast<Query>(file);
				auto edit_id = wxNewId();
				menu->Append(edit_id, _("Edit"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { edit_query(query); }, edit_id);

				auto dup_id = wxNewId();
				menu->Append(dup_id, _("Duplicate"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { DuplicateQuery(query); }, dup_id);

				auto rename_id = wxNewId();
				menu->Append(rename_id, _("Rename..."));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { RenameQuery(query); }, rename_id);
			}
			else
			{
				auto view_id = wxNewId();
				menu->Append(view_id, _("View file"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { view_file(file); }, view_id);
			}

			if (file->is_annotation())
			{
				auto annot = downcast<Annotation>(file);
				auto convert_id = wxNewId();
				if (annot->is_native())
				{
					menu->AppendSeparator();
					menu->Append(convert_id, _("Save as Praat TextGrid..."));
					Bind(wxEVT_COMMAND_MENU_SELECTED, [this,annot](wxCommandEvent &) { ConvertAnnotationToTextGrid(annot); }, convert_id);
				}
				else if (annot->is_textgrid())
				{
					auto praat_id = wxNewId();
					menu->Append(praat_id, _("Open annotation in Praat"));
					Bind(wxEVT_COMMAND_MENU_SELECTED, [this,annot](wxCommandEvent &) { OpenAnnotationInPraat(annot); }, praat_id);
					menu->AppendSeparator();

					menu->Append(convert_id, _("Save as Phonometrica annotation..."));
					Bind(wxEVT_COMMAND_MENU_SELECTED, [this,annot](wxCommandEvent &) { ConvertTextGridToAnnotation(annot); }, convert_id);
				}
			}
			else if (file->is_script())
			{
				auto script = downcast<Script>(file);
				auto run_id = wxNewId();
				menu->Append(run_id, _("Run"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { execute_script(script->path()); }, run_id);
				menu->AppendSeparator();
			}

			auto save_id = wxNewId();
			auto label = file->has_path() ? _("Save") : _("Save as...");
			menu->Append(save_id, label);
			menu->Enable(save_id, file->modified());
			Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { SaveFile(file); Project::updated(); }, save_id);
			menu->AppendSeparator();

			auto clip_id = wxNewId();
			menu->Append(clip_id, _("Copy path to clipboard"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,file](wxCommandEvent &) { CopyTextToClipboard(file->path()); }, clip_id);
			menu->AppendSeparator();

			auto remove_id = wxNewId();
			menu->Append(remove_id, _("Remove file from project"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,file](wxCommandEvent &) mutable { RemoveFile(file); }, remove_id);
		}
	}
	else if (items.size() == 2)
	{
		AutoAnnotation annot;
		AutoSound sound;

		if (items[1]->is_annotation() && items[2]->is_sound())
		{
			annot = downcast<Annotation>(items[1]);
			sound = downcast<Sound>(items[2]);
		}
		else if (items[1]->is_sound() && items[2]->is_annotation())
		{
			sound = downcast<Sound>(items[1]);
			annot = downcast<Annotation>(items[2]);
		}

		if (annot)
		{
			auto bind_id = wxNewId();
			menu->Append(bind_id, _("Bind annotation to sound file"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) {
				annot->set_sound(sound);
				Project::updated();
			}, bind_id);
			menu->AppendSeparator();
		}
	}

	if (items.size() > 1)
	{
		auto remove_id = wxNewId();
		menu->Append(remove_id, _("Remove files from project"));
		Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { RemoveItems(items); }, remove_id);
	}

	PopupMenu(menu);
}

VNodeList ProjectManager::GetSelectedItems() const
{
	wxArrayTreeItemIds items;
	tree->GetSelections(items);
	VNodeList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(tree->GetItemData(item));
		if (!data) {
			wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
			return files;
		}
		auto vnode = data->node;
		files.append(vnode->shared_from_this());
	}

	return files;
}

wxTreeItemId ProjectManager::GetSelectedId() const
{
	wxArrayTreeItemIds items;
	tree->GetSelections(items);
	if (items.IsEmpty()) {
		return wxTreeItemId();
	}

	return items.front();
}


std::shared_ptr<VFolder> ProjectManager::GetSelectedFolder() const
{
	wxArrayTreeItemIds ids;
	tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(id));
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
	tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(id));
	if (!data) {
		wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
		return nullptr;
	}
	auto vnode = data->node;

	return downcast<VFile>(vnode->shared_from_this());
}

void ProjectManager::RemoveDirectory(std::shared_ptr<VFolder> &folder)
{
	auto result = ask_question(_("Are you sure you want to remove this directory from the current project?\n"
	                             "(Its content won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	Project::get()->remove(folder);
	Project::updated();
}

void ProjectManager::RemoveFile(std::shared_ptr<VFile> &file)
{
	auto result = ask_question(_("Are you sure you want to remove this file from the current project?\n"
	                             "(It won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	Project::get()->remove(file);
	Project::updated();
}

void ProjectManager::RemoveFiles(VNodeList files)
{
	int result;

	if (files.empty())
	{
		return;
	}
	else if (files.size() == 1)
	{
		if (files.front()->is_folder())
		{
			result = ask_question(_("Are you sure you want to remove this folder from the current project?\n"
			                        "(It won't be deleted from your hard drive.)"), _("Confirm"));
		}
		else
		{
			result = ask_question(_("Are you sure you want to remove this file from the current project?\n"
			                        "(It won't be deleted from your hard drive.)"), _("Confirm"));
		}
	}
	else
	{
		result = ask_question(_("Are you sure you want to remove these files from the current project?\n"
						  "(They won't be deleted from your hard drive.)"), _("Confirm"));
	}

	if (result != wxYES) {
		return;
	}

	Project::get()->remove(files);
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
	main_label->SetLabel(label);
}

void ProjectManager::OnExpandDirectory(wxCommandEvent &e)
{
	wxArrayTreeItemIds ids;
	tree->GetSelections(ids);
	auto id = ids.front();
	ExpandNode(id);
}

void ProjectManager::OnCollapseDirectory(wxCommandEvent &)
{
	wxArrayTreeItemIds ids;
	tree->GetSelections(ids);
	auto id = ids.front();
	CollapseNode(id);
}

void ProjectManager::ExpandNode(wxTreeItemId node)
{
	tree->Expand(node);
	wxTreeItemIdValue cookie;
	wxTreeItemId child = tree->GetFirstChild(node, cookie);

	while (child.IsOk())
	{
		ExpandNode(child);
		child = tree->GetNextChild(node, cookie);
	}
}

void ProjectManager::CollapseNode(wxTreeItemId node)
{
	tree->Collapse(node);
	wxTreeItemIdValue cookie;
	wxTreeItemId child = tree->GetFirstChild(node, cookie);

	while (child.IsOk())
	{
		CollapseNode(child);
		child = tree->GetNextChild(node, cookie);
	}
}

void ProjectManager::SetExpansionFlag(wxTreeItemId node)
{
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(node));
	assert(data);
	auto vnode = data->node;
	assert(node);

	if (vnode->is_folder())
	{
		auto vfolder = dynamic_cast<VFolder*>(vnode);
		vfolder->set_expanded(tree->IsExpanded(node));
		wxTreeItemIdValue cookie;
		wxTreeItemId child = tree->GetFirstChild(node, cookie);

		while (child.IsOk())
		{
			SetExpansionFlag(child);
			child = tree->GetNextChild(node, cookie);
		}
	}
}

void ProjectManager::OnAddFilesToDirectory(wxCommandEvent &e)
{
	FileDialog dlg(this, _("Add files to directory..."), "", "Phonometrica files (*.*)|*.*",
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
	tree->Expand(GetSelectedId());
	CheckProjectImport();

	// Files are added silently, so we need to explicitly modify the project
	project->modify();
	Project::updated();
}

wxTreeItemId ProjectManager::GetParentDirectory(wxTreeItemId item) const
{
	while (true)
	{
		auto tmp = tree->GetItemParent(item);
		if (tmp == root) {
			return item;
		}
		item = tmp;
	}
}

void ProjectManager::OnDragItem(wxTreeEvent &e)
{
	dragged_files = GetSelectedItems();
	auto project = Project::get();

	for (auto &item : dragged_files)
	{
		if (item->is_folder() && project->is_root(dynamic_cast<VFolder*>(item.get())))
		{
			dragged_files.clear();
			return;
		}
	}

	e.Allow();
}

void ProjectManager::OnDropItem(wxTreeEvent &e)
{
	auto dest_item = e.GetItem();
	if (!dest_item.IsOk()) {
		return;
	}

	auto dest_data = dynamic_cast<ItemData*>(tree->GetItemData(dest_item));
	assert(dest_data);
	auto dest_node = dest_data->node;

	// Ensure that the dragged items and the drop target have the same root
	// and that the drop target is not lower than the dragged items in the tree
	auto toplevel = dest_node->toplevel();

	for (auto &file : dragged_files)
	{
		if (file->toplevel() != toplevel || file->contains(dest_node))
		{
			dragged_files.clear();
			return;
		}
	}

	// If the target is a folder, append at the end
	if (dest_node->is_folder())
	{
		auto folder = dynamic_cast<VFolder*>(dest_node);
		tree->Expand(dest_item);

		for (auto &file : dragged_files) {
			file->move_to(folder, -1);
		}
	}
	else
	{
		auto parent_item = tree->GetItemParent(dest_item);
		int i = 1;
		wxTreeItemIdValue cookie;
		wxTreeItemId child = tree->GetFirstChild(parent_item, cookie);

		while (child.IsOk())
		{
			if (child == dest_item)
			{
				auto parent_data = dynamic_cast<ItemData*>(tree->GetItemData(parent_item));
				auto folder = dynamic_cast<VFolder*>(parent_data->node);

				for (auto &file : dragged_files) {
					file->move_to(folder, i+1);
				}
				break;
			}
			child = tree->GetNextChild(parent_item, cookie);
			i++;
		}
	}

	e.Allow();
	dragged_files.clear();
	Project::updated();
}

void ProjectManager::CheckProjectImport()
{
	if (Project::get()->import_flag())
	{
		wxMessageBox(_("Some files were ignored:\n their type was inadequate or they were already present in the corpus."),
			   _("Files ignored"), wxICON_INFORMATION);
	}
	Project::get()->clear_import_flag();
}

void ProjectManager::SetScriptingFunctions()
{

}

void ProjectManager::ConvertAnnotationToTextGrid(const AutoAnnotation &annot)
{
	String name = filesystem::base_name(annot->path());
	name.replace_last(PHON_EXT_ANNOTATION, ".TextGrid");
	FileDialog dlg(this, _("Save as TextGrid..."), name, "TextGrid (*.TextGrid)|*.TextGrid",
	                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	auto path = dlg.GetPath();
	annot->write_as_textgrid(path);
	AskImportFile(path);
}

void ProjectManager::ConvertTextGridToAnnotation(const AutoAnnotation &annot)
{
	String name = filesystem::base_name(annot->path());
	name.replace_last(".TextGrid", PHON_EXT_ANNOTATION);
	FileDialog dlg(this, _("Save as annotation..."), name, "Annotation (*.phon-annot)|*.phon-annot",
	                 wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	auto path = dlg.GetPath();
	if (!path.ends_with(PHON_EXT_ANNOTATION)) {
		path.append(PHON_EXT_ANNOTATION);
	}
	annot->write_as_native(path);
	AskImportFile(path);
}

void ProjectManager::OpenAnnotationInPraat(const AutoAnnotation &annot)
{
	try
	{
		if (annot->has_sound())
		{
			praat::open_textgrid(annot->path(), annot->sound()->path());
		}
		else
		{
			praat::open_textgrid(annot->path());
		}
	}
	catch (std::exception &e)
	{
		wxString msg = _("Cannot open TextGrid in Praat: ");
		msg.Append(wxString::FromUTF8(e.what()));
		wxMessageBox(msg, _("Praat error"), wxICON_ERROR);
	}
}

void ProjectManager::AskImportFile(const String &path)
{
	auto reply = ask_question(_("Would you like to import this annotation into the current project?"), _("Import file?"));

	if (reply == wxYES)
	{
		auto project = Project::get();
		project->import_file(path);
		project->notify_update();
	}
}

void ProjectManager::CopyTextToClipboard(const wxString &text)
{
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(text));
		wxTheClipboard->Close();
	}
}

void ProjectManager::RemoveItems(const VNodeList &items)
{
	auto result = ask_question(_("Are you sure you want to remove these files from the current project?\n"
							  "(They won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	auto project = Project::get();

	for (auto &item : items)
	{
		if (item->is_file())
		{
			auto file = downcast<VFile>(item->shared_from_this());
			project->remove(file);
		}
		else
		{
			auto folder = downcast<VFolder>(item->shared_from_this());
			project->remove(folder);
		}
	}
	Project::updated();
}

void ProjectManager::RenameQuery(const AutoQuery &query)
{
	String name = wxGetTextFromUser(_("New query name:"), _("Rename query..."));

	if (!name.empty())
	{
		query->set_label(name, true);
		Project::updated();
	}
}

void ProjectManager::DuplicateQuery(const AutoQuery &query)
{
	auto label = query->label();
	label.append(" (copy)");
	String name = wxGetTextFromUser(_("New query name:"), _("Duplicate query..."), label);

	if (!name.empty())
	{
		auto parent = query->parent();
		if (!parent) parent = Project::get()->queries().get();
		auto copy = query->clone();
		copy->set_label(name, true);
		parent->append(std::move(copy));
		Project::updated();
	}
}

void ProjectManager::OnQuickSearch(wxCommandEvent &e)
{
	auto project = Project::get();
	if (project->empty()) {
		return;
	}

	String text = search_ctrl->GetValue();
	search_string = text.to_lower();
	ClearProject();

	FillFolder(corpus_item, *project->corpus());
	FillFolder(data_item, *project->data());
	FillFolder(query_item, *project->queries());
	FillFolder(script_item, *project->scripts());
	FillFolder(bookmark_item, *project->bookmarks());
	tree->ExpandAll();
}

void ProjectManager::SaveFile(const std::shared_ptr<VFile> &file)
{
	if (!file->has_path())
	{
		FileDialog dlg(this, _("Save file..."), "", "Phonometrica files (*.*)|*.*",
		               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

		if (dlg.ShowModal() == wxID_CANCEL) {
			return;
		}
		file->set_path(dlg.GetPath(), false);
	}
	file->save();
	Project::updated();
}

void ProjectManager::OnProjectContextMenu(wxMouseEvent &e)
{
	auto menu = new wxMenu;
	auto expand_entry = menu->Append(wxNewId(), _("Expand project"));
	auto collapse_entry = menu->Append(wxNewId(), _("Collapse project"));
	menu->AppendSeparator();
	auto rename_entry = menu->Append(wxNewId(), _("Rename project..."));
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { tree->ExpandAll(); }, expand_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { tree->CollapseAll(); }, collapse_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnRenameProject, this, rename_entry->GetId());

	auto pos =  menu_btn->GetPosition();
	pos.y +=  menu_btn->GetSize().GetHeight();
	PopupMenu(menu, pos);
}

void ProjectManager::OnRenameProject(wxCommandEvent &)
{
	String name = wxGetTextFromUser(_("New project name:"), _("Rename project..."));

	if (!name.empty())
	{
		Project::get()->set_label(name);
		UpdateLabel();
	}
}

void ProjectManager::OnKeyDown(wxKeyEvent &e)
{
	if (e.GetKeyCode() == WXK_DELETE)
	{
		RemoveFiles(GetSelectedItems());
	}
	else
	{
		e.Skip();
	}
}

#ifdef __WXMSW__
void ProjectManager::OnShowToolTip(wxTreeEvent &e)
{
	auto items = GetSelectedItems();
	if (items.size() == 1 && items.front()->is_file())
	{
		e.SetToolTip(downcast<VFile>(items.front())->path());
	}
}
#endif
} // namespace phonometrica