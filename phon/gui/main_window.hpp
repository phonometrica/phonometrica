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
 * Purpose: main window.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MAIN_WINDOW_HPP
#define PHONOMETRICA_MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QSplitter>
#include <phon/runtime/runtime.hpp>
#include <phon/application/plugin.hpp>
#include <phon/gui/file_manager.hpp>
#include <phon/gui/main_area.hpp>
#include <phon/gui/splitter.hpp>
#include <phon/gui/query_editor.hpp>

namespace phonometrica {

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(Runtime &rt, QWidget *parent = nullptr);

    ~MainWindow();

    void display();

public slots:

    void closeEvent (QCloseEvent *event) override;

    void showConsole(bool);

private slots:

    void showInfo(bool);

    void showProject(bool);

    void restoreDefaultLayout(bool);

    void updateConsoleAction(bool);

    void updateInfoAction(bool);

    void adjustSplitters();

    void maximizeViewer();

    void executeQuery(AutoQuery query);

    void updateStatus(const String &);

    void installPlugin(bool);

    void uninstallPlugin(bool);

private:

    bool finalize();

    void makeMenu(QWidget *panel);

    void addWindowMenu(QMenuBar *menubar);

    void addToolsMenu(QMenuBar *menubar);

    void setToolsMenu();

    void setShellFunctions();

    void initialize();
    
    void preInitialize();

    void postInitialize();

    void setStretchFactor(double ratio);

    void adjustProject();

    void openQueryEditor(Query::Type type);

    void openQueryEditor(AutoProtocol protocol, Query::Type type);

    void runLastQuery();

    void cacheQueryEditor(QueryEditor *ed);

    void setDatabaseConnection();

    void loadPluginsAndScripts(const String &dir);

    void loadPlugin(const String &path);

    void importMetadata();

    Plugin *findPlugin(const String &name);

    Splitter *splitter;

    Runtime &runtime;

    FileManager *file_manager;

    MainArea *main_area;

    QAction *show_project, *show_console, *show_info, *restore_layout;

    QueryEditor *query_editor = nullptr; // cache last query

    QMenu *tools_menu = nullptr;

    QAction *tool_separator = nullptr;

    Array<AutoPlugin> plugins;
};

} // phonometrica

#endif // PHONOMETRICA_MAIN_WINDOW_HPP
