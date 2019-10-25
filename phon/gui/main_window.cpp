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
#include <phon/runtime/variant.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/gui/preference_editor.hpp>
#include <phon/gui/csv_import_dialog.hpp>
#include <phon/gui/user_dialog.hpp>
#include <phon/gui/text_viewer.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>
#include <phon/include/speech_analysis_phon.hpp>
#include <phon/include/transphon_phon.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/any.hpp>
#include <phon/utils/zip.hpp>

#ifdef PHON_EMBED_SCRIPTS
#include <phon/include/initialize_phon.hpp>
#include <phon/include/event_phon.hpp>
#include <phon/include/menu_phon.hpp>
#endif

namespace phonometrica {

MainWindow::MainWindow(Runtime &rt, QWidget *parent)
    : QMainWindow(parent), runtime(rt)
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
    connect(main_area->console(), &Console::shown, this, &MainWindow::showConsole);
	connect(main_area->console(), &Console::shown, [this](bool value) { show_console->setChecked(!value); });
    connect(main_area->infoPanel(), &InfoPanel::shown, file_manager, &FileManager::updateInfoStatus);
    connect(file_manager, &FileManager::toggleConsole, main_area, &MainArea::toggleConsole);
    connect(file_manager, &FileManager::toggleInfo, main_area, &MainArea::toggleInfo);
    connect(Project::instance(), &Project::notify_update, file_manager, &FileManager::refreshProject);
	connect(Project::instance(), &Project::notify_closed, viewer, &Viewer::closeAll);
    connect(Project::instance(), &Project::metadata_updated, file_manager, &FileManager::refreshLabel);
    connect(Project::instance(), &Project::metadata_updated, main_area->infoPanel(), &InfoPanel::reset);
    connect(Project::instance(), &Project::initialized, this, &MainWindow::setDatabaseConnection);
    connect(file_manager->tree(), &ProjectCtrl::script_selected, viewer, &Viewer::openScriptView);
    connect(file_manager->tree(), &ProjectCtrl::files_selected, main_area->infoPanel(), &InfoPanel::showSelection);
    connect(file_manager->tree(), &ProjectCtrl::no_selection, main_area->infoPanel(), &InfoPanel::showEmptySelection);
    connect(Project::instance(), &Project::request_save, viewer, &Viewer::saveViews);
    connect(file_manager->tree(), &ProjectCtrl::view_file, viewer, &Viewer::view);
    connect(file_manager->tree(), &ProjectCtrl::view_annotation, viewer, &Viewer::editAnnotation);
    connect(main_area->viewer(), &Viewer::statusMessage, this, &MainWindow::updateStatus);

    setDatabaseConnection();
}

void MainWindow::updateStatus(const String &msg)
{
	statusBar()->showMessage(msg, 2000);
}

void MainWindow::adjustProject()
{
    auto project_ratio = Settings::get_number(runtime, "project_ratio");
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
    if (!Settings::get_boolean(runtime, "hide_info"))
        Settings::set_value(runtime, "info_ratio", info_ratio);
    if (!Settings::get_boolean(runtime, "hide_console"))
        Settings::set_value(runtime, "console_ratio", console_ratio);
    if (!Settings::get_boolean(runtime, "hide_project"))
        Settings::set_value(runtime, "project_ratio", project_ratio);

    auto geom = this->geometry();
    double x = geom.x();
    double y = geom.y();
    double w = geom.width();
    double h = geom.height();
    Settings::set_value(runtime, "geometry", {x, y, w, h });
    Settings::set_value(runtime, "full_screen", this->isMaximized());

    if (query_editor) delete query_editor;

    try
    {
        Settings::write(runtime);
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
    Settings::set_value(runtime, "hide_project", !new_state);
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

    Settings::set_value(runtime, "hide_project", false);
    Settings::set_value(runtime, "hide_info", false);
    Settings::set_value(runtime, "hide_console", false);
    Settings::set_value(runtime, "hide_console", false);
    Settings::set_value(runtime, "console_ratio", DEFAULT_CONSOLE_RATIO);
    Settings::set_value(runtime, "project_ratio", DEFAULT_FILE_MANAGER_RATIO);
    Settings::set_value(runtime, "info_ratio", DEFAULT_INFO_RATIO);
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

    auto create_tool_menu = [=](Runtime &rt) {
		this->addToolsMenu(menubar);
    };

    runtime.get_global("phon");
    {
        runtime.add_method("add_menu", add_menu, 1);
        runtime.add_method("add_submenu", add_submenu, 2);
        runtime.add_method("clear_menu", clear_menu, 1);
        runtime.add_method("enable_menu", enable_menu, 2);
        runtime.add_method("enable_action", enable_action, 2);
        runtime.add_method("add_action", add_action, 2);
        runtime.add_method("add_separator", add_separator, 0);
        runtime.add_method("set_checkable", set_checkable, 2);
        runtime.add_method("check_action", check_action, 2);
        runtime.add_method("set_action_shortcut", set_action_shortcut, 2);
        runtime.add_method("set_action_tooltip", set_action_tooltip, 2);
        runtime.add_method("bind_action", bind_action, 2);
        runtime.add_method("create_window_menu", create_window_menu, 0);
        runtime.add_method("create_tool_menu", create_tool_menu, 0);
    }
    runtime.pop();

    try
    {
        run_script(runtime, menu);
    }
    catch (std::exception &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Menu creation failed"), e.what());
        dlg.exec();
    }

    setMenuBar(menubar);
}

void MainWindow::setToolsMenu()
{
	tool_separator = tools_menu->addSeparator();

	auto install_action = new QAction(tr("Install plugin..."));
	tools_menu->addAction(install_action);

	auto uninstall_action = new QAction(tr("Uninstall plugin..."));
	tools_menu->addAction(uninstall_action);
	tools_menu->addSeparator();

	auto extend_action = new QAction(tr("How to extend this menu"));
	tools_menu->addAction(extend_action);

	connect(install_action, &QAction::triggered, this, &MainWindow::installPlugin);
	connect(uninstall_action, &QAction::triggered, this, &MainWindow::uninstallPlugin);
	connect(extend_action, &QAction::triggered, [&](bool) {
		runtime.do_string(R"__(
		var page = phon.config.get_documentation_page("scripting/plugins.html")
		phon.show_documentation(page)
		)__");
	});
}

void MainWindow::addToolsMenu(QMenuBar *menubar)
{
	tools_menu = new QMenu(tr("Tools"));
	setToolsMenu();
	menubar->addMenu(tools_menu);
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
    show_info->setShortcut(QKeySequence("ctrl+alt+m"));
    maximize->setShortcut(QKeySequence("ctrl+alt+v"));
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
        auto msg = rt.to_string(1);
        String title = rt.arg_count() > 1 ? rt.to_string(2) : String("Warning");
        QMessageBox dlg(QMessageBox::Warning, title, msg);
        dlg.exec();
        rt.push_null();
    };

    auto alert = [](Runtime &rt) {
        auto msg = rt.to_string(1);
	    String title = rt.arg_count() > 1 ? rt.to_string(2) : String("Error");
	    QMessageBox dlg(QMessageBox::Critical, title, msg);
        dlg.exec();
        rt.push_null();
    };

    auto info = [](Runtime &rt) {
        auto msg = rt.to_string(1);
	    String title = rt.arg_count() > 1 ? rt.to_string(2) : String("Information");
	    QMessageBox dlg(QMessageBox::Information, title, msg);
        dlg.exec();
        rt.push_null();
    };

    auto ask = [this](Runtime &rt) {
    	auto msg = rt.to_string(1);
    	String title = rt.arg_count() > 1 ? rt.to_string(2) : String("Question");
	    auto reply = QMessageBox::question(this, title, msg, QMessageBox::Yes|QMessageBox::No);
	    rt.push_boolean(reply == QMessageBox::Yes);
    };

    auto about = [=](Runtime &rt) {
        auto msg = rt.to_string(1);
        auto title = rt.to_string(2);
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
        auto s = rt.to_string(1);
        auto dir = Settings::get_last_directory(rt);
        QString filter = rt.arg_count() > 1 ? QString(rt.to_string(2)) : QString();
        auto path = QFileDialog::getSaveFileName(this, s, dir, filter, &filter);
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

    auto run_last_query = [=](Runtime &rt) {
    	this->runLastQuery();
    	rt.push_null();
    };

    auto input = [=](Runtime &rt) {
        auto label = rt.to_string(1);
        auto title = rt.to_string(2);
        auto text = rt.to_string(3);
        auto result = QInputDialog::getText(this, title, label, QLineEdit::Normal, text);
        rt.push(result);
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

    auto get_plugin_version = [this](Runtime &rt) {
    	auto name = rt.to_string(1);
        auto plugin = findPlugin(name);
        if (plugin) {
        	rt.push(plugin->version());
        }
        else {
        	rt.push_null();
        }
    };

    auto get_plugin_resource = [this](Runtime &rt) {
    	auto plugin = rt.to_string(1);
    	auto name = rt.to_string(2);
    	auto resource = filesystem::join(Settings::plugin_directory(), plugin, "Resources", name);
		rt.push(std::move(resource));
    };

    auto close_current_view = [this](Runtime &rt) {
    	main_area->viewer()->closeCurrentView();
    };

    auto view_annotation = [this](Runtime &rt) {
	    auto annot = rt.cast_user_data<AutoAnnotation>(1);
	    intptr_t layer = 1;
	    double from = 0.0;
	    double to = 10.0;
	    if (rt.arg_count() >= 2)
	    {
	    	layer = rt.to_integer(2);
	    }
	    if (rt.arg_count() == 4)
	    {
	    	from = rt.to_number(3);
	    	to = rt.to_number(4);
	    }
	    main_area->viewer()->editAnnotation(std::move(annot), layer, from, to);
	    rt.push_null();
    };

	auto import_metadata = [this](Runtime &rt) {
		importMetadata();
		rt.push_null();
	};

	auto export_metadata = [](Runtime &rt) {
		auto path = rt.to_string(1);
		Project::instance()->export_metadata(path);
		rt.push_null();
	};

	auto create_dialog = [this](Runtime &rt) {
		auto s = rt.to_string(1);
		UserDialog dlg(rt, s, this);
		if (dlg.exec() == QDialog::Accepted)
		{
			rt.push(dlg.get());
		}
		else
		{
			rt.push_null();
		}
	};

	auto set_status = [this](Runtime &rt) {
		auto msg = rt.to_string(1);
		auto time = rt.arg_count() == 1 ? 2000 : rt.to_integer(2);
		statusBar()->showMessage(msg, time);
		rt.push_null();
	};

	auto transphon = [=](Runtime &rt) {
		run_script(rt, transphon);
		rt.push_null();
	};

	auto get_plugin_list = [this](Runtime &rt) {
		Array<Variant> names;
		for (auto &plugin : plugins) {
			names.append(plugin->label());
		}
		rt.push(std::move(names));
	};

	auto view_text = [this](Runtime &rt) {
		auto path = rt.to_string(1);
		auto title = rt.to_string(2);
		TextViewer viewer(path, title, this);
		if (rt.arg_count() >= 3)
		{
			int w = rt.to_integer(3);
			viewer.setMinimumWidth(w);
		}
		if (rt.arg_count() == 4)
		{
			int h = rt.to_integer(4);
			viewer.setMinimumHeight(h);
		}
		viewer.exec();
		rt.push_null();
	};

	auto get_current_sound = [this](Runtime &rt) {
		auto viewer = main_area->viewer();
		auto sound = viewer->getCurrentSound();
		if (sound) {
			rt.new_user_data(Sound::meta(), "Sound", sound);
		}
		else {
			rt.push_null();
		}
	};

	auto get_current_annot = [this](Runtime &rt) {
		auto viewer = main_area->viewer();
		auto annot = viewer->getCurrentAnnotation();
		if (annot) {
			rt.new_user_data(Annotation::meta(), "Annotation", annot);
		}
		else {
			rt.push_null();
		}
	};

	runtime.add_global_function("view_text", view_text, 2);
	runtime.add_global_function("warning", warning, 1);
	runtime.add_global_function("alert", alert, 1);
	runtime.add_global_function("info", info, 1);
	runtime.add_global_function("ask", ask, 1);
	runtime.add_global_function("about", about, 2);
	runtime.add_global_function("open_file_dialog", open_file_dialog, 1);
	runtime.add_global_function("open_files_dialog", open_files_dialog, 1);
	runtime.add_global_function("open_directory_dialog", open_directory_dialog, 1);
	runtime.add_global_function("save_file_dialog", save_file_dialog, 1);
	runtime.add_global_function("get_input", input, 3);
	runtime.add_global_function("get_plugin_version", get_plugin_version, 1);
	runtime.add_global_function("get_plugin_resource", get_plugin_resource, 2);
	runtime.add_global_function("set_status", set_status, 1);
	runtime.add_global_function("get_current_sound", get_current_sound, 0);
	runtime.add_global_function("get_current_annotation", get_current_annot, 0);

    runtime.get_global("phon");
    {
        runtime.add_method("show_documentation", show_doc, 1);
        runtime.add_accessor("version", get_version);
        runtime.add_accessor("date", get_date);
        runtime.add_accessor("supported_sound_formats", get_supported_sound_formats);
        runtime.add_accessor("rtaudio_version", get_rtaudio_version);
        runtime.add_accessor("libsndfile_version", get_libsndfile_version);
        runtime.add_method("quit", quit, 0);
        runtime.add_method("edit_settings", edit_settings, 0);
        runtime.add_method("view_script", view_script, 1);
        runtime.add_method("run_script", run_script, 1);
        runtime.add_method("open_query_editor", open_query_editor, 0);
        runtime.add_method("run_last_query", run_last_query, 0);
        runtime.add_method("close_current_view", close_current_view, 0);
        runtime.add_method("view_annotation", view_annotation, 4);
        runtime.add_method("import_metadata", import_metadata, 0);
	    runtime.add_method("export_metadata", export_metadata, 1);
	    runtime.add_method("__create_dialog", create_dialog, 1);
	    runtime.add_method("get_plugin_list", get_plugin_list, 0);
	    runtime.add_method("transphon", transphon, 0);

	    // Define 'phon.config'
        Settings::initialize(runtime);
    }
    runtime.pop();
	runtime.add_global_function("test_global", info, 1);

	// Accept either an object or a string as input, and return an object
	runtime.do_string(R"_(
		function create_dialog(data)
			if typeof(data) == "Object" then
				data = json.stringify(data)
		    end
			data = phon.__create_dialog(data)

			return json.parse(data)
		end
)_");
}

void MainWindow::initialize()
{
    try
    {
        run_script(runtime, initialize);
        run_script(runtime, event);
        Project::create(runtime);
        Project::initialize(runtime);
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
        runtime.do_string("phon = {}");
        Settings::read(runtime);
    }
    catch (std::runtime_error &e)
    {
        QMessageBox dlg(QMessageBox::Critical, tr("Pre-initialization failed"), e.what());
        dlg.exec();
    }
}

void MainWindow::postInitialize()
{
	run_script(runtime, speech_analysis);

	// Load system plugins and scripts, and then the user's plugins and scripts.
	String resources_dir = Settings::get_string(runtime, "resources_directory");
	String user_dir = Settings::settings_directory();
	loadPluginsAndScripts(resources_dir);
	loadPluginsAndScripts(user_dir);

#if PHON_WINDOWS
	QFontDatabase::addApplicationFont(":/fonts/NotoSansMono-Regular.ttf");
	QFontDatabase::addApplicationFont(":/fonts/NotoSansMono-Bold.ttf");
#endif
}

void MainWindow::loadPluginsAndScripts(const String &root)
{
	String plugin_dir = filesystem::join(root, "Plugins");

	if (filesystem::exists(plugin_dir))
	{
		auto files = filesystem::list_directory(plugin_dir);
		std::sort(files.begin(), files.end());

		for (auto &name : files)
		{
			String path = filesystem::join(plugin_dir, name);
			if (filesystem::is_directory(path))
			{
				try
				{
					loadPlugin(path);
				}
				catch (std::exception &e)
				{
					QMessageBox dlg(QMessageBox::Critical, tr("Plugin initialization failed"), e.what());
					dlg.exec();
				}
			}
		}
	}

	String scripts_dir = filesystem::join(root, "Scripts");

	if (filesystem::exists(scripts_dir))
	{
		auto files = filesystem::list_directory(scripts_dir);
		std::sort(files.begin(), files.end());

		for (auto &name : files)
		{
			String path = filesystem::join(scripts_dir, name);

			if (filesystem::is_file(path))
			{
				try
				{
					runtime.do_file(path);
				}
				catch (std::exception &e)
				{
					auto msg = utils::format("Error in script %: %", path, e.what());
					QMessageBox dlg(QMessageBox::Critical, tr("Post-initialization error"), QString::fromStdString(msg));
					dlg.exec();
				}
			}
		}
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
	if (!main_area->viewer()->finalize()) {
		return false;
	}
    auto project = Project::instance();

    if (project->modified())
    {
        auto autosave = Settings::get_boolean(runtime, "autosave");

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

        runtime.do_string("phon.save_project()");
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
	openQueryEditor(nullptr);
}

void MainWindow::openQueryEditor(AutoProtocol protocol)
{
	int context_length = (int) Settings::get_number(runtime, "match_window_length");
	auto editor = new QueryEditor(runtime, std::move(protocol), this, context_length);
	connect(editor, &QueryEditor::queryReady, this, &MainWindow::executeQuery);
	editor->resize(1100, 800);
	editor->show();
	cacheQueryEditor(editor);
}

void MainWindow::runLastQuery()
{
	if (query_editor)
	{
		query_editor->resurrect();
	}
	else
	{
		openQueryEditor();
	}
}

void MainWindow::cacheQueryEditor(QueryEditor *ed)
{
	if (query_editor && query_editor != ed) {
		delete query_editor;
	}

	query_editor = ed;
}

void MainWindow::executeQuery(AutoQuery query)
{
	auto console = main_area->console();
	connect(query.get(), &Query::debug, console, &Console::warn);
	connect(query.get(), &Query::done, console, &Console::setPrompt);
	auto dataset = query->execute();

	if (dataset->empty())
	{
		QMessageBox dlg(QMessageBox::Information, tr("Information"), tr("No match found!"));
		dlg.exec();
	}
	else
	{
		main_area->viewer()->openTableView(std::move(dataset));
	}
}

void MainWindow::setDatabaseConnection()
{
	auto &db = Project::instance()->database();
	connect(&db, &MetaDatabase::saving_metadata, this, &MainWindow::updateStatus);
}

void MainWindow::loadPlugin(const String &path)
{
	String msg("Loading plugin ");
	msg.append(path);
	updateStatus(msg);

	auto menu = new QMenu;

	// Create callback to add a separator, a script or a protocol to the plugin's menu.
	auto script_callback = [=](String name, Plugin::MenuEntry target, String shortcut) {
		if (name.empty())
		{
			menu->addSeparator();
			return;
		}
		auto action = new QAction(name);
		if (!shortcut.empty()) action->setShortcut(QKeySequence(shortcut));
		menu->addAction(action);

		if (target.type() == typeid(String))
		{
			auto script = std::any_cast<String>(target);
			connect(action, &QAction::triggered, [script, this](bool) {
				runtime.do_file(script);
			});
		}
		else
		{
			auto protocol = std::any_cast<AutoProtocol>(target);
			connect(action, &QAction::triggered, [protocol, this](bool) {
				openQueryEditor(protocol);
			});
		}
	};

	try
	{
		auto plugin = std::make_shared<Plugin>(runtime, path, script_callback);

		if (plugin->has_entries())
		{
			String label = plugin->label();
			menu->setTitle(label);
			auto desc = plugin->description();

			if (!desc.empty())
			{
				menu->addSeparator();
				String title("About ");
				title.append(label);
				auto about_action = new QAction(title);
				menu->addAction(about_action);

				connect(about_action, &QAction::triggered, [=](bool) {
					QMessageBox::about(this, title, desc);
				});
			}

			auto action = tools_menu->insertMenu(tool_separator, menu);
			plugin->setAction(action);
		}
		else
		{
			delete menu;
		}
		plugins.append(std::move(plugin));
	}
	catch (...)
	{
		delete menu;
		throw;
	}
}

void MainWindow::installPlugin(bool)
{
	String dir = Settings::get_last_directory(runtime);
	auto path = QFileDialog::getOpenFileName(this, tr("Select plugin..."), dir, "ZIP (*.zip)");
	if (path.isNull()) return;
	String archive = path;

	// Compare list of plugins before and after the installation to find the one that was installed.
	auto plugin_dir = Settings::plugin_directory();
	auto plugins1 = filesystem::list_directory(plugin_dir);

	utils::unzip(archive, plugin_dir);

	auto plugins2 = filesystem::list_directory(plugin_dir);
	std::sort(plugins1.begin(), plugins1.end());
	std::sort(plugins2.begin(), plugins2.end());
	std::vector<String> diff;
	std::set_difference(plugins2.begin(), plugins2.end(), plugins1.begin(), plugins1.end(),
			std::inserter(diff, diff.begin()));

	if (diff.size() == 1)
	{
		String path = filesystem::join(plugin_dir, diff.front());
		loadPlugin(path);
		String label = plugins.last()->label();
		auto msg = utils::format("The \"%\" plugin has been installed!", label);

		QMessageBox dlg(QMessageBox::Information, tr("Success"), QString::fromStdString(msg));
		dlg.exec();
	}
	else
	{
		QMessageBox msg(QMessageBox::Critical, tr("Error"),
				tr("Plugin installation failed.\nIf you tried to reinstall an existing plugin, "
	   "you can safely ignore this message, but you should restart the program."));
		msg.exec();
	}
}

void MainWindow::uninstallPlugin(bool)
{
	if (plugins.empty())
	{
		QMessageBox::information(this, tr("No plugin found"), tr("You don't have any plugin installed!"));
		return;
	}

	QStringList names;
	for (auto &p : plugins) names << p->label();
	bool ok;
	String name = QInputDialog::getItem(this, tr("Uninstall plugin"), tr("Choose plugin to uninstall"), names, 0,
			false, &ok);

	for (int i = 1; i <= plugins.size(); i++)
	{
		auto &p = plugins[i];
		if (p->label() == name)
		{
			tools_menu->removeAction(p->action());
			filesystem::remove(p->path());
			String label = p->label();
			plugins.remove_at(i);
			auto msg = utils::format("The \"%\" plugin has been uninstalled!", label);
			QMessageBox::information(this, tr("Success"), QString::fromStdString(msg));
			return;
		}
	}
}

Plugin *MainWindow::findPlugin(const String &name)
{
	for (auto &plugin : plugins)
	{
		if (plugin->label() == name) {
			return plugin.get();
		}
	}

	return nullptr;
}

void MainWindow::importMetadata()
{
	CsvImportDialog dlg(this, runtime);

	if (dlg.exec() == QDialog::Accepted)
	{
		auto path = dlg.path();
		auto sep = dlg.separator();
		Project::instance()->import_metadata(path, sep);
	}
}

void MainWindow::display()
{
	if (Settings::get_boolean(runtime, "full_screen"))
	{
		showMaximized();
	}
	else
	{
		try
		{
			auto &lst = Settings::get_list(runtime, "geometry");
			auto x = int(lst.at(1).to_number());
			auto y = int(lst.at(2).to_number());
			auto w = int(lst.at(3).to_number());
			auto h = int(lst.at(4).to_number());
			setGeometry(x, y, w, h);
			show();
		}
		catch (...)
		{
			showMaximized();
		}
	}

	if (Settings::get_boolean(runtime, "hide_console"))
		file_manager->console_button->click();
	if (Settings::get_boolean(runtime, "hide_info"))
		file_manager->info_button->click();
	if (Settings::get_boolean(runtime, "hide_project"))
	{
		show_project->setChecked(false);
		showProject(false);
	}

	if (Settings::get_boolean(runtime, "autoload"))
	{
		runtime.do_string(R"__(
	var recent = phon.settings.recent_projects
	if not recent.is_empty() then
		phon.project.open(recent[1])
	end)__");
	}

	main_area->focusConsole();
	postInitialize();

	// FIXME: We need to delay splitter adjustment, otherwise they won't show up in the right place.
	//  See: https://stackoverflow.com/questions/28795329/qsplitter-sizes-indicates-wrong-sizes
	QTimer::singleShot(50, this, SLOT(adjustSplitters()));

	updateStatus("Ready");
}

} // phonometrica

