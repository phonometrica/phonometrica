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
#include <phon/gui/preference_editor.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>
#include "main_window.hpp"


#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/initialize_phon.hpp>
#include <phon/include/event_phon.hpp>
#include <phon/include/menu_phon.hpp>
#endif

namespace phonometrica {

MainWindow::MainWindow(Environment &env, QWidget *parent)
    : QMainWindow(parent), env(env)
{
    preInitialize();
    setShellFunctions();
    initialize();

    auto panel = new QWidget;
    auto layout = new QVBoxLayout;
    splitter = new Splitter(Qt::Horizontal);
    file_manager = new FileManager(env);
    main_area = new MainArea(env);

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
    connect(Project::instance(), &Project::metadata_updated, file_manager, &FileManager::refreshLabel);
    connect(Project::instance(), &Project::metadata_updated, main_area->infoPanel(), &InfoPanel::reset);
    connect(file_manager->tree(), &ProjectCtrl::script_selected, viewer, &Viewer::openScriptView);
    connect(file_manager->tree(), &ProjectCtrl::files_selected, main_area->infoPanel(), &InfoPanel::showSelection);
    connect(file_manager->tree(), &ProjectCtrl::no_selection, main_area->infoPanel(), &InfoPanel::showEmptySelection);
    connect(Project::instance(), &Project::request_save, viewer, &Viewer::saveViews);
    connect(file_manager->tree(), &ProjectCtrl::view_file, viewer, &Viewer::view);

    postInitialize();

	if (Settings::get_boolean(env, "full_screen"))
	{
		showMaximized();
	}
	else
	{
		try
		{
			auto &lst = Settings::get_list(env, "geometry");
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

    if (Settings::get_boolean(env, "hide_console"))
        file_manager->console_button->click();
    if (Settings::get_boolean(env, "hide_info"))
        file_manager->info_button->click();
    if (Settings::get_boolean(env, "hide_project"))
    {
        show_project->setChecked(false);
        showProject(false);
    }

    if (Settings::get_boolean(env, "autoload"))
    {
        env.do_string(R"__(
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
    auto project_ratio = Settings::get_number(env, "project_ratio");
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
    if (!Settings::get_boolean(env, "hide_info"))
        Settings::set_value(env, "info_ratio", info_ratio);
    if (!Settings::get_boolean(env, "hide_console"))
        Settings::set_value(env, "console_ratio", console_ratio);
    if (!Settings::get_boolean(env, "hide_project"))
        Settings::set_value(env, "project_ratio", project_ratio);

    auto geom = this->geometry();
    double x = geom.x();
    double y = geom.y();
    double w = geom.width();
    double h = geom.height();
    Settings::set_value(env, "geometry", { x, y, w, h });
    Settings::set_value(env, "full_screen", this->isMaximized());

    try
    {
        Settings::write(env);
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
    Settings::set_value(env, "hide_project", !new_state);
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

    Settings::set_value(env, "hide_project", false);
    Settings::set_value(env, "hide_info", false);
    Settings::set_value(env, "hide_console", false);
    Settings::set_value(env, "hide_console", false);
    Settings::set_value(env, "console_ratio", DEFAULT_CONSOLE_RATIO);
    Settings::set_value(env, "project_ratio", DEFAULT_FILE_MANAGER_RATIO);
    Settings::set_value(env, "info_ratio", DEFAULT_INFO_RATIO);
}

void MainWindow::makeMenu(QWidget *panel)
{
    auto menubar = new QMenuBar(panel);

    auto add_menu = [=](Environment &env) {
        QString name = env.to_string(-1);
        auto menu = new QMenu(name);
        menu->setToolTipsVisible(true);
        menubar->addMenu(menu);
        env.push_null(); // prototype
        env.new_user_data("Menu", menu);
    };

    auto add_submenu = [](Environment &env) {
        auto menu = env.cast_user_data<QMenu*>(1);
        QString name = env.to_string(2);
        auto submenu = menu->addMenu(name);
        submenu->setToolTipsVisible(true);
        env.push_null(); // prototype
        env.new_user_data("Menu", submenu);
    };

    auto clear_menu = [](Environment &env) {
        auto menu = env.cast_user_data<QMenu*>(1);
        menu->clear();
        env.push_null();
    };

    auto enable_menu = [](Environment &env) {
        auto menu = env.cast_user_data<QMenu*>(1);
        auto value = env.to_boolean(2);
        menu->setEnabled(value);
        env.push_null();
    };

    auto enable_action = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        auto value = env.to_boolean(2);
        action->setEnabled(value);
        env.push_null();
    };

    auto add_action = [](Environment &env) {
        auto menu = env.cast_user_data<QMenu*>(1);
        auto label = env.to_string(2);
        auto action = menu->addAction(label);
        env.push_null(); // prototype
        env.new_user_data("Action", action);
    };

    auto add_separator = [](Environment &env) {
        auto menu = env.cast_user_data<QMenu*>(1);
        menu->addSeparator();
        env.push_null();
    };

    auto set_checkable = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        auto value = env.to_boolean(2);
        action->setCheckable(value);
        env.push_null();
    };

    auto check_action = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        auto value = env.to_boolean(2);
        action->setChecked(value);
        env.push_null();
    };

    auto set_action_tooltip = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        auto value = env.to_string(2);
        action->setToolTip(value);
        env.push_null();
    };

    auto set_action_shortcut = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        auto label = env.to_string(2);
        action->setShortcut(QKeySequence(label));
        env.push_null();
    };

    auto bind_action = [](Environment &env) {
        auto action = env.cast_user_data<QAction*>(1);
        if (!env.is_callable(2)) {
            throw env.raise("Type error", "expected a callable object");
        }
        Value val(env, 2); // capture function to protect from the GC, and move it to the callback.

        connect(action, &QAction::triggered, [&env, val{std::move(val)}](bool triggered) {
            env.push(*val);
            env.push_null();
            env.push_boolean(triggered);
            env.call(1);
        });
    };

    auto create_window_menu = [=](Environment &env) {
        this->addWindowMenu(menubar);
    };

    env.get_global("phon");
    {
        env.add_method("add_menu", add_menu, 1);
        env.add_method("add_submenu", add_submenu, 2);
        env.add_method("clear_menu", clear_menu, 1);
        env.add_method("enable_menu", enable_menu, 2);
        env.add_method("enable_action", enable_action, 2);
        env.add_method("add_action", add_action, 2);
        env.add_method("add_separator", add_separator, 0);
        env.add_method("set_checkable", set_checkable, 2);
        env.add_method("check_action", check_action, 2);
        env.add_method("set_action_shortcut", set_action_shortcut, 2);
        env.add_method("set_action_tooltip", set_action_tooltip, 2);
        env.add_method("bind_action", bind_action, 2);
        env.add_method("create_window_menu", create_window_menu, 0);
    }
    env.pop();

    try
    {
        run_script(env, menu);
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
    auto warning = [](Environment &env) {
        auto msg = env.to_string(-1);
        QMessageBox dlg(QMessageBox::Warning, "Warning", msg);
        dlg.exec();
        env.push_null();
    };

    auto alert = [](Environment &env) {
        auto msg = env.to_string(-1);
        QMessageBox dlg(QMessageBox::Critical, "Error", msg);
        dlg.exec();
        env.push_null();
    };

    auto info = [](Environment &env) {
        auto msg = env.to_string(-1);
        QMessageBox dlg(QMessageBox::Information, "Information", msg);
        dlg.exec();
        env.push_null();
    };

    auto about = [=](Environment &env) {
        auto title = env.to_string(1);
        auto msg = env.to_string(2);
        QMessageBox::about(this, title, msg);
        env.push_null();
    };

    auto open_file_dialog = [=](Environment &env) {
        auto msg = env.to_string(1);
        auto dir = Settings::get_last_directory(env);
        String filter = env.arg_count() > 1 ? env.to_string(2) : String();
        auto path = QFileDialog::getOpenFileName(this, msg, dir, filter);
        if (path.isNull())
        {
            env.push_null();
        }
        else
        {
            Settings::set_last_directory(env, path);
            env.push(path);
        }
    };

    auto open_files_dialog = [=](Environment &env) {
        auto msg = env.to_string(1);
        auto dir = Settings::get_last_directory(env);
        String filter = env.arg_count() > 1 ? env.to_string(2) : String();
        auto paths = QFileDialog::getOpenFileNames(this, msg, dir, filter);
        Array<Variant> result;
        bool done = false;

        for (auto &path : paths)
        {
            result.append(String(path));
            if (!done)
            {
                Settings::set_last_directory(env, path);
                done = true;
            }
        }

        std::sort(result.begin(), result.end());
        env.push(std::move(result));
    };

    auto open_directory_dialog = [=](Environment &env) {
        auto s = env.to_string(-1);
        auto dir = Settings::get_last_directory(env);
        auto path = QFileDialog::getExistingDirectory(this, s, dir);
        if (path.isNull())
        {
            env.push_null();
        }
        else
        {
            Settings::set_last_directory(env, path);
            env.push(path);
        }
    };

    auto save_file_dialog = [=](Environment &env) {
        auto s = env.to_string(-1);
        auto dir = Settings::get_last_directory(env);
        String filter = env.arg_count() > 1 ? env.to_string(2) : String();
        auto path = QFileDialog::getSaveFileName(this, s, dir, filter);
        if (path.isNull())
        {
            env.push_null();
        }
        else
        {
            Settings::set_last_directory(env, path);
            env.push(path);
        }
    };

    auto input = [=](Environment &env) {
        auto title = env.to_string(1);
        auto text = env.to_string(2);
        auto label = env.to_string(3);
        auto result = QInputDialog::getText(this, title, text, QLineEdit::Normal, label);
        env.push(result);
    };

    auto message = [=](Environment &env) {
        auto title = env.to_string(1);
        auto msg = env.to_string(2);
        QMessageBox dlg(QMessageBox::Information, title, msg);
        dlg.exec();
        env.push_null();
    };

    auto show_doc = [=](Environment &env) {
        auto path = env.to_string(1);
        main_area->viewer()->showDocumentation(path);
        env.push_null();
    };

    auto get_version = [](Environment &env) {
        env.push(utils::get_version());
    };

    auto get_date = [](Environment &env) {
        env.push(utils::get_date());
    };

    auto get_supported_sound_formats = [](Environment &env) {
        Array<Variant> sounds;
        for (auto &s : Sound::supported_sound_format_names()) {
            sounds.append(s);
        }
        env.push(std::move(sounds));
    };

    auto get_rtaudio_version = [](Environment &env) {
        env.push(Sound::rtaudio_version());
    };

    auto get_libsndfile_version = [](Environment &env) {
        env.push(Sound::libsndfile_version());
    };

    auto quit = [=](Environment &env) {
        if (this->finalize()) {
            QApplication::quit();
        }
    };

    auto edit_settings = [=](Environment &env) {
        PreferenceEditor dlg(this, env);
        dlg.exec();
        env.push_null();
    };

    auto view_script = [=](Environment &env) {
        auto viewer = main_area->viewer();
        if (env.is_null(1)) {
            viewer->newScript();
        }
        else
        {
            auto path = env.to_string(1);
            Project::instance()->import_file(path);
            viewer->openScript(path);
        }

        env.push_null();
    };

    auto run_script = [=](Environment &env) {
        auto path = env.to_string(1);
        try
        {
            env.do_file(path);
        }
        catch (std::runtime_error &e)
        {
            QMessageBox dlg(QMessageBox::Critical, tr("Execution failed"), e.what());
            dlg.exec();
        }
    };

    env.get_global("phon");
    {
        env.add_method("warning", warning, 1);
        env.add_method("alert", alert, 1);
        env.add_method("info", info, 1);
        env.add_method("about", about, 2);
        env.add_method("message", message, 2);
        env.add_method("open_file_dialog", open_file_dialog, 1);
        env.add_method("open_files_dialog", open_files_dialog, 1);
        env.add_method("open_directory_dialog", open_directory_dialog, 1);
        env.add_method("save_file_dialog", save_file_dialog, 1);
        env.add_method("input", input, 3);
        env.add_method("show_documentation", show_doc, 1);
        env.add_accessor("version", get_version);
        env.add_accessor("date", get_date);
        env.add_accessor("supported_sound_formats", get_supported_sound_formats);
        env.add_accessor("rtaudio_version", get_rtaudio_version);
        env.add_accessor("libsndfile_version", get_libsndfile_version);
        env.add_method("quit", quit, 0);
        env.add_method("edit_settings", edit_settings, 0);
        env.add_method("view_script", view_script, 1);
        env.add_method("run_script", run_script, 1);

        // Define 'phon.config'
        Settings::initialize(env);
    }
    env.pop();
}

void MainWindow::initialize()
{
    try
    {
        run_script(env, initialize);
        run_script(env, event);
        Project::create(env);
        Project::initialize(env);
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
        env.do_string("phon = {}");
        Settings::read(env);
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
        env.do_string("phon.load_plugins()");
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
        auto autosave = Settings::get_boolean(env, "autosave");

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

        env.do_string("phon.save_project()");
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


} // phonometrica

