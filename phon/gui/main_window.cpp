/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/log.h>
#include <wx/aboutdlg.h>
#include <wx/msgdlg.h>
#include <wx/choicdlg.h>
#include <wx/textdlg.h>
#include <phon/gui/macros.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/gui/pref/preferences_editor.hpp>
#include <phon/gui/dialog.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/include/icons.hpp>
#include <phon/utils/zip.hpp>

namespace phonometrica {

// File menu
static const int ID_FILE_NEW_SCRIPT = wxID_NEW;
static const int ID_FILE_OPEN_PROJECT = wxID_OPEN;
static const int ID_FILE_ADD_FILES = wxNewId();
static const int ID_FILE_ADD_FOLDER = wxNewId();
static const int ID_FILE_CLEAR_RECENT = wxNewId();
static const int ID_FILE_OPEN_LAST = wxNewId();
static const int ID_FILE_CLOSE_PROJECT = wxNewId();
static const int ID_FILE_SAVE = wxID_SAVE;
static const int ID_FILE_SAVE_AS = wxID_SAVEAS;
static const int ID_FILE_PREFERENCES = wxID_PREFERENCES;
static const int ID_FILE_IMPORT_METADATA = wxNewId();
static const int ID_FILE_EXPORT_ANNOTATIONS = wxNewId();
static const int ID_FILE_EXPORT_METADATA = wxNewId();
static const int ID_FILE_CLOSE_VIEW = wxID_CLOSE;
static const int ID_FILE_EXIT = wxID_EXIT;

// Analysis menu
static const int ID_ANALYSIS_FIND = wxID_FIND;
static const int ID_ANALYSIS_FORMANTS = wxNewId();

// Tools menu
static const int ID_TOOLS_RUN = wxNewId();
static const int ID_TOOLS_INSTALL = wxNewId();
static const int ID_TOOLS_UNINSTALL = wxNewId();
static const int ID_TOOLS_EXTEND = wxNewId();

// Window menu
static const int ID_WINDOW_MAXIMIZE_VIEWER = wxNewId();
static const int ID_WINDOW_HIDE_PROJECT = wxNewId();
static const int ID_WINDOW_HIDE_CONSOLE = wxNewId();
static const int ID_WINDOW_HIDE_INFO = wxNewId();
static const int ID_WINDOW_DEFAULT_LAYOUT = wxNewId();

// Help menu
static const int ID_HELP_ABOUT = wxID_ABOUT;
static const int ID_HELP_WEBSITE = wxNewId();
static const int ID_HELP_CITE = wxNewId();
static const int ID_HELP_LICENSE = wxNewId();
static const int ID_HELP_DOC = wxID_HELP;
static const int ID_HELP_SCRIPTING = wxNewId();
static const int ID_HELP_ACKNOWLEDGEMENTS = wxNewId();
static const int ID_HELP_SOUND_INFO = wxNewId();

MainWindow::MainWindow(Runtime &rt, const wxString &title) :
		wxFrame(nullptr, wxNewId(), title), runtime(rt)
{
	wxSize size(1200, 800);
	SetSize(FromDIP(size));
	MakeMenus();
	SetupUi();
	RestoreGeometry();
	SetBindings();
	SetAccelerators();
	SetStartView();
}

void MainWindow::MakeMenus()
{
	// FIXME: There seems to be a bug in the way macOS's Window menu is handled by wxWidgets.
	//  In order to get the menu in the right place, to get special menu items in the application menu,
	//  and to be able to populate the Window menu, we first create it where it belongs (before Help),
	//  and we populate the menu at the end. This seems to be the only way to get it to work on my machine
	//  (macOS 10.13.6).

	m_menubar = new wxMenuBar;
	m_menubar->Append(MakeFileMenu(), _("&File"));
	m_menubar->Append(MakeAnalysisMenu(), _("&Analysis"));
	m_menubar->Append(MakeToolsMenu(), _("&Tools"));
#ifdef __WXMAC__
	m_menubar->Append(new wxMenu, _("&Window"));
#else
	m_menubar->Append(MakeWindowMenu(), _("&Window"));
#endif
	m_menubar->Append(MakeHelpMenu(), _("&Help"));
	SetMenuBar(m_menubar);

#ifdef __WXMAC__
	wxMenuBar::MacSetCommonMenuBar(m_menubar);

	auto i = m_menubar->FindMenu("Window");
	auto menu = m_menubar->GetMenu(i);
	menu->AppendSeparator();
	PopulateWindowMenu(menu);
#endif
}

wxMenu *MainWindow::MakeFileMenu()
{
	auto menu = new wxMenu;

	menu->Append(ID_FILE_NEW_SCRIPT, _("New script...\tctrl+n"));
	menu->AppendSeparator();

	menu->Append(ID_FILE_OPEN_PROJECT, _("Open project...\tCtrl+o"));
	menu->Append(ID_FILE_ADD_FILES, _("Add files to project...\tCtrl+Shift+a"),
	             _("Add one or more file(s) to the current project"));
	menu->Append(ID_FILE_ADD_FOLDER, _("Add content of directory to project..."),
	             _("Recursively import the content of a directory"));
	menu->Append(ID_FILE_CLOSE_PROJECT, _("Close current project"), _("Close the current project"));
	menu->AppendSeparator();

	m_recent_submenu = new wxMenu();
	m_recent_item = menu->AppendSubMenu(m_recent_submenu, _("Recent projects"));
	m_last_item = menu->Append(ID_FILE_OPEN_LAST, _("Open most recent project\tCtrl+Shift+o"),
	                           _("Open the last project that was used in the previous session"));
	menu->AppendSeparator();

	m_save_item = menu->Append(ID_FILE_SAVE, _("Save project\tCtrl+Shift+s"));
	m_save_as_item = menu->Append(ID_FILE_SAVE_AS, _("Save project as..."));
	menu->AppendSeparator();

	menu->Append(ID_FILE_PREFERENCES, _("Preferences..."));
	menu->AppendSeparator();

	auto import_menu = new wxMenu();
	import_menu->Append(ID_FILE_IMPORT_METADATA, _("Import metadata from CSV file..."));
	menu->AppendSubMenu(import_menu, _("Import"));
	auto export_menu = new wxMenu();
	export_menu->Append(ID_FILE_EXPORT_ANNOTATIONS, _("Export annotation(s) to plain text..."));
	export_menu->Append(ID_FILE_EXPORT_METADATA, _("Export project metadata to CSV file..."));
	menu->AppendSubMenu(export_menu, _("Export"));
	menu->AppendSeparator();

	menu->Append(ID_FILE_CLOSE_VIEW, _("Close current view\tctrl+w"));
	menu->AppendSeparator();

	menu->Append(ID_FILE_EXIT, _("Quit\tCtrl+q"));

	PopulateRecentProjects();
	EnableSaveFile(false);
	m_file_menu = menu;

	return menu;
}

wxMenu *MainWindow::MakeAnalysisMenu()
{
	auto menu = new wxMenu;
	menu->Append(ID_ANALYSIS_FIND, _("Find in annotations...\tctrl+shift+f"));
	menu->Append(ID_ANALYSIS_FORMANTS, _("Measure formants..."));

	return menu;
}

wxMenu *MainWindow::MakeWindowMenu()
{
	auto menu = new wxMenu;
	PopulateWindowMenu(menu);

	return menu;
}

void MainWindow::PopulateWindowMenu(wxMenu *menu)
{
	m_project_item = menu->AppendCheckItem(ID_WINDOW_HIDE_PROJECT, _("Hide project panel\tctrl+Left"));
	m_info_item = menu->AppendCheckItem(ID_WINDOW_HIDE_INFO, _("Hide information panel\tctrl+Right"));
	m_console_item = menu->AppendCheckItem(ID_WINDOW_HIDE_CONSOLE, _("Hide console\tctrl+Down"));
	menu->AppendSeparator();
	menu->Append(ID_WINDOW_MAXIMIZE_VIEWER, _("Maximize viewer\tctrl+Up"));
	menu->AppendSeparator();
	menu->Append(ID_WINDOW_DEFAULT_LAYOUT, _("Restore default layout\tctrl+shift+Up"));
}

wxMenu *MainWindow::MakeToolsMenu()
{
	auto menu = new wxMenu;

	menu->Append(ID_TOOLS_RUN, _("Run script..."));
	menu->AppendSeparator();
	menu->Append(ID_TOOLS_INSTALL, _("Install plugin..."));
	menu->Append(ID_TOOLS_UNINSTALL, _("Uninstall plugin..."));
	menu->AppendSeparator();
	menu->Append(ID_TOOLS_EXTEND, _("How to extend this menu"));
	m_tools_menu = menu;

	return menu;
}

wxMenu *MainWindow::MakeHelpMenu()
{
	auto menu = new wxMenu;

	menu->Append(ID_HELP_DOC, _("Documentation"));
	menu->Append(ID_HELP_SCRIPTING, _("Scripting"));
	menu->AppendSeparator();
	menu->Append(ID_HELP_WEBSITE, _("Go to website"));
	menu->Append(ID_HELP_ACKNOWLEDGEMENTS, _("Acknowledgements"));
	menu->AppendSeparator();
	menu->Append(ID_HELP_SOUND_INFO, _("Sound information"));
	menu->AppendSeparator();
	menu->Append(ID_HELP_ABOUT, _("About Phonometrica"));

	return menu;
}

void MainWindow::SetBindings()
{
	// File menu
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnNewScript, this, ID_FILE_NEW_SCRIPT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenProject, this, ID_FILE_OPEN_PROJECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenMostRecentProject, this, ID_FILE_OPEN_LAST);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnExit, this, ID_FILE_EXIT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAddFilesToProject, this, ID_FILE_ADD_FILES);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAddDirectoryToProject, this, ID_FILE_ADD_FOLDER);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSaveProject, this, ID_FILE_SAVE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSaveProjectAs, this, ID_FILE_SAVE_AS);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnCloseProject, this, ID_FILE_CLOSE_PROJECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnEditPreferences, this, ID_FILE_PREFERENCES);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnImportMetadata, this, ID_FILE_IMPORT_METADATA);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnExportMetadata, this, ID_FILE_EXPORT_METADATA);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnCloseCurrentView, this, ID_FILE_CLOSE_VIEW);


	// Tools menu
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnRunScript, this, ID_TOOLS_RUN);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnInstallPlugin, this, ID_TOOLS_INSTALL);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnUninstallPlugin, this, ID_TOOLS_UNINSTALL);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnExtendTools, this, ID_TOOLS_EXTEND);


	// Window menu
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnRestoreDefaultLayout, this, ID_WINDOW_DEFAULT_LAYOUT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnHideProject, this, ID_WINDOW_HIDE_PROJECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnHideInfo, this, ID_WINDOW_HIDE_INFO);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnHideConsole, this, ID_WINDOW_HIDE_CONSOLE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnMaximizeViewer, this, ID_WINDOW_MAXIMIZE_VIEWER);

	// Help menu.
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenDocumentation, this, ID_HELP_DOC);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnHelpScripting, this, ID_HELP_SCRIPTING);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnGoToWebsite, this, ID_HELP_WEBSITE);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnCitation, this, ID_HELP_CITE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenLicense, this, ID_HELP_LICENSE);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenAcknowledgements, this, ID_HELP_ACKNOWLEDGEMENTS);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSoundInfo, this, ID_HELP_SOUND_INFO);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAbout, this, ID_HELP_ABOUT);

	// Sashes
	m_project_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnFileManagerSashMoved, this);
	m_viewer_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnViewerSashMoved, this);
	m_info_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnInfoSashMoved, this);

	m_project_manager->view_file.connect(&Viewer::OnViewFile, m_viewer);
	m_project_manager->files_selected.connect(&InfoPanel::OnSetFileSelection, m_info_panel);
	m_project_manager->execute_script.connect(&Console::RunScript, m_console);

	auto project = Project::get();
	project->notify_update.connect(&ProjectManager::OnProjectUpdated, m_project_manager);
	project->metadata_updated.connect(&ProjectManager::UpdateLabel, m_project_manager);

//	connect(Project::instance(), &Project::notify_closed, viewer, &Viewer::closeAll);
//	connect(Project::instance(), &Project::request_save, viewer, &Viewer::saveViews);


	Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnCloseRequest, this);
}

void MainWindow::OnExit(wxCommandEvent &)
{
	Finalize();
}

void MainWindow::OnCloseRequest(wxCloseEvent &e)
{
	if (!Finalize()) {
		e.Veto();
	}
}

bool MainWindow::Finalize()
{
	auto project = Project::get();
	if (!m_viewer->Finalize()) {
		return false;
	}
	if (project->modified())
	{
		if (Settings::get_boolean("autosave"))
		{
			SaveProject();
		}
		else
		{
			auto reply = wxMessageBox(_("The project has been modified. Would you like to save it?"),
			                          _("Save project?"), wxCANCEL|wxYES|wxNO|wxYES_DEFAULT|wxICON_QUESTION);

			if (reply == wxCANCEL) {
				return false;
			}
			if (reply == wxYES)
			{
				SaveProject();
			}
		}
	}
	SaveGeometry();
	Destroy();

	return true;
}

void MainWindow::EnableSaveFile(bool value)
{
	m_save_item->Enable(value);
	m_save_as_item->Enable(value);
}

void MainWindow::EnableRecentProjects(bool value)
{
	m_recent_item->Enable(value);
	m_last_item->Enable(value);
}

void MainWindow::OnNewScript(wxCommandEvent &)
{
	m_viewer->NewScript();
}

void MainWindow::SetupUi()
{
	long sash_flags = wxSP_THIN_SASH|wxSP_LIVE_UPDATE;
	wxIcon icon;
	icon.CopyFromBitmap(wxBITMAP_PNG_FROM_DATA(sound_wave_small));
	SetIcon(icon);
	// Split project manager on the left and the main area.
	m_project_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sash_flags);
	m_project_manager = new ProjectManager(runtime, m_project_splitter);
	m_main_area = new wxPanel(m_project_splitter, -1, wxDefaultPosition, wxDefaultSize);

	// The main area contains the console at the bottom, the info panel on the right, and the viewer in the center.
	m_info_splitter = new wxSplitterWindow(m_main_area, wxID_ANY, wxDefaultPosition, wxDefaultSize, sash_flags);
	m_central_panel = new wxPanel(m_info_splitter);
	m_info_panel = new InfoPanel(runtime, m_info_splitter);

	// Split viewer and info panel.
	m_viewer_splitter = new wxSplitterWindow(m_central_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, sash_flags);
	m_viewer = new Viewer(runtime, m_viewer_splitter, this);
	m_console = new Console(runtime, m_viewer_splitter);

	// Set sizers.
	auto sizer1 = new wxBoxSizer(wxVERTICAL);
	sizer1->Add(m_project_splitter, 1, wxEXPAND, 0);
	this->SetSizer(sizer1);

	auto sizer2 = new wxBoxSizer(wxVERTICAL);
	sizer2->Add(m_info_splitter, 1, wxEXPAND);
	m_main_area->SetSizer(sizer2);

	auto sizer3 = new wxBoxSizer(wxVERTICAL);
	sizer3->Add(m_viewer_splitter, 1, wxEXPAND, 0);
	m_central_panel->SetSizer(sizer3);
}

void MainWindow::PostInitialize()
{
	ShowAllPanels();
	SetShellFunctions();
	
	// Load system plugins and scripts, and then the user's plugins and scripts.
	String resources_dir = Settings::resources_directory();
	String user_dir = Settings::settings_directory();
	LoadPluginsAndScripts(resources_dir);
	LoadPluginsAndScripts(user_dir);
	m_console->SetFocus();

	if (Settings::get_boolean("autoload"))
	{
		OpenMostRecentProject();
	}
}

void MainWindow::ShowAllPanels()
{
	m_project_splitter->SplitVertically(m_project_manager, m_main_area);
	m_info_splitter->SplitVertically(m_central_panel, m_info_panel);
	m_viewer_splitter->SplitHorizontally(m_viewer, m_console);
	UpdateLayout();
}

void MainWindow::OnGoToWebsite(wxCommandEvent &)
{
	wxLaunchDefaultBrowser("http://www.phonometrica-ling.org");
}

void MainWindow::OnOpenLicense(wxCommandEvent &)
{
	OpenDocumentation("about/license");
}

void MainWindow::OnOpenAcknowledgements(wxCommandEvent &)
{
	OpenDocumentation("about/acknowledgements.html");
}

void MainWindow::OnOpenDocumentation(wxCommandEvent &)
{
	OpenDocumentation(String());
}

void MainWindow::OpenDocumentation(String page)
{
	auto url = Settings::get_documentation_page(std::move(page));
	wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);

//	if (!result || !filesystem::exists(path)) {
//		auto msg = utils::format("Could not open file \"%\"", path);
//		wxMessageBox(msg, "Error", wxICON_ERROR);
//	}
}

void MainWindow::SetStartView()
{
	m_viewer->SetStartView();
}

void MainWindow::UpdateLayout()
{
	UpdateProjectLayout();
	UpdateInfoLayout();
	UpdateViewerLayout();
}

void MainWindow::UpdateProjectLayout()
{
	auto ratio = Settings::get_number("project_ratio");

	if (ratio == 0.0)
	{
		if (m_project_splitter->IsSplit()) {
			m_project_splitter->Unsplit(m_project_manager);
		}
		if (!m_project_item->IsChecked()) {
			m_project_item->Check();
		}
	}
	else
	{
		if (!m_project_splitter->IsSplit()) {
			m_project_splitter->SplitVertically(m_project_manager, m_main_area);
		}
		int pos = (int) ((double) GetSize().GetWidth() * ratio);
		m_project_splitter->SetSashGravity(ratio);
		m_project_splitter->SetSashPosition(pos);
	}
}

void MainWindow::UpdateViewerLayout()
{
	auto ratio = Settings::get_number("console_ratio");

	if (ratio == 0.0)
	{
		if (m_viewer_splitter->IsSplit()) {
			m_viewer_splitter->Unsplit(m_console);
		}
		if (!m_console_item->IsChecked()) {
			m_console_item->Check();
		}
	}
	else
	{
		if (!m_viewer_splitter->IsSplit()) {
			m_viewer_splitter->SplitHorizontally(m_viewer, m_console);
		}
		int height = GetMainAreaHeight();
		auto pos = int(height * ratio);
		m_viewer_splitter->SetSashGravity(ratio);
		m_viewer_splitter->SetSashPosition(pos);
	}
}

void MainWindow::UpdateInfoLayout()
{
	auto ratio = Settings::get_number("info_ratio");

	if (ratio == 0.0)
	{
		if (m_info_splitter->IsSplit()) {
			m_info_splitter->Unsplit(m_info_panel);
		}
		if (!m_info_item->IsChecked()) {
			m_info_item->Check();
		}
	}
	else
	{
		if (!m_info_splitter->IsSplit()) {
			m_info_splitter->SplitVertically(m_central_panel, m_info_panel);
		}
		int width = GetMainAreaWidth();
		auto pos = int(width * ratio);
		m_info_splitter->SetSashPosition(pos);
		m_info_splitter->SetSashGravity(ratio);
	}
}

void MainWindow::OnResize(wxSizeEvent &e)
{
	UpdateLayout();
	e.Skip();
}

int MainWindow::GetMainAreaHeight() const
{
	return (m_main_area->GetSize().GetHeight());
}

int MainWindow::GetMainAreaWidth() const
{
	return m_main_area->GetSize().GetWidth();
}

void MainWindow::OnFileManagerSashMoved(wxSplitterEvent &e)
{
	int pos = e.GetSashPosition();
	int width = GetClientSize().GetWidth();
	double ratio = double(pos) / width;
	m_project_splitter->SetSashGravity(ratio);
	Settings::set_value("project_ratio", ratio);
}

void MainWindow::OnInfoSashMoved(wxSplitterEvent &e)
{
	int pos = e.GetSashPosition();
	int width = GetMainAreaWidth();
	double ratio = double(pos) / width;
	m_viewer_splitter->SetSashGravity(ratio);
	Settings::set_value("info_ratio", ratio);
}

void MainWindow::OnViewerSashMoved(wxSplitterEvent &e)
{
	int pos = e.GetSashPosition();
	int height = GetMainAreaHeight();
	double ratio = double(pos) / height;
	m_viewer_splitter->SetSashGravity(ratio);
	Settings::set_value("console_ratio", ratio);
}

void MainWindow::SaveGeometry()
{
	int x, y, w, h;
	GetPosition(&x, &y);
	GetSize(&w, &h);

	Settings::set_value("geometry", { double(x), double(y), double(w), double(h) });
	Settings::set_value("full_screen", this->IsMaximized());
}

void MainWindow::RestoreGeometry()
{
	if (Settings::get_boolean("full_screen"))
	{
		Maximize();
	}
	else
	{
		auto &lst = Settings::get_list("geometry");
		auto x = int(lst.at(1).get_number());
		auto y = int(lst.at(2).get_number());
		auto w = int(lst.at(3).get_number());
		auto h = int(lst.at(4).get_number());
		SetPosition(wxPoint(x, y));
		SetSize(w, h);
	}
}

void MainWindow::OnRestoreDefaultLayout(wxCommandEvent &)
{
	Settings::set_value("project_ratio", DEFAULT_PROJECT_RATIO);
	Settings::set_value("info_ratio", DEFAULT_INFO_RATIO);
	Settings::set_value("console_ratio", DEFAULT_CONSOLE_RATIO);
	UpdateLayout();
	m_console_item->Check(false);
	m_info_item->Check(false);
	m_project_item->Check(false);

	// TODO: update status bar buttons when restoring default layout
}

void MainWindow::OnHideProject(wxCommandEvent &)
{
	Settings::set_value("project_ratio", GetProjectRatio());
	UpdateProjectLayout();
}

void MainWindow::OnHideInfo(wxCommandEvent &)
{
	Settings::set_value("info_ratio", GetInfoRatio());
	UpdateInfoLayout();
}

void MainWindow::OnHideConsole(wxCommandEvent &)
{
	Settings::set_value("console_ratio", GetConsoleRatio());
	UpdateViewerLayout();
}

void MainWindow::OnMaximizeViewer(wxCommandEvent &)
{
	m_console_item->Check(true);
	m_info_item->Check(true);
	m_project_item->Check(true);
	Settings::set_value("console_ratio", 0.0);
	UpdateViewerLayout();
	Settings::set_value("info_ratio", 0.0);
	UpdateInfoLayout();
	Settings::set_value("project_ratio", 0.0);
	UpdateProjectLayout();
}

double MainWindow::GetProjectRatio() const
{
	return m_project_item->IsChecked() ? 0.0 : DEFAULT_PROJECT_RATIO;
}

double MainWindow::GetInfoRatio() const
{
	return m_info_item->IsChecked() ? 0.0 : DEFAULT_INFO_RATIO;
}

double MainWindow::GetConsoleRatio() const
{
	return m_console_item->IsChecked() ? 0.0 : DEFAULT_CONSOLE_RATIO;
}

void MainWindow::OnAbout(wxCommandEvent &)
{
	wxAboutDialogInfo dlg;
	dlg.SetName("Phonometrica");
    wxIcon icon;
    icon.CopyFromBitmap(wxBITMAP_PNG_FROM_DATA(sound_wave_small));
	dlg.SetIcon(icon);
	dlg.SetVersion(utils::get_version());
	dlg.SetDescription(_("A program for speech annotation and analysis"));
	dlg.SetCopyright("(C) 2019-2021");
	dlg.SetWebSite("http://www.phonometrica-ling.org");
	dlg.AddDeveloper("Julien Eychenne");
	dlg.AddDeveloper(wxString::FromUTF8("Léa Courdès-Murphy"));
	dlg.AddArtist(_("Icons by Icons8 <https://icons8.com>"));
	wxAboutBox(dlg, this);
}

void MainWindow::OnAddFilesToProject(wxCommandEvent &)
{
	FileDialog dlg(this, _("Add files to project..."), "", "Phonometrica files (*.*)|*.*",
				  wxFD_OPEN|wxFD_MULTIPLE|wxFD_FILE_MUST_EXIST);

	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	wxArrayString paths;
	dlg.GetPaths(paths);
	auto project = Project::get();
	project->clear_import_flag();

	for (auto &path : paths) {
		project->import_file(path);
	}
	ProjectManager::CheckProjectImport();
	Project::updated();
	EnableSaveFile(true);
}

void MainWindow::OnAddDirectoryToProject(wxCommandEvent &)
{
	DirDialog dlg(this, _("Add content of directory..."), wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST);

	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	auto project = Project::get();
	project->clear_import_flag();
	project->import_directory(dlg.GetPath());
	ProjectManager::CheckProjectImport();
	Project::updated();
}

void MainWindow::OnHelpScripting(wxCommandEvent &)
{
	OpenDocumentation("scripting");
}

void MainWindow::OnCloseCurrentView(wxCommandEvent &)
{
	m_viewer->CloseCurrentView();
}

void MainWindow::OnOpenProject(wxCommandEvent &)
{
	FileDialog dlg(this, _("Open project..."), "", "Phonometrica projects (*.phon-project)|*.phon-project", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	auto path = dlg.GetPath();
	OpenProject(path);
}

void MainWindow::OnEditPreferences(wxCommandEvent &)
{
	PreferencesEditor dlg(this);
#ifdef __WXGTK__
    wxSize size(450, 270);
#else
    wxSize size(450, 250);
#endif
	dlg.SetSize(FromDIP(size));
	if (dlg.ShowModal() == wxID_OK)
	{
		m_viewer->AdjustFontSize();
	}
}

void MainWindow::SetAccelerators()
{
	wxAcceleratorEntry entries[4];
	auto id_run = wxNewId();
	auto id_save = wxNewId();
	entries[0].Set(wxACCEL_CTRL, (int) 'R', id_run);
	entries[1].Set(wxACCEL_CTRL, (int) 'S', id_save);
//	entries[2].Set(wxACCEL_CTRL, (int) 'K', wxID_CLOSE);
//	entries[2].Set(wxACCEL_SHIFT, (int) 'A', ID_ABOUT);
//	entries[3].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_CUT);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
	Bind(wxEVT_MENU, &MainWindow::OnRun, this, id_run);
	Bind(wxEVT_MENU, &MainWindow::OnSave, this, id_save);
}

void MainWindow::OnRun(wxCommandEvent &)
{
	auto view = m_viewer->GetCurrentView();
	view->Run();
}

void MainWindow::OnSave(wxCommandEvent &)
{
	auto view = m_viewer->GetCurrentView();
	view->Save();
}

void MainWindow::LoadPluginsAndScripts(const String &root)
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
					LoadPlugin(path);
				}
				catch (std::exception &e)
				{
					wxMessageBox(e.what(), _("Plugin initialization failed"), wxICON_ERROR);
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
					wxMessageBox(msg, _("Post-initialization error"), wxICON_ERROR);
				}
			}
		}
	}
}

void MainWindow::LoadPlugin(const String &path)
{
	auto menu = new wxMenu;

	// Create callback to add a separator, a script or a protocol to the plugin's menu.
	auto script_callback = [=](String name, Plugin::MenuEntry target) {
		if (name.empty())
		{
			m_tool_separator = menu->AppendSeparator();
			return;
		}
		auto id = wxNewId();
		auto action = new wxMenuItem(menu, id, name);
		menu->Append(action);

		if (target.type() == typeid(String))
		{
			auto script = std::any_cast<String>(target);

			if (script.ends_with(".html")) // Launch help page
			{
				this->Bind(wxEVT_COMMAND_MENU_SELECTED, [script](wxCommandEvent &) {
					String url("file://");
					url.append(script);
					wxLaunchDefaultBrowser(url, wxBROWSER_NOBUSYCURSOR);
				}, id);
			}
			else // Or run a script
			{
				this->Bind(wxEVT_COMMAND_MENU_SELECTED, [script, this](wxCommandEvent &) {
					m_console->RunScript(script);
				}, id);
			}
		}
		else
		{
			auto protocol = std::any_cast<AutoProtocol>(target);
			this->Bind(wxEVT_COMMAND_MENU_SELECTED, [protocol, this](wxCommandEvent &) {
				//openQueryEditor(protocol, Query::Type::CodingProtocol);
				// TODO: display query protocol
			}, id);
		}
	};

	try
	{
		auto plugin = std::make_shared<Plugin>(runtime, path, script_callback);

		if (plugin->has_entries())
		{
			String label = plugin->label();
			menu->SetTitle(label);
			auto desc = plugin->description();

			if (!desc.empty())
			{
				menu->AppendSeparator();
				String title("About ");
				title.append(label);
				auto action_id = wxNewId();
				menu->Append(action_id, title);

				Bind(wxEVT_COMMAND_MENU_SELECTED, [=](wxCommandEvent &) {
					wxMessageBox(desc, title, wxICON_INFORMATION);
				}, action_id);
			}

			auto pos = (size_t) m_plugins.size();
			auto id = wxNewId();
			m_tools_menu->Insert(pos, id, plugin->label(), menu);
			plugin->set_menu_id(id);
			if (pos == 0) {
				m_tool_separator = m_tools_menu->InsertSeparator(1);
			}
		}
		else
		{
			delete menu;
		}
		m_plugins.append(std::move(plugin));
	}
	catch (...)
	{
		delete menu;
		throw;
	}
}

void MainWindow::OnRunScript(wxCommandEvent &)
{
	FileDialog dlg(this, _("Run script..."), "", "Phonometrica scripts (*.phon)|*.phon", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}

	m_console->RunScript(dlg.GetPath());
}

void MainWindow::OnExtendTools(wxCommandEvent &)
{
	OpenDocumentation("scripting/plugins.html");
}

void MainWindow::OnInstallPlugin(wxCommandEvent &)
{
	String dir = Settings::get_last_directory();
	FileDialog dlg(this, _("Select plugin..."), "", "Zip file (*.zip)|*.zip", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	String archive = dlg.GetPath();

	// Temporary installation to retrieve the plugin's name.
	auto temp_dir = filesystem::join(filesystem::temp_directory(), "PHON_TEMP_PLUGIN");
	filesystem::create_directory(temp_dir);
	utils::unzip(archive, temp_dir);
	auto content = filesystem::list_directory(temp_dir);

	// Sanity checks
	if (content.size() != 1) {
		wxMessageBox(_("The plugin must contain a single directory"), _("Invalid plugin"), wxICON_ERROR);
		return;
	}
	auto basename = content[1];
	auto temp_plugin = filesystem::join(temp_dir, basename);
	if (!filesystem::is_directory(temp_plugin)) {
		wxMessageBox(_("The plugin must contain  a directory"), _("Invalid plugin"), wxICON_ERROR);
		return;
	}
	filesystem::remove_directory(temp_dir);

	// Check whether the plugin already exists
	auto plugin_dir = Settings::plugin_directory();
	auto plugin_path = filesystem::join(plugin_dir, basename);

	if (filesystem::exists(plugin_path))
	{
		wxMessageDialog dlg(this, _("I found a previous version of this plugin. Would you like me to replace it with "
							  "the newer version?"), _("Update plugin?"), wxYES|wxNO|wxYES_DEFAULT);
		if (dlg.ShowModal() != wxID_YES) return;
		bool found = false;
		for (intptr_t i = 1; i <= m_plugins.size(); i++)
		{
			auto &p = m_plugins[i];

			if (p->path() == plugin_path)
			{
				UninstallPlugin((int) i, false);
				found = true;
				break;
			}
		}
		if (!found) {
			wxMessageBox(_("The plugin could not be uninstalled. I won't try to install the new plugin.\nThis error should not happen, please report it!"),
				_("Failure"), wxICON_ERROR);
		}
	}

	// Proceed to installation
	utils::unzip(archive, plugin_dir);
	LoadPlugin(plugin_path);
	String label = m_plugins.last()->label();
	auto msg = utils::format("The plugin \"%\" has been installed!", label);
	wxMessageBox(msg, _("Success"), wxICON_INFORMATION);
}

void MainWindow::OnUninstallPlugin(wxCommandEvent &)
{
	if (m_plugins.empty())
	{
		wxMessageBox(_("You don't have any plugin installed!"), _("No plugin found"), wxICON_INFORMATION);
		return;
	}
	wxArrayString names;
	for (auto &p : m_plugins) {
		names.Add(p->label());
	}
	int index = wxGetSingleChoiceIndex(_("Which plugin do you want to uninstall?"), _("Uninstall plugin"), names);
	UninstallPlugin(index + 1, true); // to base 1
}

void MainWindow::UninstallPlugin(int index, bool verbose)
{
	auto p = m_plugins[index];
	filesystem::remove(p->path());
	String label = p->label();
	m_plugins.remove_at(index);
	auto item = m_tools_menu->Remove(p->menu_id());
	delete item;

	if (m_plugins.empty())
	{
		m_tools_menu->Remove(m_tool_separator); // Remove separator
		m_tool_separator = nullptr;
	}
	if (verbose) {
		auto msg = utils::format("The plugin \"%\" has been uninstalled!", label);
		wxMessageBox(_(msg), _("Success"), wxICON_INFORMATION);
	}
}

void MainWindow::UpdateRecentProjects(const String &most_recent)
{
	try
	{
		auto &lst = Settings::get_list("recent_projects");
		lst.remove(most_recent);
		lst.prepend(most_recent);
		PopulateRecentProjects();
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid recent project"), wxICON_ERROR);
	}
}

void MainWindow::PopulateRecentProjects()
{
	try
	{
		auto &lst = Settings::get_list("recent_projects");

		while (m_recent_submenu->GetMenuItemCount() != 0)
		{
			auto item = m_recent_submenu->FindItemByPosition(0);
			m_recent_submenu->Destroy(item);
		}

		for (auto &item : lst)
		{
			auto &path = cast<String>(item);
			auto id = wxNewId();
			m_recent_submenu->Append(id, path);
			Bind(wxEVT_COMMAND_MENU_SELECTED, [this,path](wxCommandEvent &) { OpenProject(path); }, id);
		}
		m_recent_submenu->AppendSeparator();
		m_recent_submenu->Append(ID_FILE_CLEAR_RECENT, _("Clear recent projects"));
		Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnClearRecentProjects, this, ID_FILE_CLEAR_RECENT);
		EnableRecentProjects(!lst.empty());
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid recent project"), wxICON_ERROR);
	}
}

void MainWindow::OnClearRecentProjects(wxCommandEvent &)
{
	try
	{
		Array<Variant> projects;
		Settings::set_value("recent_projects", projects);
		PopulateRecentProjects();
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid recent project"), wxICON_ERROR);
	}
}

void MainWindow::OnOpenMostRecentProject(wxCommandEvent &)
{
	OpenMostRecentProject();
}

void MainWindow::OpenProject(const String &path)
{
	Project::get()->open(path);
	UpdateRecentProjects(path);
	EnableSaveFile(true);
}

void MainWindow::OpenMostRecentProject()
{
	try
	{
		auto &lst = Settings::get_list("recent_projects");
		if (lst.empty()) return;
		auto path = cast<String>(lst.first());
		OpenProject(path);
	}
	catch (std::exception &e)
	{
		wxMessageBox(e.what(), _("Invalid recent project"), wxICON_ERROR);
	}
}

void MainWindow::OnCloseProject(wxCommandEvent &)
{
	Project::close();
	EnableSaveFile(false);
}

void MainWindow::OnSaveProject(wxCommandEvent &)
{
	SaveProject();
}

void MainWindow::OnSaveProjectAs(wxCommandEvent &)
{
	SaveProjectAs();
}

void MainWindow::OnImportMetadata(wxCommandEvent &)
{
	m_info_panel->ImportMetadata();
}

void MainWindow::OnExportMetadata(wxCommandEvent &)
{
	m_info_panel->ExportMetadata();
}

void MainWindow::SetShellFunctions()
{
	auto warning2 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		wxMessageBox(msg, title, wxICON_WARNING);
		return Variant();
	};

	auto warning1 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		wxMessageBox(msg, _("Warning"), wxICON_WARNING);
		return Variant();
	};

	auto alert2 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		wxMessageBox(msg, title, wxICON_ERROR);
		return Variant();
	};

	auto alert1 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		wxMessageBox(msg, _("Error"), wxICON_ERROR);
		return Variant();
	};

	auto info2 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		wxMessageBox(msg, title, wxICON_INFORMATION);
		return Variant();
	};

	auto info1 = [](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		wxMessageBox(msg, _("Information"), wxICON_INFORMATION);
		return Variant();
	};

	auto ask1 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		wxMessageDialog dlg(this, msg, _("Question"), wxNO_DEFAULT|wxYES_NO|wxICON_QUESTION);
		return (dlg.ShowModal() == wxID_YES);
	};

	auto ask2 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		wxMessageDialog dlg(this, msg, title, wxNO_DEFAULT|wxYES_NO|wxICON_QUESTION);
		return (dlg.ShowModal() == wxID_YES);
	};

	auto open_file_dialog = [=](Runtime &, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		FileDialog dlg(this, msg);
		if (dlg.ShowModal() != wxID_OK) {
			return Variant();
		}
		return String(dlg.GetPath());
	};

	auto open_files_dialog = [=](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		FileDialog dlg(this, msg, wxEmptyString, wxString::FromAscii(wxFileSelectorDefaultWildcardStr),
				   wxFD_DEFAULT_STYLE|wxFD_MULTIPLE);
		if (dlg.ShowModal() != wxID_OK) {
			return Variant();
		}
		Array<Variant> result;
		wxArrayString paths;
		dlg.GetPaths(paths);

		for (auto &path : paths) {
			result.append(String(path));
		}
		std::sort(result.begin(), result.end());
		return make_handle<List>(&rt, std::move(result));
	};

	auto open_directory_dialog = [=](Runtime &, std::span<Variant> args) -> Variant {
		auto &s = cast<String>(args[0]);
		DirDialog dlg(this, s, wxDD_DEFAULT_STYLE);

		if (dlg.ShowModal() != wxID_OK) {
			return Variant();
		}
		return String(dlg.GetPath());
	};

	auto save_file_dialog = [=](Runtime &, std::span<Variant> args) -> Variant {
		auto &s = cast<String>(args[0]);
		FileDialog dlg(this, s, wxEmptyString, wxString::FromAscii(wxFileSelectorDefaultWildcardStr), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		if (dlg.ShowModal() != wxID_OK) {
			return Variant();
		}
		return String(dlg.GetPath());
	};

	auto input = [=](Runtime &, std::span<Variant> args) -> Variant {
		auto &label = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		auto &value = cast<String>(args[2]);
		wxTextEntryDialog dlg(this, label, title, value);
		if (dlg.ShowModal() != wxID_OK) {
			return Variant();
		}
		return String(dlg.GetValue());
	};

	auto get_version = [](Runtime &, std::span<Variant> args) -> Variant {
		return String(utils::get_version());
	};

	auto get_date = [](Runtime &, std::span<Variant> args) -> Variant {
		return String(utils::get_date());
	};

	auto get_supported_sound_formats = [](Runtime &rt, std::span<Variant> args) -> Variant {
		Array<Variant> sounds;
		for (auto &s : Sound::supported_sound_format_names()) {
			sounds.append(s);
		}
		return make_handle<List>(&rt, std::move(sounds));
	};

	auto get_plugin_version = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &name = cast<String>(args[0]);
		auto plugin = FindPlugin(name);
		if (plugin) return plugin->version();
		return Variant();
	};

	auto get_plugin_resource = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &plugin = cast<String>(args[0]);
		auto &name = cast<String>(args[1]);
		auto resource = filesystem::join(Settings::plugin_directory(), plugin, "Resources", name);
		return std::move(resource);
	};

	auto close_current_view = [this](Runtime &, std::span<Variant> args) -> Variant {
		m_viewer->CloseCurrentView();
		return Variant();
	};

#if 0
	auto view_annotation1 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &annot = cast<AutoAnnotation>(args[0]);
		m_viewer->editAnnotation(std::move(annot), 1, 0.0, 10.0);
		return Variant();
	};

	auto view_annotation2 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &annot = cast<AutoAnnotation>(args[0]);
		intptr_t layer = cast<intptr_t>(args[1]);
		double from = args[2].resolve().get_number();
		double to = args[3].resolve().get_number();
		m_viewer->editAnnotation(std::move(annot), layer, from, to);
		return Variant();
	};

	auto import_metadata = [this](Runtime &, std::span<Variant> args) -> Variant {
		m_info_panel->ImportMetadata();
		return Variant();
	};

	auto export_metadata = [this](Runtime &, std::span<Variant> args) -> Variant {
		m_info_panel->ExportMetadata();
		return Variant();
	};

	// TODO: avoid using third-party json library to create dialogs
	auto create_dialog1 = [this](Runtime &rt, std::span<Variant> args) -> Variant {
		auto &s = cast<String>(args[0]);
		UserDialog dlg(rt, s, this);
		if (dlg.exec() == QDialog::Accepted) {
			return rt.do_string(dlg.get());
		}
		return Variant();
	};

	auto create_dialog2 = [this](Runtime &rt, std::span<Variant> args) -> Variant {
		auto s = args[0].to_string();
		UserDialog dlg(rt, s, this);
		if (dlg.exec() == QDialog::Accepted) {
			return rt.do_string(dlg.get());
		}
		return Variant();
	};

	auto set_status1 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		statusBar()->showMessage(msg, 2000);
		return Variant();
	};

	auto set_status2 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &msg = cast<String>(args[0]);
		auto time = cast<intptr_t>(args[1]);
		statusBar()->showMessage(msg, time);
		return Variant();
	};

	auto transphon = [=](Runtime &rt, std::span<Variant> args) -> Variant {
		run_script(rt, transphon);
		return Variant();
	};

	auto get_plugin_list = [this](Runtime &rt, std::span<Variant> args) -> Variant {
		Array<Variant> names;
		for (auto &plugin : plugins) {
			names.append(plugin->label());
		}
		return make_handle<List>(&rt, std::move(names));
	};

	auto view_text1 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &path = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		TextViewer viewer(path, title, this);
		viewer.exec();
		return Variant();
	};

	auto view_text2 = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto &path = cast<String>(args[0]);
		auto &title = cast<String>(args[1]);
		auto w = cast<intptr_t>(args[2]);
		auto h = cast<intptr_t>(args[3]);
		TextViewer viewer(path, title, this);
		viewer.setMinimumWidth(w);
		viewer.setMinimumHeight(h);
		viewer.exec();
		return Variant();
	};

	auto get_current_sound = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto viewer = m_viewer;
		auto sound = viewer->getCurrentSound();
		if (sound) {
			return make_handle<AutoSound>(std::move(sound));
		}
		return Variant();
	};

	auto get_current_annot = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto viewer = m_viewer;
		auto annot = viewer->getCurrentAnnotation();
		if (annot) {
			return make_handle<AutoAnnotation>(std::move(annot));
		}
		return Variant();
	};

	auto get_window_duration = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto viewer = m_viewer;
		return viewer->getWindowDuration();
	};

	auto get_selection_duration = [this](Runtime &, std::span<Variant> args) -> Variant {
		auto viewer = m_viewer;
		return viewer->getSelectionDuration();
	};
#endif

#define CLS(T) get_class<T>()
//	runtime.add_global("view_text", view_text1, { CLS(String), CLS(String) });
//	runtime.add_global("view_text", view_text2, { CLS(String), CLS(String), CLS(intptr_t), CLS(intptr_t) });
	runtime.add_global("warning", warning1, { CLS(String) });
	runtime.add_global("warning", warning2, { CLS(String), CLS(String) });
	runtime.add_global("alert", alert1, { CLS(String) });
	runtime.add_global("alert", alert2, { CLS(String), CLS(String) });
	runtime.add_global("info", info1, { CLS(String) });
	runtime.add_global("info", info2, { CLS(String), CLS(String) });
	runtime.add_global("ask", ask1, { CLS(String) });
	runtime.add_global("ask", ask2, { CLS(String), CLS(String) });
	runtime.add_global("open_file_dialog", open_file_dialog, { CLS(String) });
	runtime.add_global("open_files_dialog", open_files_dialog, { CLS(String) });
	runtime.add_global("open_directory_dialog", open_directory_dialog, { CLS(String) });
	runtime.add_global("save_file_dialog", save_file_dialog, { CLS(String) });
	runtime.add_global("get_input", input, { CLS(String), CLS(String), CLS(String) });
	runtime.add_global("get_plugin_version", get_plugin_version, { CLS(String) });
	runtime.add_global("get_plugin_resource", get_plugin_resource, { CLS(String), CLS(String) });
//	runtime.add_global("set_status", set_status1, { CLS(String) });
//	runtime.add_global("set_status", set_status2, { CLS(String), CLS(intptr_t) });
//	runtime.add_global("get_current_sound", get_current_sound, { });
//	runtime.add_global("get_current_annotation", get_current_annot, { });
//	runtime.add_global("get_window_duration", get_window_duration,  { });
//	runtime.add_global("get_selection_duration", get_selection_duration, { });

	auto rt = &runtime;
	auto &phon = cast<Module>(runtime["phon"]);
	phon.define(rt, "get_version", get_version, { });
	phon.define(rt, "get_date", get_date, { });
	phon.define(rt, "get_supported_sound_formats", get_supported_sound_formats, { });
//	phon.define(rt, "view_script", view_script, { });
//	phon.define(rt, "view_script", view_script2, { CLS(String) });
//	phon.define(rt, "run_script", run_script, { CLS(String) });
	phon.define(rt, "close_current_view", close_current_view, { });
//	phon.define(rt, "view_annotation", view_annotation1, { CLS(AutoAnnotation) });
//	phon.define(rt, "view_annotation", view_annotation2, { CLS(AutoAnnotation), CLS(intptr_t), CLS(double), CLS(double) });
//	phon.define(rt, "import_metadata", import_metadata, { });
//	phon.define(rt, "export_metadata", export_metadata, { });
//	phon.define(rt, "create_dialog", create_dialog1, { CLS(String) });
//	phon.define(rt, "create_dialog", create_dialog2, { CLS(Table) });
//	phon.define(rt, "get_plugin_list", get_plugin_list, { });
//	phon.define(rt, "transphon", transphon, { });
#undef CLS
}

Plugin *MainWindow::FindPlugin(const String &name)
{
	for (auto &plugin : m_plugins)
	{
		if (plugin->label() == name) {
			return plugin.get();
		}
	}

	return nullptr;
}

void MainWindow::OnSoundInfo(wxCommandEvent &)
{
	auto msg = _("Supported sound formats on this platform:\n");
	msg.Append(String::join(Sound::supported_sound_format_names(), ", "));
	msg.Append("\n\n");
	msg.Append(_("libsndfile: version "));
	msg.Append(Sound::libsndfile_version());
	msg.Append("\n\n");
	msg.Append(_("RTAudio: version "));
	msg.Append(Sound::rtaudio_version());

	wxMessageBox(msg, _("Sound information"), wxICON_INFORMATION);
}

void MainWindow::SaveProject()
{
	auto project = Project::get();
	if (!project->has_path())
	{
		SaveProjectAs();
	}
	else
	{
		project->save();
	}
}

void MainWindow::SaveProjectAs()
{
	FileDialog dlg(this, _("Save project as.."), "", "Phonometrica project (*.phon-projet)|*.phon-project",
	               wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	String path = dlg.GetPath();
	Project::get()->save(path);
	UpdateRecentProjects(path);
}


} // namespace phonometrica