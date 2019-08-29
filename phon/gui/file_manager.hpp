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
 * Purpose: file manager (left panel in the main window).                                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <QFrame>
#include <QLabel>
#include <QDropEvent>
#include <QPushButton>
#include <QStatusBar>
#include <phon/gui/project_ctrl.hpp>

namespace phonometrica {

class FileManager final : public QFrame
{
    Q_OBJECT

public:

    explicit FileManager(Runtime &rt, QWidget *parent = nullptr);

    QPushButton *console_button, *info_button;

    // Update buttons and toggle panel.
    void consoleClicked(bool);

    void infoClicked(bool);

    void initStatusBar(QStatusBar *status_bar);

    ProjectCtrl *tree() { return project_ctrl; }

signals:

    void toggleConsole();

    void toggleInfo();

public slots:

    // Let the file manager update the buttons when the panels are toggled.
    void updateConsoleStatus(bool);

    void updateInfoStatus(bool);

    void refreshProject();

    void refreshLabel();

private:

    Runtime &rt;

    ProjectCtrl *project_ctrl;

#ifdef Q_OS_MAC
    QLabel *label;
#endif

    bool info_shown = true;

    bool console_shown = true;
};

} // phonometrica


#endif // FILE_MANAGER_HPP
