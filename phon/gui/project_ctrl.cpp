/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QDropEvent>
#include <QQueue>
#include <QInputDialog>
#include <QMenu>
#include <QFileIconProvider>
#include <phon/gui/project_ctrl.hpp>
#include <phon/gui/metadata_editor.hpp>
#include <phon/application/praat.hpp>
#include "project_ctrl.hpp"


namespace phonometrica {

struct TreeItem final : public QTreeWidgetItem
{
    TreeItem() = default;

    TreeItem(QTreeWidget *parent, VNode *n) :
        QTreeWidgetItem(parent), node(n) { }

    TreeItem(QTreeWidget *parent, QTreeWidgetItem *previous, VNode *n) :
        QTreeWidgetItem(parent, previous), node(n) { }

    TreeItem(VNode *n) :
        QTreeWidgetItem(), node(n) { }

    ~TreeItem() override = default;

    VNode *node = nullptr;
};


class ContextMenu final : public QMenu
{
public:

    ContextMenu(QWidget *parent) : QMenu(parent)
    {

    }

    void display(const QPoint &pos)
    {
        exec(this->parentWidget()->mapToGlobal(pos));
    }

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ProjectCtrl::ProjectCtrl(Runtime &rt, QWidget *parent) :
    QTreeWidget(parent), rt(rt)
{
#ifdef Q_OS_MAC // color the sidebar (as in Mail or Finder)
    QPalette p(this->palette());
    p.setColor(QPalette::Base, PHON_MAC_SIDEBAR_COLOR);
    setPalette(p);
    setStyleSheet("QTreeWidget { border: 0px; }"
                  "QHeaderView::section {"
                  "background-color: " PHON_MAC_SIDEBAR_COLOR ";"
                  "}");

    headerItem()->setHidden(true);
#endif

    setFocusPolicy(Qt::NoFocus);
    setDragDropMode(QAbstractItemView::InternalMove);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::SelectedClicked);
    connect(this, &ProjectCtrl::itemExpanded, this, &ProjectCtrl::changeExpanded);
    setFrameShadow(QFrame::Plain);
    refresh();

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemDoubleClicked(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onItemSelected(QTreeWidgetItem*)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(onRightClick(const QPoint &)));

}

void ProjectCtrl::setProjectLabel(const QString &name)
{
    setHeaderLabel(name);
}

void ProjectCtrl::refresh()
{
    auto project = Project::instance();
    auto corpus_folder = project ? project->corpus().get() : nullptr;
    auto data_folder = project ? project->data().get() : nullptr;
    auto query_folder = project ? project->queries().get() : nullptr;
    auto scripts_folder = project ? project->scripts().get() : nullptr;
    auto bookmarks_folder = project ? project->bookmarks().get() : nullptr;

    this->clear();
    corpus = new TreeItem(this, corpus_folder);
    corpus->setText(0, tr("Corpus"));
    corpus->setIcon(0, QIcon(":/icons/corpus.png"));
	queries = new TreeItem(this, corpus, query_folder);
	queries->setText(0, tr("Queries"));
	queries->setIcon(0, QIcon(":/icons/search.png"));
    data = new TreeItem(this, queries, data_folder);
    data->setText(0, tr("Datasets"));
    data->setIcon(0, QIcon(":/icons/data.png"));
    scripts = new TreeItem(this, data, scripts_folder);
    scripts->setText(0, tr("Scripts"));
    scripts->setIcon(0, QIcon(":/icons/source_code.png"));
    bookmarks = new TreeItem(this, scripts, bookmarks_folder);
    bookmarks->setText(0, tr("Bookmarks"));
    bookmarks->setIcon(0, QIcon(":/icons/favorite.png"));

    resetLabel();
    fillFolder(corpus, *project->corpus());
    fillFolder(data, *project->data());
    fillFolder(queries, *project->queries());
    fillFolder(scripts, *project->scripts());
    fillFolder(bookmarks, *project->bookmarks());
    corpus->setExpanded(true);
    data->setExpanded(true);
    scripts->setExpanded(true);
    bookmarks->setExpanded(true);

}

void ProjectCtrl::fillFolder(QTreeWidgetItem *item, VFolder &folder)
{
    QFileIconProvider provider;
    QIcon file_icon = provider.icon(QFileIconProvider::File);
	QIcon dir_icon = provider.icon(QFileIconProvider::Folder);
    QIcon annot_icon(":/icons/annotation.png");
    QIcon sound_icon(":/icons/sound.png");
    QIcon script_icon(":/icons/script.png");
    QIcon bookmark_icon(":/icons/bookmark.png");
    QIcon data_icon(":/icons/dataset.png");
    QIcon doc_icon(":/icons/document.png");

    for (int i = 1; i <= folder.size(); i++)
    {
        auto &node = folder.get(i);
        auto child = new TreeItem(node.get());
        child->setText(0, node->label());

        if (node->is_folder())
        {
            auto &subfolder = dynamic_cast<VFolder&>(*node);
            child->setIcon(0, dir_icon);
            item->insertChild(i-1, child);
            fillFolder(child, subfolder);
        }
        else if (node->is_bookmark())
        {
            auto &bookmark = dynamic_cast<Bookmark&>(*node);
            child->setIcon(0, bookmark_icon);
            item->insertChild(i-1, child);
        }
        else
        {
            auto &vfile = dynamic_cast<VFile&>(*node);
            child->setToolTip(0, vfile.path());
            child->setFlags(child->flags() ^ Qt::ItemIsDropEnabled);

            if (vfile.is_annotation())
            {
                child->setIcon(0, annot_icon);
            }
            else if (vfile.is_sound())
            {
                child->setIcon(0, sound_icon);
            }
            else if (vfile.is_script())
            {
                child->setIcon(0, script_icon);
            }
            else if (vfile.is_dataset())
            {
                child->setIcon(0, data_icon);
            }
            else if (vfile.is_document())
            {
                child->setIcon(0, doc_icon);
            }
            else
            {
                child->setIcon(0, file_icon);
            }
            item->insertChild(i-1, child);
        }
    }

    item->setExpanded(folder.expanded());
}

static VNode *get_vnode(QTreeWidgetItem *item)
{
    auto tree_item = dynamic_cast<TreeItem*>(item);
    assert(tree_item);

    return tree_item->node;
}

void ProjectCtrl::changeExpanded(QTreeWidgetItem *item)
{
    bool value = isItemExpanded(item);
    auto folder = dynamic_cast<VFolder*>(get_vnode(item));
    assert(folder);
    folder->set_expanded(value);
}

QTreeWidgetItem* ProjectCtrl::childFromParentIndex(const QModelIndex &parent, int index)
{
    QTreeWidgetItem *parent_item  = this->itemFromIndex(parent);

    if (! parent_item)
        return nullptr; /* root item, can't be moved  this->topLevelItem(index); */
    else
        return parent_item->child(index); /* folder */
}

void ProjectCtrl::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsAboutToBeRemoved(parent, start, end);
    auto item = childFromParentIndex(parent, start);

    if (item)
    {
        mv_buffer.push(std::make_unique<Movement>(item));
    }
}

void ProjectCtrl::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    if (!mv_buffer.empty())
    {
        auto mv = std::move(mv_buffer.back());
        mv_buffer.pop();
        mv->new_parent = itemFromIndex(parent);
        mv->new_pos = start;
        drop_queue.push(std::move(mv));
    }
}

void ProjectCtrl::dropEvent(QDropEvent *event)
{
    QTreeWidget::dropEvent(event);
    Q_ASSERT(mv_buffer.empty());

    while (! drop_queue.empty())
    {
        auto mv = std::move(drop_queue.back());
        drop_queue.pop();

        // FIXME: the movement pointer was null once, but I can't reproduce the problem
        if (!mv)
        {
            QMessageBox dlg(QMessageBox::Critical, "Error", "Cannot move item(s): invalid internal state");
            dlg.exec();
            return;
        }

        auto item_vnode = get_vnode(mv->item);
        QTreeWidgetItem *parent_item = mv->new_parent;

        if(! parent_item) continue;

        auto vnode = get_vnode(parent_item);

        if (!vnode->is_folder())
        {
           event->ignore();
        }
        else
        {
            auto old_folder = item_vnode->parent();
            auto new_folder = static_cast<VFolder*>(vnode);

            // Make sure we don't move annotations into the script folder, for example.
            if (old_folder->toplevel() != new_folder->toplevel())
            {
                event->ignore();
            }
            else
            {
                new_folder->set_expanded(true);
                item_vnode->move_to(new_folder, mv->new_pos + 1);
                event->accept();
            }
        }
    }

    refresh();
}

void ProjectCtrl::onItemDoubleClicked(QTreeWidgetItem *item)
{
    auto vnode = get_vnode(item);

    if (vnode->is_script())
    {
        auto script = downcast<Script>(vnode->shared_from_this());
        emit script_selected(script);
    }
    else if (vnode->is_file())
    {
        auto file = downcast<VFile>(vnode->shared_from_this());
        emit view_file(file);
    }
}

void ProjectCtrl::onItemSelected(QTreeWidgetItem *)
{
    auto items = selectedItems();

    if (items.size() == 1)
    {
        auto vnode = get_vnode(items.front());

        if (vnode->is_file())
        {
            auto vf = downcast<VFile>(vnode->shared_from_this());
            emit files_selected({std::move(vf)});

        }
        else if (vnode->is_folder())
        {
            emit no_selection();
        }
    }
    else
    {
        VFileList files = get_vfiles(items);

        if (!files.empty())
        {
            emit files_selected(std::move(files));
        }
        else
        {
            emit no_selection();
        }
    }
}

void ProjectCtrl::onRightClick(const QPoint &pos)
{
    // Make sure the info panel gets updated.
    onItemSelected(nullptr);

    QList<QTreeWidgetItem*> items = selectedItems();
    if (items.isEmpty()) return;

    ContextMenu menu(this);

    if (items.size() == 1)
    {
        auto vnode = get_vnode(items.front());

        if (vnode->is_file())
        {
            auto file = downcast<VFile>(vnode->shared_from_this());

            auto view_action = new QAction(tr("View file"), this);
            menu.addAction(view_action);
            connect(view_action, &QAction::triggered, [=](bool){
                emit view_file(file);
            });
            menu.addSeparator();

            if (file->is_annotation())
            {
                auto annot = downcast<Annotation>(file);

                if (annot->is_textgrid())
                {
                    auto action = new QAction(tr("Open annotation in Praat"), this);
                    menu.addAction(action);
                    connect(action, &QAction::triggered, this, [=](bool) {
                        openInPraat(annot);
                    });
                }
            }
            else if (file->is_sound())
            {
                auto path = file->path();
                auto action = new QAction(tr("Open sound in Praat"), this);
                connect(action, &QAction::triggered, [=](bool) {
                    praat::open_sound(path);
                });
            }
            else if (file->is_script())
            {
                auto script = downcast<Script>(file);
                auto path = script->path();
                auto action = new QAction(tr("Run"), this);
                menu.addAction(action);
                connect(action, &QAction::triggered, this, [=](bool) {
                    if (!path.empty())
                    {
                        try
                        {
                            rt.do_file(path);
                        }
                        catch (std::exception &e)
                        {
                            QMessageBox dlg(QMessageBox::Critical, "Error", e.what());
                            dlg.exec();
                        }
                    }
                });
            }
        }
        else if (vnode->is_folder())
        {
            auto folder = downcast<VFolder>(vnode->shared_from_this());

            auto add_dir = new QAction(tr("Create subfolder..."));
            menu.addAction(add_dir);
            connect(add_dir, &QAction::triggered, [=](bool){
                String name = QInputDialog::getText(this, tr("New folder..."), tr("Folder name:"), QLineEdit::Normal);

                if (!name.empty())
                {
                    folder->add_subfolder(name);
                    Project::updated();
                }
            });

            auto project = Project::instance();
            if (!project->is_root(folder))
            {
                auto rename_dir = new QAction(tr("Rename folder..."), this);
                menu.addAction(rename_dir);
                connect(rename_dir, &QAction::triggered, [=](bool){
                    String name = QInputDialog::getText(this, tr("Rename folder..."), tr("New folder name:"), QLineEdit::Normal);

                    if (!name.empty())
                    {
                        folder->set_label(name);
                        Project::updated();
                    }
                });

                auto remove_dir = new QAction(tr("Remove folder"), this);
                menu.addSeparator();
                menu.addAction(remove_dir);
                connect(remove_dir, &QAction::triggered, [=](bool) mutable {
                    Project::instance()->remove(folder);
                    Project::updated();
                });
            }
        }
    }
    else
    {
        if (items.size() == 2)
        {
            auto n1 = get_vnode(items[0]);
            auto n2 = get_vnode(items[1]);

            if ((n1->is_annotation() && n2->is_sound()) || (n2->is_annotation() && n1->is_sound()))
            {
                std::shared_ptr<Annotation> annot;
                std::shared_ptr<Sound> snd;

                if (n1->is_annotation())
                {
                    annot = downcast<Annotation>(n1->shared_from_this());
                    snd = downcast<Sound>(n2->shared_from_this());
                }
                else
                {
                    annot = downcast<Annotation>(n2->shared_from_this());
                    snd = downcast<Sound>(n1->shared_from_this());
                }

                auto action = new QAction(tr("Bind annotation to sound file"), this);
                menu.addAction(action);
                connect(action, &QAction::triggered, [=](bool) {
                    annot->set_sound(snd);
                    Project::updated();
                });
                menu.addSeparator();
            }
        }
    }

    menu.addSeparator();
    VFileList files = get_vfiles(items);

    if (!files.empty())
    {
        auto action_properties = new QAction(tr("Edit properties..."), this);
        menu.addAction(action_properties);
        menu.addSeparator();
        connect(action_properties, &QAction::triggered, [=](bool) mutable {
            MetadataEditor ed(this, std::move(files));
            ed.exec();
            Project::updated();
        });

        auto action_save = new QAction(tr("Save file(s)"), this);
        bool enable = false;
        for (auto &f : files)
        {
            if (f->modified()) {
                enable = true; break;
            }
        }
        action_save->setEnabled(enable);
        menu.addAction(action_save);
        connect(action_save, &QAction::triggered, [=](bool) mutable {
            for (auto &f : files) {
                f->save();
            }
            Project::updated();
        });

        auto action_remove = new QAction(tr("Remove file(s) from project"), this);
        menu.addAction(action_remove);
        connect(action_remove, &QAction::triggered, [=](bool) mutable {
            Project::instance()->remove(files);
            Project::updated();
        });
    }

    menu.display(pos);
}

void ProjectCtrl::openInPraat(const std::shared_ptr<Annotation> &annot)
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

VFileList ProjectCtrl::get_vfiles(const QList<QTreeWidgetItem *> &items)
{
    VFileList files;

    for (auto item : items)
    {
        auto vnode = get_vnode(item);

        if (vnode->is_file())
        {
            files.append(downcast<VFile>(vnode->shared_from_this()));
        }
    }

    return files;
}

void ProjectCtrl::resetLabel()
{
    auto project = Project::instance();
    String label = project->label();
	if (label.empty())
		label = "Untitled project";
	else
		label.prepend("Project ");
    if (project->modified()) label.append('*');
    setProjectLabel(label);
}


} // phonometrica

