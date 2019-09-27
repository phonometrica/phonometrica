/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: custom tree widget that displays a project.                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROJECT_CTRL_HPP
#define PHONOMETRICA_PROJECT_CTRL_HPP

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

    void openInPraat(const AutoAnnotation &annot);

private:

	void textGridToNative(const AutoAnnotation &annot);

	void nativeToTextGrid(const AutoAnnotation &annot);

    void fillFolder(QTreeWidgetItem *item, VFolder &folder);

    QTreeWidgetItem* childFromParentIndex(const QModelIndex &parent, int index);

    VFileList get_vfiles(const QList<QTreeWidgetItem*> &items);

    void askImportFile(const String &path);

    static void copyPathToClipboard(const String &path);

    Runtime &rt;

    QTreeWidgetItem *corpus,*data, *queries, *scripts, *bookmarks;

    // intercept items being moved in the tree
    std::queue<std::unique_ptr<Movement>> mv_buffer;

    // queue from buffer (items are processed in dropEvent())
    std::queue<std::unique_ptr<Movement>> drop_queue;
};

} // phonometrica


#endif // PHONOMETRICA_PROJECT_CTRL_HPP
