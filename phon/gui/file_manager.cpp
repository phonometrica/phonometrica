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
    setProjectLabel(tr("Untitled project"));
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

void FileManager::setProjectLabel(const QString &name)
{
#ifdef Q_OS_MAC
    label->setText(name);
#else
    project_ctrl->setProjectLabel(name);
#endif
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
    project_ctrl->refresh();
}

void FileManager::initStatusBar(QStatusBar *status_bar)
{
    console_button = new QPushButton(QIcon(":/icons/console.png"), tr("Hide console"));
    console_button->setFlat(true);
    console_button->installEventFilter(this);

    info_button = new QPushButton(QIcon(":/icons/info.png"), tr("Hide metadata"));
    info_button->setFlat(true);

    status_bar->addWidget(console_button);
    status_bar->addWidget(info_button);
}

void FileManager::refreshLabel()
{
    project_ctrl->resetLabel();
}


} // phonometrica

