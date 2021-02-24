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
#include <phon/gui/text_viewer.hpp>
#include <phon/include/icons.hpp>
#include <phon/application/macros.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/praat.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

struct ItemData final : public wxTreeItemData
{
	ItemData(Element *n) :
			wxTreeItemData(), node(n) { }

	~ItemData() override = default;

	Element *node = nullptr;
};

ProjectManager::ProjectManager(Runtime &rt, wxWindow *parent) :
	wxPanel(parent), runtime(rt)
{
	tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT | wxTR_MULTIPLE | wxTR_NO_LINES | wxTR_DEFAULT_STYLE);
	timer.SetOwner(tree);
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

	activity_indicator = new wxActivityIndicator(this, wxID_ANY, wxDefaultPosition, wxSize(30, 30));
    activity_indicator->Hide();
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->Add(main_label, 0, wxALIGN_CENTER);
	hsizer->AddStretchSpacer();
	hsizer->Add(activity_indicator);
	menu_btn = new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
	menu_btn->SetBitmap(wxBITMAP_PNG_FROM_DATA(menu));
	menu_btn->SetMaxSize(wxSize(30, 30));
	menu_btn->SetToolTip(_("Show project menu"));
	hsizer->Add(menu_btn, 0, wxRIGHT, 10);


	auto sizer = new wxBoxSizer(wxVERTICAL);
#ifdef __WXGTK__
	sizer->AddSpacer(2);
#elif defined (__WXMAC__)
	sizer->AddSpacer(5);
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
	textgrid_img = images->Add(wxBITMAP_PNG_FROM_DATA(textgrid));
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
	conc_img = images->Add(wxBITMAP_PNG_FROM_DATA(dataset));
	csv_img = images->Add(wxBITMAP_PNG_FROM_DATA(csv));
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
	// FIXME: the middle button event doesn't seem to be handled on Windows, so we simulate it.
#ifdef __WXMSW__
	tree->Bind(wxEVT_MIDDLE_UP, &ProjectManager::OnMouseMiddleClick, this);
#else
	Bind(wxEVT_TREE_ITEM_MIDDLE_CLICK, &ProjectManager::OnMiddleClick, this);
#endif
	search_ctrl->Bind(wxEVT_TEXT, &ProjectManager::OnQuickSearch, this);
	menu_btn->Bind(wxEVT_LEFT_DOWN, &ProjectManager::OnProjectContextMenu, this);
	// Disable tooltips for now.
//	// Tooltips are only supported on Windows, so we roll our own cross-platform tooltips.
//#ifdef __WXMSW__
//	Bind(wxEVT_TREE_ITEM_GETTOOLTIP, &ProjectManager::OnShowToolTip, this);
//#else
//	tree->Bind(wxEVT_MOTION, &ProjectManager::OnMouseMove, this);
//	tree->Bind(wxEVT_TIMER, &ProjectManager::OnTimerDone, this);
//#endif

	SetScriptingFunctions();
}

void ProjectManager::Populate()
{
	auto p = Project::get();
	corpus_item = tree->AppendItem(root, _("Corpus"), corpus_img, corpus_img, new ItemData(p->corpus().get()));
	query_item = tree->AppendItem(root, _("Queries"), queries_img, queries_img, new ItemData(p->queries().get()));
	data_item = tree->AppendItem(root, _("Data tables"), datasets_img, datasets_img, new ItemData(p->data().get()));
	script_item = tree->AppendItem(root, _("Scripts"), scripts_img, scripts_img, new ItemData(p->scripts().get()));
	bookmark_item = tree->AppendItem(root, _("Bookmarks"), bookmarks_img, bookmarks_img, new ItemData(p->bookmarks().get()));
}

void ProjectManager::OnProjectUpdated()
{
	ClearProject(true);
	UpdateProject();
}

void ProjectManager::OnProjectClosed()
{
	ClearProject(false);
	UpdateLabel();
	main_label->SetToolTip(wxString());
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
}

void ProjectManager::ClearProject(bool set_flag)
{
	if (set_flag)
	{
		SetExpansionFlag(corpus_item);
		SetExpansionFlag(query_item);
		SetExpansionFlag(data_item);
		SetExpansionFlag(script_item);
		SetExpansionFlag(bookmark_item);
	}

	tree->DeleteChildren(corpus_item);
	tree->DeleteChildren(query_item);
	tree->DeleteChildren(data_item);
	tree->DeleteChildren(script_item);
	tree->DeleteChildren(bookmark_item);
}

void ProjectManager::FillFolder(wxTreeItemId item, Directory &folder)
{
	for (int i = 1; i <= folder.size(); i++)
	{
		wxTreeItemId child;
		auto &node = folder.get(i);

		// Dismiss files and folders that don't match the quick search string.
		if (!search_string.empty() && !node->quick_search(search_string)) {
			continue;
		}

		if (node->is<Directory>())
		{
			auto &subfolder = dynamic_cast<Directory&>(*node);
			auto data = new ItemData(&subfolder);
			child = tree->AppendItem(item, node->label(), folder_img, folder_img, data);
			FillFolder(child, subfolder);
		}
		else if (node->is<Bookmark>())
		{
			auto data = new ItemData(node.get());
			child = tree->AppendItem(item, node->label(), bookmark_img, bookmark_img, data);
		}
		else
		{
			auto &vfile = dynamic_cast<Document&>(*node);
			int img;

			if (vfile.is<Annotation>())
			{
				if (dynamic_cast<Annotation&>(vfile).is_textgrid()) {
					img = textgrid_img;
				}
				else {
					img = annot_img;
				}
			}
			else if (vfile.is<Sound>())
			{
				img = sound_img;
			}
			else if (vfile.is<Script>())
			{
				img = script_img;
			}
			else if (vfile.is<Concordance>())
			{
				img = conc_img;
			}
			else if (vfile.is<Dataset>())
			{
				img = csv_img;
			}
			else if (vfile.is<Query>())
			{
				img = query_img;
			}
			else if (vfile.is<Bookmark>())
			{
				img = bookmark_img;
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
	DocList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(tree->GetItemData(item));
		if (!data) return; // should never happen
		auto elem = data->node;
		assert(elem);

		if (elem->is<Document>())
		{
			auto doc = static_cast<Document*>(elem);
			files.append(Handle<Document>(doc));
		}
		else if (elem->is<Bookmark>() && items.size() == 1)
		{
			auto bookmark = static_cast<Bookmark*>(elem);
			bookmark_selected(Handle<Bookmark>(bookmark));
			return;
		}
	}
	files_selected(std::move(files));
}

void ProjectManager::OnItemDoubleClicked(wxTreeEvent &e)
{
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(e.GetItem()));
	if (!data) return; // should never happen
	auto elem = data->node;
	auto id = data->GetId();
	assert(elem);

	if (elem->is<Directory>())
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
	else if (elem->is<Bookmark>())
	{
		wxCommandEvent dummy;
		OnViewBookmark(dummy);
	}
	else
	{
		auto doc = static_cast<Document*>(elem);

		if (doc->is<Query>())
		{
			edit_query(Handle<Query>(static_cast<Query*>(doc)));
		}
		else
		{
			view_file(Handle<Document>(doc));
		}
	}
}

void ProjectManager::OnMouseMiddleClick(wxMouseEvent &e)
{
    auto item = FindItem(e.GetPosition());
    if (item.IsOk())
    {
	    wxTreeEvent evt(0, tree, item);
	    OnMiddleClick(evt);
    }
}

void ProjectManager::OnMiddleClick(wxTreeEvent &e)
{
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(e.GetItem()));
	if (!data) return; // should never happen
	auto vnode = data->node;
	auto id = data->GetId();
	assert(vnode);

	if (vnode->is<Directory>())
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

		if (item->is<Directory>())
		{
			auto folder = recast<Directory>(item);
			wxArrayTreeItemIds ids;
			tree->GetSelections(ids);
			auto tree_item = ids.front();

			auto expand_id = wxNewId();
			menu->Append(expand_id, _("Expand content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnExpandDirectory, this, expand_id);
			auto collapse_id = wxNewId();
			menu->Append(collapse_id, _("Collapse content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnCollapseDirectory, this, collapse_id);
			auto sort_id = wxNewId();
			menu->Append(sort_id, _("Sort content"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [folder](wxCommandEvent &) { folder->sort(); Project::updated(); }, sort_id);
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
		else if (item->is<Document>())
		{
			auto file = recast<Document>(item);

			if (file->is<Query>())
			{
				auto query = recast<Query>(file);
				auto edit_id = wxNewId();
				menu->Append(edit_id, _("Edit"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { edit_query(query); }, edit_id);

				auto dup_id = wxNewId();
				menu->Append(dup_id, _("Duplicate..."));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { DuplicateQuery(query); }, dup_id);

				auto rename_id = wxNewId();
				menu->Append(rename_id, _("Rename..."));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { RenameQuery(query); }, rename_id);
				menu->AppendSeparator();

				auto exe_id = wxNewId();
				menu->Append(exe_id, _("Execute"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [this,query](wxCommandEvent &) { view_file(query->execute()); Project::updated(); }, exe_id);
				menu->AppendSeparator();
			}
			else
			{
				auto view_id = wxNewId();
				menu->Append(view_id, _("View file"));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { view_file(file); }, view_id);
			}

			if (file->is<Annotation>())
			{
				auto annot = recast<Annotation>(file);
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
			else if (file->is<Concordance>())
			{
				auto conc = recast<Concordance>(file);
				auto rename_id = wxNewId();
				menu->Append(rename_id, _("Rename..."));
				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { RenameConcordance(conc); }, rename_id);
				menu->AppendSeparator();
			}
			else if (file->is<Script>())
			{
				auto script = recast<Script>(file);
				auto run_id = wxNewId();
				menu->Append(run_id, _("Execute"));
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
		else if (item->is<Bookmark>())
		{
			auto view_id = wxNewId();
			menu->Append(view_id, _("View bookmark"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnViewBookmark, this, view_id);

			if (item->is<TimeStamp>())
			{
				auto stamp = recast<TimeStamp>(item);
				auto annot = stamp->annotation();

				if (annot->is_textgrid())
				{
					auto praat_id = wxNewId();
					menu->Append(praat_id, _("Open in Praat"));
					Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { OpenBookmarkInPraat(stamp); }, praat_id);
				}
			}
			auto bookmark = recast<Bookmark>(item);
			menu->AppendSeparator();
			auto remove_id = wxNewId();
			menu->Append(remove_id, _("Remove"));
			Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) mutable { RemoveBookmark(bookmark); }, remove_id);
		}
	}
	else if (items.size() == 2)
	{
		Handle<Annotation> annot;
		Handle<Sound> sound;

		if (items[1]->is<Annotation>() && items[2]->is<Sound>())
		{
			annot = recast<Annotation>(items[1]);
			sound = recast<Sound>(items[2]);
		}
		else if (items[1]->is<Sound>() && items[2]->is<Annotation>())
		{
			sound = recast<Sound>(items[1]);
			annot = recast<Annotation>(items[2]);
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

	timer.Stop();
	PopupMenu(menu);
}

ElementList ProjectManager::GetSelectedItems() const
{
	wxArrayTreeItemIds items;
	tree->GetSelections(items);
	ElementList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(tree->GetItemData(item));
		if (!data) {
			wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
			return files;
		}
		auto elem = data->node;
		files.append(Handle<Element>(elem));
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


Handle<Directory> ProjectManager::GetSelectedFolder() const
{
	wxArrayTreeItemIds ids;
	tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(id));
	if (!data) {
		wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
		return nullptr;
	}
	auto elem = data->node;

	return Handle<Directory>(static_cast<Directory*>(elem));
}

Handle<Document> ProjectManager::GetSelectedFile() const
{
	wxArrayTreeItemIds ids;
	tree->GetSelections(ids);
	auto id = ids.front();
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(id));
	if (!data) {
		wxMessageBox(_("Null tree item data"), _("Internal error"), wxICON_ERROR);
		return nullptr;
	}
	auto elem = data->node;

	return Handle<Document>(static_cast<Document*>(elem));
}

void ProjectManager::RemoveDirectory(Handle<Directory> &folder)
{
	auto result = ask_question(_("Are you sure you want to remove this directory from the current project?\n"
	                             "(Its content won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	Project::get()->remove(folder);
	Project::updated();
}

void ProjectManager::RemoveBookmark(Handle<Bookmark> &bookmark)
{
	Project::get()->remove(bookmark);
	Project::updated();
}

void ProjectManager::RemoveFile(Handle<Document> &file)
{
	auto result = ask_question(_("Are you sure you want to remove this file from the current project?\n"
	                             "(It won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	Project::get()->remove(file);
	Project::updated();
}

void ProjectManager::RemoveFiles(ElementList files)
{
	int result;

	if (files.empty())
	{
		return;
	}
	else if (files.size() == 1)
	{
		if (files.front()->is<Directory>())
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

void ProjectManager::RenameDirectory(const Handle<Directory> &folder)
{
	String name = wxGetTextFromUser(_("New directory name:"), _("Rename directory..."));

	if (!name.empty())
	{
		folder->set_label(name);
		Project::updated();
	}
}

void ProjectManager::CreateSubdirectory(const Handle<Directory> &folder)
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

	if (vnode->is<Directory>())
	{
		auto vfolder = dynamic_cast<Directory*>(vnode);
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
		project->add_file(path, folder, type, false);
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
		if (item->is<Directory>() && project->is_root(dynamic_cast<Directory*>(item.get())))
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
	if (dest_node->is<Directory>())
	{
		auto folder = dynamic_cast<Directory*>(dest_node);
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
				auto folder = dynamic_cast<Directory*>(parent_data->node);

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

void ProjectManager::ConvertAnnotationToTextGrid(const Handle<Annotation> &annot)
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

void ProjectManager::ConvertTextGridToAnnotation(const Handle<Annotation> &annot)
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

void ProjectManager::OpenAnnotationInPraat(const Handle<Annotation> &annot)
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

void ProjectManager::RemoveItems(const ElementList &items)
{
	auto result = ask_question(_("Are you sure you want to remove these files from the current project?\n"
							  "(They won't be deleted from your hard drive.)"), _("Confirm"));

	if (result != wxYES) {
		return;
	}

	auto project = Project::get();

	for (auto &item : items)
	{
		if (item->is<Document>())
		{
			auto file = recast<Document>(item);
			project->remove(file);
		}
		else
		{
			auto folder = recast<Directory>(item);
			project->remove(folder);
		}
	}
	Project::updated();
}

void ProjectManager::RenameConcordance(const Handle<Concordance> &conc)
{
	String name = wxGetTextFromUser(_("New concordance name:"), _("Rename concordance..."));

	if (!name.empty())
	{
		conc->set_label(name, true);
		Project::updated();
	}
}

void ProjectManager::RenameQuery(const Handle<Query> &query)
{
	String name = wxGetTextFromUser(_("New query name:"), _("Rename query..."));

	if (!name.empty())
	{
		query->set_label(name, true);
		Project::updated();
	}
}

void ProjectManager::DuplicateQuery(const Handle<Query> &query)
{
	auto label = query->label();
	label.append(" (copy)");
	String name = wxGetTextFromUser(_("New query name:"), _("Duplicate query..."), label);

	if (!name.empty())
	{
		auto parent = query->parent();
		if (!parent) parent = Project::get()->queries().get();
		auto copy = query->copy();
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
	ClearProject(true);

	FillFolder(corpus_item, *project->corpus());
	FillFolder(data_item, *project->data());
	FillFolder(query_item, *project->queries());
	FillFolder(script_item, *project->scripts());
	FillFolder(bookmark_item, *project->bookmarks());
	tree->ExpandAll();
}

void ProjectManager::SaveFile(const Handle<Document> &file)
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
	auto save_entry = menu->Append(wxNewId(), _("Save project"));
	auto rename_entry = menu->Append(wxNewId(), _("Rename project..."));
	menu->AppendSeparator();
	auto stat_entry = menu->Append(wxNewId(), _("Summary statistics"));
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { tree->ExpandAll(); }, expand_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { tree->CollapseAll(); }, collapse_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) { request_save(); }, save_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnRenameProject, this, rename_entry->GetId());
	Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectManager::OnSummaryStatistics, this, stat_entry->GetId());

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

void ProjectManager::StartActivity()
{
	activity_indicator->Start();
}

void ProjectManager::StopActivity()
{
	activity_indicator->Stop();
}

void ProjectManager::Expand()
{
	tree->Expand(corpus_item);
	tree->Expand(query_item);
	tree->Expand(data_item);
	tree->Expand(script_item);
	tree->Expand(bookmark_item);
}

void ProjectManager::OnShowToolTip(wxTreeEvent &e)
{
#ifdef __WXMSW__
	auto items = GetSelectedItems();
	if (items.size() != 1) { return; }
	auto &item = items.front();
	if (item->is<Document>())
	{
		e.SetToolTip(recast<Document>(item)->path());
	}
	else if (item->is<Bookmark>())
	{
		e.SetToolTip(recast<Bookmark>(item)->tooltip());
	}
#else
	auto data = dynamic_cast<ItemData*>(tree->GetItemData(e.GetItem()));
	auto elem = data->node;
	Document *doc;
	Bookmark *bookmark;
	wxString title, content;

	if ((doc = dynamic_cast<Document*>(elem)))
	{
		title = doc->class_name();
		content = doc->path();
	}
	else if ((bookmark = dynamic_cast<Bookmark*>(elem)))
	{
		title = _("Bookmark");
		content = bookmark->tooltip();
	}

	if (!content.empty())
	{
		// Note: the title is truncated on Linux.
		wxRichToolTip tip(title, content);
		auto pos = tree->ScreenToClient(wxGetMousePosition()) - wxPoint(40, 30);
		wxRect rect(pos.x, pos.y, 80, 50);
		tip.ShowFor(tree, &rect);
	}
#endif
}

wxTreeItemId ProjectManager::FindItem(wxPoint pos)
{
	return FindItem(pos, root);
}

wxTreeItemId ProjectManager::FindItem(wxPoint pos, wxTreeItemId node)
{
	wxTreeItemIdValue cookie;
	wxTreeItemId child = tree->GetFirstChild(node, cookie);

	while (child.IsOk())
	{
		if (tree->IsVisible(child))
		{
			wxRect rect;
			tree->GetBoundingRect(child, rect);

			if (rect.Contains(pos)) {
				return child;
			}

			auto item = FindItem(pos, child);
			if (item.IsOk()) {
				return item;
			}
		}
		child = tree->GetNextChild(node, cookie);
	}

	return wxTreeItemId();
}

void ProjectManager::OnMouseMove(wxMouseEvent &)
{
    timer.Stop();
	timer.Start(1000, true);
}

void ProjectManager::OnTimerDone(wxTimerEvent &)
{
	auto pos = tree->ScreenToClient(wxGetMousePosition());
	auto item = FindItem(pos);
	if (item.IsOk())
	{
		wxTreeEvent evt(0, tree, item);
		OnShowToolTip(evt);
	}
}

void ProjectManager::OnSummaryStatistics(wxCommandEvent &)
{
	auto stat = Project::get()->get_statistics();
	auto keys = stat.keys();
	std::sort(keys.begin(), keys.end());
	intptr_t total = 0;
	String result;

	for (auto &key : keys)
	{
		result.append(key);
		result.append(": ");
		intptr_t value = stat[key];
		total += value;
		result.append(String::convert(value));
		result.append(" files\n");
	}
	result.append("------------------------\nTotal: ");
	result.append(String::convert(total));
	result.append(" files\n");

	TextViewer viewer(this, _("Summary statistics"), result);
	viewer.ShowModal();
}

void ProjectManager::OnViewBookmark(wxCommandEvent &)
{
	wxMessageBox(_("Not implemented yet!"), _(""), wxICON_INFORMATION);
}

void ProjectManager::OpenBookmarkInPraat(const Handle<TimeStamp> &stamp)
{
	auto annot = stamp->annotation();
	annot->open();
	String sound_path;

	if (annot->has_sound()) {
		sound_path = annot->sound()->path();
	}
	try
	{
		// We can't reliably get the interval because Phonometrica's precision and Praat's differ.
		auto &events = annot->get_layer_events(stamp->layer());
		double t = stamp->start();
		const double threshold = 0.0000000001;

		for (intptr_t i = 1; i <= events.size(); i++)
		{
			auto &e = events[i];

			if (std::abs(e->start_time() - t) < threshold)
			{
				praat::open_interval(stamp->layer(), i, annot->path(), sound_path);
				return;
			}
		}

		wxMessageBox(_("Event not found!"), _("Praat error"), wxICON_ERROR);
	}
	catch (std::exception &e)
	{
		wxString msg = _("Cannot open event in Praat");
		msg.Append(wxString::FromUTF8(e.what()));
		wxMessageBox(msg, _("System error"), wxICON_ERROR);
	}
}

} // namespace phonometrica