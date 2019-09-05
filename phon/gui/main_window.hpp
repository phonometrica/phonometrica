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
 * Purpose: main window.                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QSplitter>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/file_manager.hpp>
#include <phon/gui/main_area.hpp>
#include <phon/gui/splitter.hpp>

namespace phonometrica {

class QueryEditor;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(Runtime &rt, QWidget *parent = nullptr);

    ~MainWindow();

public slots:

    void closeEvent (QCloseEvent *event) override;

private slots:

    void showConsole(bool);

    void showInfo(bool);

    void showProject(bool);

    void restoreDefaultLayout(bool);

    void updateConsoleAction(bool);

    void updateInfoAction(bool);

    void adjustSplitters();

    void maximizeViewer();

private:

    bool finalize();

    void makeMenu(QWidget *panel);

    void addWindowMenu(QMenuBar *menubar);

    void setShellFunctions();

    void initialize();
    
    void preInitialize();

    void postInitialize();

    void setStretchFactor(double ratio);

    void adjustProject();

    void openQueryEditor();

    void runLastQuery();

    void cacheQueryEditor(QueryEditor *ed);

    Splitter *splitter;

    Runtime &rt;

    FileManager *file_manager;

    MainArea *main_area;

    QAction *show_project, *show_console, *show_info, *restore_layout;

    QueryEditor *query_editor = nullptr; // cache last query
};

} // phonometrica

#endif // MAIN_WINDOW_HPP
