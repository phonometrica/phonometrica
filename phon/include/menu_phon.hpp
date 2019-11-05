#ifndef MENU_PHON_SCRIPT_INCLUDE
#define MENU_PHON_SCRIPT_INCLUDE

static const char *menu_script = R"_(

function view_script()
	phon.view_script(null)
end

function run_script()
	var path = open_file_dialog("Open Phonometrica script", "Script (*.phon)")
	if path then
		phon.run_script(path)
	end
end

function enable_most_recent()
	var value = not phon.settings.recent_projects.is_empty()
	phon.enable_action(phon.actions.most_recent, value)
end

function update_recent_projects(path)
	phon.settings.recent_projects.remove(path)
	phon.settings.recent_projects.prepend(path)
	set_recent_projects(null)
end

function clear_recent()
	phon.settings.recent_projects.clear()
	set_recent_projects(null)	
	phon.enable_action(phon.actions.most_recent, false)
end

function set_recent_projects(submenu)
	var recent = phon.settings.recent_projects

	if submenu then
		phon.recent_submenu = submenu 
		
		for var i = recent.length downto 1 do 
			if not system.exists(recent[i]) then
				recent.remove_at(i)
			end
		end
	else
		submenu = phon.recent_submenu
		phon.clear_menu(submenu)
	end

	var enable = not recent.is_empty()
	phon.enable_menu(submenu, enable)

	function generate_callback(path)
		return (function() 
			phon.project.open(path)
			update_recent_projects(path)
		end)
	end

	foreach var path in recent do
		var base = system.get_base_name(path)
		var action = phon.add_action(submenu, base)
		phon.set_action_tooltip(action, path)
		phon.bind_action(action, generate_callback(path))
	end

	phon.add_separator(submenu)
	action = phon.add_action(submenu, "Clear recent projects")
	phon.bind_action(action, clear_recent)

	if not submenu then
		enable_most_recent()
	end
end

function import_metadata()
	if (phon.project.is_empty()) then
		alert("Cannot add metadata to an empty project")
		return
	end

	phon.import_metadata()
end

function export_metadata()
	if (phon.project.is_empty()) then
		alert("Cannot export metadata: project is empty")
		return
	end
	var path = save_file_dialog("Export metadata to CSV file...", "CSV (*.csv)")
	if path then
		phon.export_metadata(path)
	end
end

phon.open_project = function()
	var path = open_file_dialog("Open Phonometrica project", "Project (*.phon-project)")
	if path then
		phon.project.open(path)
		update_recent_projects(path)
		enable_most_recent()
	end
end

function open_most_recent_project()
	var recent = phon.settings.recent_projects
	if recent.is_empty() then
		return
	end

	var path = recent[1]
	phon.project.open(path)
end

phon.add_files = function()
	var paths = open_files_dialog("Add file(s) to project...", "Phonometrica files (*.*)")
	foreach var path in paths do
		phon.project.add_file(path)
	end

	phon.project.refresh()
end

phon.add_folder = function()
	var path = open_directory_dialog("Add content of folder...")
	if path then
		phon.project.add_folder(path)
		phon.project.refresh()
	end
end

function save_project_as()
	var path = save_file_dialog("Save project as...", "Phonometrica project (*.phon-project)")
	if path then
		if not path.ends_with(".phon-project") then
			path = path & ".phon-project"
		end
		phon.project.save(path)
		update_recent_projects(path)
	end
end

phon.save_project = function()
	if phon.project.has_path() then
		phon.project.save()
	else
		save_project_as()
	end
end



function new_query()
	if phon.project.is_empty() then
		alert("The current project is empty!")
	else
		phon.open_query_editor()
	end
end



phon.show_help = function()
	var page = phon.config.get_documentation_page("index.html")
	phon.show_documentation(page)
end

function show_acknowledgements()
	var page = phon.config.get_documentation_page("about/acknowledgements.html")
	phon.show_documentation(page)
end

function show_license()
	var page = phon.config.get_documentation_page("about/license.html")
	phon.show_documentation(page)
end

function show_about()
	var title = "About Phonometrica"
	var msg = "<b>Phonometrica " & phon.version & "</b><br/>\n" 
		& "Copyright (C) 2019 Julien Eychenne & Léa Courdès-Murphy<br/>" 
		& "Phonometrica is distributed under the CECILL license version 2.1.<br/>"
		& "See \"License\" for details.<br/>"
		& "Website: <a href=\"http://www.phonometrica-ling.org\">http://www.phonometrica-ling.org</a><br/>"
		& "Icons by <a href=\"https://icons8.com\">Icons8</a>.<br/>"
		& "Build date: " & phon.date
	about(msg, title)
end

function show_scripting()
	var page = phon.config.get_documentation_page("scripting/index.html")
	phon.show_documentation(page)
end

function sound_info()
	var title = "Sound information"
	var msg = "<b>Supported sound formats on this platform</b><br/>"
		& phon.supported_sound_formats.join(", ")
		& "<br/><b>libsndfile version</b><br/> " & phon.libsndfile_version
		& "<br/><b>RTAudio version</b><br/> " & phon.rtaudio_version
    about(msg, title)
end

function view_log()
	var path = system.join_path(system.user_directory, "phonometrica.log")
	if system.exists(path) then
		view_text(path, "View log", 500)
	else
		info("There is no log file")
	end
end

function erase_log()
	var path = system.join_path(system.user_directory, "phonometrica.log")
	if system.exists(path) then
		system.remove_file(path)
	end
end

function show_release()
	var page = phon.config.get_documentation_page("about/release-notes.html")
	phon.show_documentation(page)
end



function populate_menu(menu, desc)
	foreach var item in desc do
		if not item then
			phon.add_separator(menu)

		elsif typeof item[2] == "List" then
			var lst = item[2]
			var submenu = phon.add_submenu(menu, item[1])

			if lst.is_empty() and item.length > 2 then
				item[3](submenu)
			else
				populate_menu(submenu, lst)
			end

		else
			var action = phon.add_action(menu, item[1])
			phon.bind_action(action, item[2])
			if item.length > 2 then
				phon.set_action_shortcut(action, item[3])
			end
			if item.length > 3 then
				phon.actions[item[4]] = action
			end
		end
	end
end

function create_menu(desc, name)
	var menu = phon.add_menu(name)
	populate_menu(menu, desc)

	return menu
end


file_menu = [
	["New script...", view_script],
	["Run script...", run_script],
	null,
	["Open project...", phon.open_project, "ctrl+o"],
	["Add file(s) to project...", phon.add_files, "ctrl+shift+a"],
	["Add content of directory to project...", phon.add_folder],
	["Close current project", phon.project.close],
	null,
	["Recent projects", [], set_recent_projects],
	["Open most recent project", open_most_recent_project, "ctrl+shift+o", "most_recent"],
	null,
	["Save project as...", save_project_as],
	["Save project", phon.save_project, "ctrl+shift+s"],
	null,
	["Settings...", phon.edit_settings],
	null,
	["Import", [
		["Import metadata from CSV file...", import_metadata],
	]],
	["Export", [
		["Export annotation(s) to plain text...", phon.transphon],
		["Export project metadata to CSV file...", export_metadata],
	]],
	null,
	["Close current view", phon.close_current_view, "ctrl+w"],
	null,
	["Quit", phon.quit, "ctrl+q"]
]

analysis_menu = [
	["Search annotations...", new_query, "ctrl+e"],
	["Measure formants...", phon.measure_formants],
	null,
	["Edit last query", phon.run_last_query, "ctrl+l"]
]

help_menu = [
	["Documentation", phon.show_help],
	null,
	["Scripting", show_scripting],
	null,
	["License", show_license],
	["Release notes", show_release],
	["Acknowledgements", show_acknowledgements],
	null,
	["Sound information", sound_info],
	null,
	["Debug", [
		["View log file", view_log],
		["Clear log file", erase_log]
	]],
	null,
	["About Phonometrica", show_about]
]

phon.actions = {} 

create_menu(file_menu, "File")
create_menu(analysis_menu, "Analysis")
phon.create_window_menu() # Handled in C++ code
phon.create_tool_menu()   # Handled in C++ code
create_menu(help_menu, "Help")
enable_most_recent())_";

#endif /* MENU_PHON_SCRIPT_INCLUDE */