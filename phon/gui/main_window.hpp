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
 * purpose: Main window.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MAIN_WINDOW_HPP
#define PHONOMETRICA_MAIN_WINDOW_HPP

#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/splitter.h>
#include <wx/accel.h>
#include <wx/progdlg.h>
#include <phon/runtime.hpp>
#include <phon/gui/project_manager.hpp>
#include <phon/gui/viewer.hpp>
#include <phon/gui/console.hpp>
#include <phon/gui/info_panel.hpp>
#include <phon/gui/conc/text_query_editor.hpp>
#include <phon/application/plugin.hpp>

namespace phonometrica {

class MainWindow final : public wxFrame
{
public:

	MainWindow(Runtime &rt, const wxString &title);

	void OnOpenDocumentation(wxCommandEvent &);

	void OnAddFilesToProject(wxCommandEvent &);

	void OnAddDirectoryToProject(wxCommandEvent &);

	void OnOpenProject(wxCommandEvent &);

	void OnEditPreferences(wxCommandEvent &);

	void PostInitialize();

	void OnResize(wxSizeEvent &e);

private:

	void MakeMenus();

	wxMenu *MakeFileMenu();

	wxMenu *MakeEditMenu();

	wxMenu *MakeAnalysisMenu();

	wxMenu *MakeWindowMenu();

	wxMenu *MakeToolsMenu();

	wxMenu *MakeHelpMenu();

	void PopulateWindowMenu(wxMenu *menu);

	void SetBindings();

	void OnExit(wxCommandEvent &);

	void OnCloseRequest(wxCloseEvent &);

	void OnNewScript(wxCommandEvent &);

	bool Finalize();

	void SetupUi();

	void EnableRecentProjects(bool value);

	void ShowAllPanels();

	void SetStartView();

	void UpdateLayout();

	void UpdateProjectLayout();

	void UpdateViewerLayout();

	void UpdateInfoLayout();

	void OnGoToWebsite(wxCommandEvent &);

	void OnOpenLicense(wxCommandEvent &);

	void OnOpenAcknowledgements(wxCommandEvent &);

	void OnSoundInfo(wxCommandEvent &);

	void OnAbout(wxCommandEvent &);

	void OpenDocumentation(String page);

	int GetMainAreaHeight() const;

	int GetMainAreaWidth() const;

	void OnFileManagerSashMoved(wxSplitterEvent &);

	void OnInfoSashMoved(wxSplitterEvent &);

	void OnViewerSashMoved(wxSplitterEvent &);

	void SaveGeometry();

	void RestoreGeometry();

	void OnRestoreDefaultLayout(wxCommandEvent &);

	void OnHideProject(wxCommandEvent &);

	void OnHideInfo(wxCommandEvent &);

	void OnHideConsole(wxCommandEvent &);

	void OnMaximizeViewer(wxCommandEvent &);

	void OnHelpScripting(wxCommandEvent &);

	double GetProjectRatio() const;

	double GetInfoRatio() const;

	double GetConsoleRatio() const;

	void OnCloseCurrentView(wxCommandEvent &);

	void OnCloseAllViews(wxCommandEvent &);

	void SetAccelerators();

	void OnExecute(wxCommandEvent &);

	void OnFind(wxCommandEvent &);

	void OnReplace(wxCommandEvent &);

	void OnSave(wxCommandEvent &);

	void OnEscape(wxCommandEvent &);

	void LoadPluginsAndScripts(const String &root);

	void LoadPlugin(const String &path);

	void OnRunScript(wxCommandEvent &);

	void OnExtendTools(wxCommandEvent &);

	void OnInstallPlugin(wxCommandEvent &);

	void OnUninstallPlugin(wxCommandEvent &);

	void UninstallPlugin(int index, bool verbose);

	void UpdateRecentProjects(const String &most_recent);

	void PopulateRecentProjects();

	void OnClearRecentProjects(wxCommandEvent &);

	void OnOpenMostRecentProject(wxCommandEvent &);

	void OpenMostRecentProject();

	void OpenProject(const String &path);

	void OnCloseProject(wxCommandEvent &);

	void OnSaveProject(wxCommandEvent &);

	void OnSaveProjectAs(wxCommandEvent &);

	void OnImportMetadata(wxCommandEvent &);

	void OnExportMetadata(wxCommandEvent &);

	void OnExportAnnotations(wxCommandEvent &);

	void OnFindInAnnotations(wxCommandEvent &);

	void OnMeasureFormants(wxCommandEvent &);

	void OnEditLastQuery(wxCommandEvent &);

	void EditQuery(const Handle<Query> &q);

	void RunQuery(QueryEditor &editor);

	void OnRequestConsole();

	void OnRequestProgress(const String &msg, const String &title, int count);

	void OnUpdateProgress(int i);

	void SetShellFunctions();

	void SaveProject();

	void SaveProjectAs();

	void OnUndo(wxCommandEvent &);

	void OnRedo(wxCommandEvent &);

	void OnWakeUp();

	void OnError(const String &msg);

	Plugin *FindPlugin(const String &name);

	// Project browser, on the left.
	ProjectManager *project_manager;

	// Main area, which contains a set of views displayed as tabs.
	Viewer *viewer;

	// Lua command line, at the bottom.
	Console *console;

	// Panel displaying metadata, on the right.
	InfoPanel *info_panel;

	// Panel holding the command line, the viewer and the info panel.
	wxPanel *main_area;

	// Panel holding the viewer and the info panel.
	wxPanel *central_panel;

	// Split the project manager and the remainder of the main window.
	wxSplitterWindow *project_splitter;

	// Split command line and viewer/info panel.
	wxSplitterWindow *info_splitter;

	// Split viewer and info panel.
	wxSplitterWindow *viewer_splitter;


	wxMenuBar *menubar = nullptr;
	wxMenu *file_menu = nullptr, *recent_submenu = nullptr, *tools_menu = nullptr, *edit_menu = nullptr;

	wxMenuItem *recent_item = nullptr, *last_item = nullptr, *undo_item = nullptr, *redo_item = nullptr;
	wxMenuItem *project_item = nullptr, *info_item = nullptr, *console_item = nullptr;
	wxMenuItem *tool_separator = nullptr;


	std::unique_ptr<wxProgressDialog> progress_dialog;

	wxSize editor_size;

	Array<AutoPlugin> plugins;

	Handle<Query> last_query;

	Runtime &runtime;
};

} // namespace phonometrica

#endif // PHONOMETRICA_MAIN_WINDOW_HPP
