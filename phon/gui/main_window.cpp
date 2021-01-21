/***********************************************************************************************************************
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
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/choicdlg.h>
#include <phon/gui/macros.hpp>
#include <phon/gui/main_window.hpp>
#include <phon/gui/pref/preferences_editor.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/utils/zip.hpp>

namespace phonometrica {

// File menu
static const int ID_FILE_NEW_SCRIPT = wxNewId();
static const int ID_FILE_OPEN_PROJECT = wxNewId();
static const int ID_FILE_ADD_FILES = wxNewId();
static const int ID_FILE_ADD_FOLDER = wxNewId();
static const int ID_FILE_RECENT_SUBMENU = wxNewId();
static const int ID_FILE_CLEAR_RECENT = wxNewId();
static const int ID_FILE_OPEN_LAST = wxNewId();
static const int ID_FILE_CLOSE_PROJECT = wxNewId();
static const int ID_FILE_SAVE = wxNewId();
static const int ID_FILE_SAVE_AS = wxNewId();
static const int ID_FILE_PREFERENCES = wxID_PREFERENCES;
static const int ID_FILE_IMPORT_METADATA = wxNewId();
static const int ID_FILE_EXPORT_ANNOTATIONS = wxNewId();
static const int ID_FILE_EXPORT_METADATA = wxNewId();
static const int ID_FILE_CLOSE_VIEW = wxNewId();
static const int ID_FILE_EXIT = wxID_EXIT;

// Analysis menu
static const int ID_ANALYSIS_FIND = wxNewId();
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
	SetMinSize(size);
	SetSize(size);
	MakeMenus();
	SetupUi();
//	CreateStatusBar(1);
	RestoreGeometry();
	SetBindings();
	SetAccelerators();
	SetStartView();
//	SetStatusText(_("Empty project"));
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
	menu->Append(ID_FILE_ADD_FOLDER, _("Add content of folder to project..."),
	             _("Recursively import the content of a folder"));
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

	menu->Append(ID_FILE_CLOSE_VIEW, _("Close current view\tCtrl+w"));
	menu->AppendSeparator();

	menu->Append(ID_FILE_EXIT, _("Quit\tCtrl+q"));

	EnableRecentProjects(false);
	EnableSaveFile(false);

//	SetRecentProjects();

	return menu;
}

wxMenu *MainWindow::MakeAnalysisMenu()
{
	auto menu = new wxMenu;
	menu->Append(ID_ANALYSIS_FIND, _("Find in annotations...\tctrl+shift+f"));
	menu->Append(ID_ANALYSIS_FORMANTS, _("Measure formants"));

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
	menu->Append(ID_HELP_ABOUT, _("About Phonometrica"));

	return menu;
}

void MainWindow::SetBindings()
{
	// File menu
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnNewScript, this, ID_FILE_NEW_SCRIPT);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenProject, this, ID_FILE_OPEN_PROJECT);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpenLastProject, this, ID_FILE_OPEN_LAST);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnExit, this, ID_FILE_EXIT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAddFilesToProject, this, ID_FILE_ADD_FILES);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAddFolderToProject, this, ID_FILE_ADD_FOLDER);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSaveProject, this, ID_FILE_SAVE);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSaveProjectAs, this, ID_FILE_SAVE_AS);
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnCloseProject, this, ID_FILE_CLOSE_PROJECT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnEditPreferences, this, ID_FILE_PREFERENCES);
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
//	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnSoundInfo, this, ID_HELP_SOUND_INFO);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAbout, this, ID_HELP_ABOUT);

	// Sashes
	m_project_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnFileManagerSashMoved, this);
	m_viewer_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnViewerSashMoved, this);
	m_info_splitter->Bind(wxEVT_SPLITTER_SASH_POS_CHANGED, &MainWindow::OnInfoSashMoved, this);

	Bind(wxEVT_CLOSE_WINDOW, &MainWindow::OnCloseRequest, this);
}

void MainWindow::OnExit(wxCommandEvent &)
{
	Finalize();
}

void MainWindow::OnCloseRequest(wxCloseEvent &)
{
	Finalize();
}

void MainWindow::Finalize()
{
	SaveGeometry();
	Destroy();
}

void MainWindow::EnableSaveFile(bool value)
{
	m_save_item->Enable(value);
	m_save_as_item->Enable(value);
}

void MainWindow::EnableRecentProjects(bool value)
{
	m_recent_item->Enable(value);
}

void MainWindow::OnNewScript(wxCommandEvent &)
{
	m_viewer->NewScript();
}

void MainWindow::SetupUi()
{
	long sash_flags = wxSP_THIN_SASH|wxSP_LIVE_UPDATE;
	SetIcon(wxIcon(Settings::get_icon_path("sound_wave_small.png"), wxBITMAP_TYPE_PNG));
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

	// Load system plugins and scripts, and then the user's plugins and scripts.
	String resources_dir = Settings::get_string("resources_directory");
	String user_dir = Settings::settings_directory();
	LoadPluginsAndScripts(resources_dir);
	LoadPluginsAndScripts(user_dir);
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
	String url("file://");
	filesystem::nativize(page);
	auto path = filesystem::join(Settings::get_string("resources_directory"), "html", page);
	if (!path.ends_with(".html")) {
		filesystem::append(path, "index.html");
	}
	// TODO: set proper path using settings for documentation
	url.append(path);
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
	dlg.SetIcon(wxIcon(Settings::get_icon_path("sound_wave_small.png"), wxBITMAP_TYPE_PNG));
	dlg.SetVersion(utils::get_version());
	dlg.SetDescription(_("A program for speech annotation and analysis"));
	dlg.SetCopyright("(C) 2019-2021");
	dlg.SetWebSite("http://www.phonometrica-ling.org");
	dlg.AddDeveloper("Julien Eychenne");
	dlg.AddDeveloper(wxString::FromUTF8("Léa Courdès-Murphy"));
	wxAboutBox(dlg, this);
}

void MainWindow::OnAddFilesToProject(wxCommandEvent &)
{
	wxFileDialog dlg(this, _("Add files to project..."), "", "", "Phonometrica scripts (*.phon)|*.phon", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (dlg.ShowModal() == wxID_CANCEL) {
		return;
	}
	m_viewer->NewScript(dlg.GetPath());
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

}

void MainWindow::OnEditPreferences(wxCommandEvent &)
{
	PreferencesEditor dlg(this);
	dlg.ShowModal();
}

void MainWindow::SetAccelerators()
{
	wxAcceleratorEntry entries[4];
	auto id_run = wxNewId();
	auto id_save = wxNewId();
	entries[0].Set(wxACCEL_CTRL, (int) 'R', id_run);
	entries[1].Set(wxACCEL_CTRL, (int) 'S', id_save);
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
	wxFileDialog dlg(this, _("Run script..."), "", "", "Phonometrica scripts (*.phon)|*.phon", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
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
	wxFileDialog dlg(this, _("Select plugin..."), "", "", "Zip file (*.zip)|*.zip", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
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

} // namespace phonometrica