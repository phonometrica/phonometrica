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
#include <wx/artprov.h>
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

	m_label->SetLabel(project->label());

	FillFolder(m_corpus_item, *project->corpus());
	FillFolder(m_data_item, *project->data());
	FillFolder(m_query_item, *project->queries());
	FillFolder(m_script_item, *project->scripts());
	FillFolder(m_bookmark_item, *project->bookmarks());
//	m_tree->Expand(m_corpus_item);
	m_tree->ExpandAll();
	m_tree->Refresh();
}

void ProjectManager::ClearProject()
{
	m_tree->DeleteChildren(m_corpus_item);
	m_tree->DeleteChildren(m_query_item);
	m_tree->DeleteChildren(m_data_item);
	m_tree->DeleteChildren(m_script_item);
	m_tree->DeleteChildren(m_bookmark_item);
}

void ProjectManager::FillFolder(wxTreeItemId &item, VFolder &folder)
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
	auto files = GetSelectedItems();

	if (files.size() == 1)
	{

	}
	// TODO....
}

VNodeList ProjectManager::GetSelectedItems() const
{
	wxArrayTreeItemIds items;
	m_tree->GetSelections(items);
	VNodeList files;

	for (auto &item : items)
	{
		auto data = dynamic_cast<ItemData*>(m_tree->GetItemData(item));
		if (!data) return files; // should never happen
		auto vnode = data->node;
		files.append(vnode->shared_from_this());
	}

	return files;
}
} // namespace phonometrica