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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <phon/gui/file_manager.hpp>
#include "file_manager.hpp"


namespace phonometrica {

FileManager::FileManager(Runtime &rt, QWidget *parent) :
    QFrame(parent), rt(rt)
{
    auto layout = new QVBoxLayout;
    project_ctrl = new ProjectCtrl(rt);
#ifdef Q_OS_MAC
    label = new QLabel;
    label->setContentsMargins(5, 5, 0, 0);
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);

    label->setStyleSheet("QLabel { background-color : #E5E7EC; }");
    this->setStyleSheet("QFrame { background-color : #E5E7EC; }");
    layout->addWidget(label);
#endif
    layout->addWidget(project_ctrl);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    refreshLabel();
}

void FileManager::updateConsoleStatus(bool status)
{
    if (status)
    {
        console_button->setText(tr("Show console"));
    }
    else
    {
        console_button->setText(tr("Hide console"));
    }
    console_shown = !status;
}

void FileManager::updateInfoStatus(bool status)
{
    if (status)
    {
        info_button->setText(tr("Show metadata"));
    }
    else
    {
        info_button->setText(tr("Hide metadata"));
    }
    info_shown = !status;
}

void FileManager::consoleClicked(bool)
{ 
    emit toggleConsole();
    updateConsoleStatus(console_shown);
}

void FileManager::infoClicked(bool)
{
    emit toggleInfo();
    updateInfoStatus(info_shown);
}

void FileManager::refreshProject()
{
    refreshLabel();
    project_ctrl->refresh();
}

void FileManager::initStatusBar(QStatusBar *status_bar)
{
    console_button = new QPushButton(QIcon(":/icons/console.png"), tr("Hide console"));
    console_button->setFlat(true);
    console_button->installEventFilter(this);

    info_button = new QPushButton(QIcon(":/icons/info.png"), tr("Hide metadata"));
    info_button->setFlat(true);

    status_bar->addPermanentWidget(console_button);
    status_bar->addPermanentWidget(info_button);
}

void FileManager::refreshLabel()
{
    auto project = Project::instance();
    String name = project->label();
    if (project->modified()) name.append('*');

#ifdef Q_OS_MAC
    label->setText(name);
#else
    project_ctrl->setProjectLabel(name);
#endif
}


} // phonometrica

