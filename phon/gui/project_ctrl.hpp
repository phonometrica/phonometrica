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
 * Purpose: custom tree widget that displays a project.                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PROJECT_CTRL_HPP
#define PROJECT_CTRL_HPP

#include <queue>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <phon/definitions.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

struct Movement
{
    Movement(QTreeWidgetItem *item) : item(item) { }
	QTreeWidgetItem *item, *new_parent = nullptr;
	int new_pos = 0;
};



class ProjectCtrl : public QTreeWidget
{
    Q_OBJECT

public:

    explicit ProjectCtrl(Runtime &rt, QWidget *parent = nullptr);

    void resetLabel();

signals:

    void script_selected(std::shared_ptr<Script>);

    void files_selected(VFileList);

    void no_selection();

    void view_file(const std::shared_ptr<VFile> &);

public slots:

    void setProjectLabel(const QString &name);

    void refresh();

    // Instead of using a custom model, we monitor the following methods to keep
    // the project file system and the tree in sync.
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;

    void dropEvent(QDropEvent *event) override;

private slots:

    void changeExpanded(QTreeWidgetItem *item);

    void onItemDoubleClicked(QTreeWidgetItem*);

    void onItemSelected(QTreeWidgetItem*);

    void onRightClick(const QPoint &);

    void openInPraat(const std::shared_ptr<Annotation> &annot);

private:

    void fillFolder(QTreeWidgetItem *item, VFolder &folder);

    QTreeWidgetItem* childFromParentIndex(const QModelIndex &parent, int index);

    VFileList get_vfiles(const QList<QTreeWidgetItem*> &items);

    Runtime &rt;

    QTreeWidgetItem *corpus,*data, *scripts, *bookmarks;

    // intercept items being moved in the tree
    std::queue<std::unique_ptr<Movement>> mv_buffer;

    // queue from buffer (items are processed in dropEvent())
    std::queue<std::unique_ptr<Movement>> drop_queue;
};

} // phonometrica


#endif // PROJECT_CTRL_HPP
