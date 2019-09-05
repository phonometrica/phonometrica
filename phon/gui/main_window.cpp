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

#include <QStatusBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <phon/gui/main_window.hpp>
#include <phon/gui/query_editor.hpp>
#include <phon/gui/preference_editor.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>


#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/initialize_phon.hpp>
#include <phon/include/event_phon.hpp>
#include <phon/include/menu_phon.hpp>
#endif

namespace phonometrica {

MainWindow::MainWindow(Runtime &rt, QWidget *parent)
    : QMainWindow(parent), rt(rt)
{
    preInitialize();
    setShellFunctions();
    initialize();

    auto panel = new QWidget;
    auto layout = new QVBoxLayout;
    splitter = new Splitter(Qt::Horizontal);
    file_manager = new FileManager(rt);
    main_area = new MainArea(rt);

    splitter->addWidget(file_manager);
    splitter->addWidget(main_area);

    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);
    panel->setLayout(layout);
    setCentralWidget(panel);

    auto status_bar = this->statusBar();
    status_bar->setMaximumHeight(25);
    file_manager->initStatusBar(status_bar);
    status_bar->show();
    status_bar->setContentsMargins(0, 0, 0, 0);

    makeMenu(panel);
    // This will not be layed out properly, but we set a timer at the end of the constructor to refresh the splitters.
    adjustSplitters();

    auto viewer = main_area->viewer();
    connect(main_area->console(), &Console::shown, file_manager, &FileManager::updateConsoleStatus);
    connect(main_area->infoPanel(), &InfoPanel::shown, file_manager, &FileManager::updateInfoStatus);
    connect(file_manager, &FileManager::toggleConsole, main_area, &MainArea::toggleConsole);
    connect(file_manager, &FileManager::toggleInfo, main_area, &MainArea::toggleInfo);
    connect(Project::instance(), &Project::notify_update, file_manager, &FileManager::refreshProject);
	connect(Project::instance(), &Project::notify_closed, viewer, &Viewer::closeAll);
    connect(Project::instance(), &Project::metadata_updated, file_manager, &FileManager::refreshLabel);
    connect(Project::instance(), &Project::metadata_updated, main_area->infoPanel(), &InfoPanel::reset);
    connect(file_manager->tree(), &ProjectCtrl::script_selected, viewer, &Viewer::openScriptView);
    connect(file_manager->tree(), &ProjectCtrl::files_selected, main_area->infoPanel(), &InfoPanel::showSelection);
    connect(file_manager->tree(), &ProjectCtrl::no_selection, main_area->infoPanel(), &InfoPanel::showEmptySelection);
    connect(Project::instance(), &Project::request_save, viewer, &Viewer::saveViews);
    connect(file_manager->tree(), &ProjectCtrl::view_file, viewer, &Viewer::view);

    postInitialize();

	if (Settings::get_boolean(rt, "full_screen"))
	{
		showMaximized();
	}
	else
	{
		try
		{
			auto &lst = Settings::get_list(rt, "geometry");
			auto x = int(lst.at(1).to_number());
			auto y = int(lst.at(2).to_number());
			auto w = int(lst.at(3).to_number());
			auto h = int(lst.at(4).to_number());
			setGeometry(x, y, w, h);
		}
		catch (...)
		{
			showMaximized();
		}
	}

    if (Settings::get_boolean(rt, "hide_console"))
        file_manager->console_button->click();
    if (Settings::get_boolean(rt, "hide_info"))
        file_manager->info_button->click();
    if (Settings::get_boolean(rt, "hide_project"))
    {
        show_project->setChecked(false);
        showProject(false);
    }

    if (Settings::get_boolean(rt, "autoload"))
    {
        rt.do_string(R"__(
	var recent = phon.settings.recent_projects
	if not recent.is_empty() then
		phon.project.open(recent[1])
	end)__");
    }

    main_area->focusConsole();

    // FIXME: We need to delay splitter adjustment, otherwise they won't show up in the right place.
    //  See: https://stackoverflow.com/questions/28795329/qsplitter-sizes-indicates-wrong-sizes
    QTimer::singleShot(50, this, SLOT(adjustSplitters()));
}

void MainWindow::adjustProject()
{
    auto project_ratio = Settings::get_number(rt, "project_ratio");
    setStretchFactor(project_ratio);
}

MainWindow::~MainWindow()
{
    auto sizes = splitter->sizes();
    auto project_ratio = double(sizes[0]) / (sizes[0] + sizes[1]);
    double info_ratio, console_ratio;
    std::tie(info_ratio, console_ratio) = main_area->ratios();

    // Don't update ratio if a widget is hidden, otherwise when we try to unhide them,
    // they won't be shown because their ratio is 0.
    if (!Settings::get_boolean(rt, "hide_info"))
        Settings::set_value(rt, "info_ratio", info_ratio);
    if (!Settings::get_boolean(rt, "hide_console"))
        Settings::set_value(rt, "console_ratio", console_ratio);
    if (!Settings::get_boolean(rt, "hide_project"))
        Settings::set_value(rt, "project_ratio", project_ratio);

    auto geom = this->geometry();
    double x = geom.x();
    double y = geom.y();
    double w = geom.width();
    double h = geom.height();
    Settings::set_value(rt, "geometry", { x, y, w, h });
    Settings::set_value(rt, "full_screen", this->isMaximized());

    try
    {
        Settings::write(rt);
    }
    catch (std::runtime_error &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Finalization failed"), e.what());
        dlg.exec();
    }
}

void MainWindow::showConsole(bool value)
{
    file_manager->consoleClicked(!value);
}

void MainWindow::showInfo(bool value)
{
    file_manager->infoClicked(!value);
}

void MainWindow::showProject(bool)
{
    bool new_state = !file_manager->isVisible();
    file_manager->setVisible(new_state);
    Settings::set_value(rt, "hide_project", !new_state);
}

void MainWindow::restoreDefaultLayout(bool)
{
    if (file_manager->isHidden()) {
        file_manager->show();
    }
    if (main_area->console()->isHidden()) {
        main_area->console()->show();
    }
    if (main_area->infoPanel()->isHidden()) {
        main_area->infoPanel()->show();
    }

    show_project->setChecked(true);
    show_console->setChecked(true);
    show_info->setChecked(true);

    // Restore default geometry.
    setStretchFactor(DEFAULT_FILE_MANAGER_RATIO);
    main_area->setDefaultLayout();

    Settings::set_value(rt, "hide_project", false);
    Settings::set_value(rt, "hide_info", false);
    Settings::set_value(rt, "hide_console", false);
    Settings::set_value(rt, "hide_console", false);
    Settings::set_value(rt, "console_ratio", DEFAULT_CONSOLE_RATIO);
    Settings::set_value(rt, "project_ratio", DEFAULT_FILE_MANAGER_RATIO);
    Settings::set_value(rt, "info_ratio", DEFAULT_INFO_RATIO);
}

void MainWindow::makeMenu(QWidget *panel)
{
    auto menubar = new QMenuBar(panel);

    auto add_menu = [=](Runtime &rt) {
        QString name = rt.to_string(-1);
        auto menu = new QMenu(name);
        menu->setToolTipsVisible(true);
        menubar->addMenu(menu);
        rt.push_null(); // prototype
        rt.new_user_data("Menu", menu);
    };

    auto add_submenu = [](Runtime &rt) {
        auto menu = rt.cast_user_data<QMenu*>(1);
        QString name = rt.to_string(2);
        auto submenu = menu->addMenu(name);
        submenu->setToolTipsVisible(true);
        rt.push_null(); // prototype
        rt.new_user_data("Menu", submenu);
    };

    auto clear_menu = [](Runtime &rt) {
        auto menu = rt.cast_user_data<QMenu*>(1);
        menu->clear();
        rt.push_null();
    };

    auto enable_menu = [](Runtime &rt) {
        auto menu = rt.cast_user_data<QMenu*>(1);
        auto value = rt.to_boolean(2);
        menu->setEnabled(value);
        rt.push_null();
    };

    auto enable_action = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        auto value = rt.to_boolean(2);
        action->setEnabled(value);
        rt.push_null();
    };

    auto add_action = [](Runtime &rt) {
        auto menu = rt.cast_user_data<QMenu*>(1);
        auto label = rt.to_string(2);
        auto action = menu->addAction(label);
        rt.push_null(); // prototype
        rt.new_user_data("Action", action);
    };

    auto add_separator = [](Runtime &rt) {
        auto menu = rt.cast_user_data<QMenu*>(1);
        menu->addSeparator();
        rt.push_null();
    };

    auto set_checkable = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        auto value = rt.to_boolean(2);
        action->setCheckable(value);
        rt.push_null();
    };

    auto check_action = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        auto value = rt.to_boolean(2);
        action->setChecked(value);
        rt.push_null();
    };

    auto set_action_tooltip = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        auto value = rt.to_string(2);
        action->setToolTip(value);
        rt.push_null();
    };

    auto set_action_shortcut = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        auto label = rt.to_string(2);
        action->setShortcut(QKeySequence(label));
        rt.push_null();
    };

    auto bind_action = [](Runtime &rt) {
        auto action = rt.cast_user_data<QAction*>(1);
        if (!rt.is_callable(2)) {
            throw rt.raise("Type error", "expected a callable object");
        }
        Value val(rt, 2); // capture function to protect from the GC, and move it to the callback.

        connect(action, &QAction::triggered, [&rt, val{std::move(val)}](bool triggered) {
            rt.push(*val);
            rt.push_null();
            rt.push_boolean(triggered);
            rt.call(1);
        });
    };

    auto create_window_menu = [=](Runtime &rt) {
        this->addWindowMenu(menubar);
    };

    rt.get_global("phon");
    {
        rt.add_method("add_menu", add_menu, 1);
        rt.add_method("add_submenu", add_submenu, 2);
        rt.add_method("clear_menu", clear_menu, 1);
        rt.add_method("enable_menu", enable_menu, 2);
        rt.add_method("enable_action", enable_action, 2);
        rt.add_method("add_action", add_action, 2);
        rt.add_method("add_separator", add_separator, 0);
        rt.add_method("set_checkable", set_checkable, 2);
        rt.add_method("check_action", check_action, 2);
        rt.add_method("set_action_shortcut", set_action_shortcut, 2);
        rt.add_method("set_action_tooltip", set_action_tooltip, 2);
        rt.add_method("bind_action", bind_action, 2);
        rt.add_method("create_window_menu", create_window_menu, 0);
    }
    rt.pop();

    try
    {
        run_script(rt, menu);
    }
    catch (std::exception &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Menu creation failed"), e.what());
        dlg.exec();
    }

    setMenuBar(menubar);
}


void MainWindow::addWindowMenu(QMenuBar *menubar)
{
    auto menu = new QMenu(tr("&Window"));
    show_project = menu->addAction(tr("Show project"));
    show_console = menu->addAction(tr("Show console"));
    show_info = menu->addAction(tr("Show metadata"));
    menu->addSeparator();
    auto maximize = menu->addAction(tr("Maximize viewer"));
    menu->addSeparator();
    restore_layout = menu->addAction(tr("Restore default layout"));

    show_project->setCheckable(true);
    show_project->setChecked(true);
    show_console->setCheckable(true);
    show_console->setChecked(true);
    show_info->setCheckable(true);
    show_info->setChecked(true);
    show_project->setShortcut(QKeySequence("ctrl+alt+p"));
    show_console->setShortcut(QKeySequence("ctrl+alt+c"));
    show_info->setShortcut(QKeySequence("ctrl+alt+i"));
    restore_layout->setShortcut(QKeySequence("ctrl+alt+d"));


    connect(show_project, &QAction::triggered, this, &MainWindow::showProject);
    connect(show_console, &QAction::triggered, this, &MainWindow::showConsole);
    connect(show_info, &QAction::triggered, this, &MainWindow::showInfo);
    connect(restore_layout, &QAction::triggered, this, &MainWindow::restoreDefaultLayout);
    connect(file_manager->console_button, &QPushButton::clicked, this, &MainWindow::updateConsoleAction);
    connect(file_manager->info_button, &QPushButton::clicked, this, &MainWindow::updateInfoAction);
    connect(maximize, &QAction::triggered, this, &MainWindow::maximizeViewer);

    menubar->addMenu(menu);
}

void MainWindow::setShellFunctions()
{
    auto warning = [](Runtime &rt) {
        auto msg = rt.to_string(-1);
        QMessageBox dlg(QMessageBox::Warning, "Warning", msg);
        dlg.exec();
        rt.push_null();
    };

    auto alert = [](Runtime &rt) {
        auto msg = rt.to_string(-1);
        QMessageBox dlg(QMessageBox::Critical, "Error", msg);
        dlg.exec();
        rt.push_null();
    };

    auto info = [](Runtime &rt) {
        auto msg = rt.to_string(-1);
        QMessageBox dlg(QMessageBox::Information, "Information", msg);
        dlg.exec();
        rt.push_null();
    };

    auto about = [=](Runtime &rt) {
        auto title = rt.to_string(1);
        auto msg = rt.to_string(2);
        QMessageBox::about(this, title, msg);
        rt.push_null();
    };

    auto open_file_dialog = [=](Runtime &rt) {
        auto msg = rt.to_string(1);
        auto dir = Settings::get_last_directory(rt);
        String filter = rt.arg_count() > 1 ? rt.to_string(2) : String();
        auto path = QFileDialog::getOpenFileName(this, msg, dir, filter);
        if (path.isNull())
        {
            rt.push_null();
        }
        else
        {
            Settings::set_last_directory(rt, path);
            rt.push(path);
        }
    };

    auto open_files_dialog = [=](Runtime &rt) {
        auto msg = rt.to_string(1);
        auto dir = Settings::get_last_directory(rt);
        String filter = rt.arg_count() > 1 ? rt.to_string(2) : String();
        auto paths = QFileDialog::getOpenFileNames(this, msg, dir, filter);
        Array<Variant> result;
        bool done = false;

        for (auto &path : paths)
        {
            result.append(String(path));
            if (!done)
            {
                Settings::set_last_directory(rt, path);
                done = true;
            }
        }

        std::sort(result.begin(), result.end());
        rt.push(std::move(result));
    };

    auto open_directory_dialog = [=](Runtime &rt) {
        auto s = rt.to_string(-1);
        auto dir = Settings::get_last_directory(rt);
        auto path = QFileDialog::getExistingDirectory(this, s, dir);
        if (path.isNull())
        {
            rt.push_null();
        }
        else
        {
            Settings::set_last_directory(rt, path);
            rt.push(path);
        }
    };

    auto save_file_dialog = [=](Runtime &rt) {
        auto s = rt.to_string(-1);
        auto dir = Settings::get_last_directory(rt);
        String filter = rt.arg_count() > 1 ? rt.to_string(2) : String();
        auto path = QFileDialog::getSaveFileName(this, s, dir, filter);
        if (path.isNull())
        {
            rt.push_null();
        }
        else
        {
            Settings::set_last_directory(rt, path);
            rt.push(path);
        }
    };

    auto open_query_editor = [=](Runtime &rt) {
    	this->openQueryEditor();
    	rt.push_null();
    };

    auto input = [=](Runtime &rt) {
        auto title = rt.to_string(1);
        auto text = rt.to_string(2);
        auto label = rt.to_string(3);
        auto result = QInputDialog::getText(this, title, text, QLineEdit::Normal, label);
        rt.push(result);
    };

    auto message = [=](Runtime &rt) {
        auto title = rt.to_string(1);
        auto msg = rt.to_string(2);
        QMessageBox dlg(QMessageBox::Information, title, msg);
        dlg.exec();
        rt.push_null();
    };

    auto show_doc = [=](Runtime &rt) {
        auto path = rt.to_string(1);
        main_area->viewer()->showDocumentation(path);
        rt.push_null();
    };

    auto get_version = [](Runtime &rt) {
        rt.push(utils::get_version());
    };

    auto get_date = [](Runtime &rt) {
        rt.push(utils::get_date());
    };

    auto get_supported_sound_formats = [](Runtime &rt) {
        Array<Variant> sounds;
        for (auto &s : Sound::supported_sound_format_names()) {
            sounds.append(s);
        }
        rt.push(std::move(sounds));
    };

    auto get_rtaudio_version = [](Runtime &rt) {
        rt.push(Sound::rtaudio_version());
    };

    auto get_libsndfile_version = [](Runtime &rt) {
        rt.push(Sound::libsndfile_version());
    };

    auto quit = [=](Runtime &rt) {
        if (this->finalize()) {
            QApplication::quit();
        }
    };

    auto edit_settings = [=](Runtime &rt) {
        PreferenceEditor dlg(this, rt);
        dlg.exec();
        rt.push_null();
    };

    auto view_script = [=](Runtime &rt) {
        auto viewer = main_area->viewer();
        if (rt.is_null(1)) {
            viewer->newScript();
        }
        else
        {
            auto path = rt.to_string(1);
            Project::instance()->import_file(path);
            viewer->openScript(path);
        }

        rt.push_null();
    };

    auto run_script = [=](Runtime &rt) {
        auto path = rt.to_string(1);
        try
        {
            rt.do_file(path);
        }
        catch (std::runtime_error &e)
        {
            QMessageBox dlg(QMessageBox::Critical, tr("Execution failed"), e.what());
            dlg.exec();
        }
    };

    rt.get_global("phon");
    {
        rt.add_method("warning", warning, 1);
        rt.add_method("alert", alert, 1);
        rt.add_method("info", info, 1);
        rt.add_method("about", about, 2);
        rt.add_method("message", message, 2);
        rt.add_method("open_file_dialog", open_file_dialog, 1);
        rt.add_method("open_files_dialog", open_files_dialog, 1);
        rt.add_method("open_directory_dialog", open_directory_dialog, 1);
        rt.add_method("save_file_dialog", save_file_dialog, 1);
        rt.add_method("input", input, 3);
        rt.add_method("show_documentation", show_doc, 1);
        rt.add_accessor("version", get_version);
        rt.add_accessor("date", get_date);
        rt.add_accessor("supported_sound_formats", get_supported_sound_formats);
        rt.add_accessor("rtaudio_version", get_rtaudio_version);
        rt.add_accessor("libsndfile_version", get_libsndfile_version);
        rt.add_method("quit", quit, 0);
        rt.add_method("edit_settings", edit_settings, 0);
        rt.add_method("view_script", view_script, 1);
        rt.add_method("run_script", run_script, 1);
        rt.add_method("open_query_editor", open_query_editor, 0);

        // Define 'phon.config'
        Settings::initialize(rt);
    }
    rt.pop();
}

void MainWindow::initialize()
{
    try
    {
        run_script(rt, initialize);
        run_script(rt, event);
        Project::create(rt);
        Project::initialize(rt);
    }
    catch (std::runtime_error &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Initialization failed"), e.what());
        dlg.exec();
    }
}

void MainWindow::setStretchFactor(double ratio)
{
    file_manager->setMinimumWidth(100);
    auto width = splitter->width();
    auto project_width = int(width * ratio);
    splitter->setSizes({project_width, width-project_width});
}

void MainWindow::preInitialize()
{
    try
    {
        rt.do_string("phon = {}");
        Settings::read(rt);
    }
    catch (std::runtime_error &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Post-initialization failed"), e.what());
        dlg.exec();
    }
}

void MainWindow::postInitialize()
{
    try
    {
        rt.do_string("phon.load_plugins()");
    }
    catch (std::runtime_error &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Plugin initialization failed"), e.what());
        dlg.exec();
    }
}

void MainWindow::updateConsoleAction(bool)
{
    show_console->trigger();
}

void MainWindow::updateInfoAction(bool)
{
    show_info->trigger();
}

bool MainWindow::finalize()
{
    auto project = Project::instance();

    if (project->modified())
    {
        auto autosave = Settings::get_boolean(rt, "autosave");

        if (!autosave)
        {
            auto reply = QMessageBox::question(this, "Save project?", "Your project has unsaved changes. Do you want to save them?",
                    QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

            if (reply == QMessageBox::No)
            {
                return true;
            }
            else if (reply == QMessageBox::Cancel)
            {
                return false;
            }
            // If reply == Yes, fall through and save project.
        }

        rt.do_string("phon.save_project()");
    }

    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (this->finalize())
        event->accept();
    else
        event->ignore();
}

void MainWindow::adjustSplitters()
{
    this->adjustProject();
    main_area->adjustSplitters();
}

void MainWindow::maximizeViewer()
{
    file_manager->hide();
    main_area->console()->hide();
    main_area->infoPanel()->hide();
    show_project->setChecked(false);
    show_console->setChecked(false);
    show_info->setChecked(false);
}

void MainWindow::openQueryEditor()
{
	auto editor = new QueryEditor(this);
	editor->resize(1100, 800);
	editor->show();
}


} // phonometrica

