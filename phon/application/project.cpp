/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <algorithm>
#include <phon/regex.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/bookmark.hpp>
#include <phon/application/macros.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/text.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

AutoProject Project::instance;

static String event_module_name("event");
static String emit_name("emit");
static String annotation_imported("__SIGNAL_ANNOTATION_IMPORTED");
static String sound_imported("__SIGNAL_SOUND_IMPORTED");
static String annotation_loaded("__SIGNAL_ANNOTATION_LOADED");
static String sound_loaded("__SIGNAL_SOUND_LOADED");
static String script_loaded("__SIGNAL_SCRIPT_LOADED");
static String dataset_loaded("__SIGNAL_DATASET_LOADED");
static String project_loaded("__SIGNAL_PROJECT_LOADED");
// application variables
static const std::string_view VAR_APPDIR("$PHON_APPDIR");
static const std::string_view VAR_HOME("$HOME");
static const std::string_view VAR_PROJECT("$PHON_PROJECT");


void Project::open(String path)
{
    close();
    set_path(std::move(path));
    load();
}

Project::Project(Runtime &rt, String path) :
		rt(rt),
		m_corpus(make_handle<Directory>(nullptr, "Corpus")),
		m_bookmarks(make_handle<Directory>(nullptr, "Bookmarks")),
		m_scripts(make_handle<Directory>(nullptr, "Scripts")),
		m_data(make_handle<Directory>(nullptr, "Data")),
		m_queries(make_handle<Directory>(nullptr, "Queries"))
{
	if (path.empty())
	{
		create_uuid();
		open_database();
	}
	else
	{
		set_path(std::move(path));
		load();
	}

	initialized();
}

Project::~Project()
{
	discard_database();
}

Project *Project::get()
{
	return instance.get();
}

bool Project::modified() const
{
	return m_modified || m_corpus->modified() || m_scripts->modified() || m_bookmarks->modified() || m_data->modified() || m_queries->modified();
}

void Project::discard_database()
{
	if (m_database_temp && m_database)
	{
		auto path = m_database->path();
        if (filesystem::exists(path))
        {
			m_database->close();
            filesystem::remove_file(path);
		}
	}

    m_database = nullptr;
	m_database_temp = true;
}

const String &Project::path() const
{
	return m_path;
}

bool Project::has_path() const
{
	return !m_path.empty();
}

void Project::set_path(String value)
{
	filesystem::nativize(value);
	m_directory = filesystem::directory_name(value);
	m_path = std::move(value);
}

void Project::save(String path)
{
    set_path(std::move(path));
    save();
}

void Project::save()
{
	start_activity();
	// Save content before writing the project, because write() will reset modifications on all VFiles.
	m_corpus->save_content();
	m_scripts->save_content();
	m_data->save_content();
	m_queries->save_content();

	write();

	m_modified = false;
	// Once the project has been written to disk, its database must not be discarded.
	m_database_temp = false;

	notify_update();
	stop_activity();
}

void Project::load()
{
	assert(!m_path.empty());
	assert(m_accumulator.empty());
	start_activity();

	xml_document doc;
	xml_node root = read_xml(doc, m_path);

	static const std::string_view project_tag("Phonometrica");
	static const std::string_view class_tag("Project");
	static const std::string_view corpus_tag("Corpus");
	static const std::string_view meta_tag("Metadata");
	static const std::string_view queries_tag("Queries");
	static const std::string_view scripts_tag("Scripts");
	static const std::string_view bookmarks_tag("Bookmarks");
	static const std::string_view data_tag("Data");

	if (root.name() != project_tag) {
		throw error("[Input/Output] Invalid XML project root");
	}

    auto attr = root.attribute("class");

	if (!attr || attr.as_string() != class_tag) {
	    throw error("[Input/Output] Expected a Project file, got a % file instead", attr.as_string());
	}

	attr = root.attribute("label");
	if (attr) {
		m_label = attr.value();
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == corpus_tag)
		{
			int size = 0;
			auto attr = node.attribute("size");
			if (attr) {
				size = attr.as_int();
			}

			parse_corpus(node, m_corpus.get());
		}
		else if (node.name() == meta_tag)
		{
			parse_metadata(node);
		}
		else if (node.name() == queries_tag)
		{
			parse_queries(node, m_queries.get());
		}
		else if (node.name() == scripts_tag)
		{
			parse_scripts(node, m_scripts.get());
		}
		else if (node.name() == bookmarks_tag)
		{
			parse_bookmarks(node, m_bookmarks.get());
		}
		else if (node.name() == data_tag)
		{
			parse_data(node, m_data.get());
		}
	}

	m_database_temp = false;
	bind_annotations();
	notify_update();
	stop_activity();
	emit(project_loaded);
}

const Handle<Directory> & Project::corpus() const
{
	return m_corpus;
}

const Handle<Directory> & Project::bookmarks() const
{
	return m_bookmarks;
}

const Handle<Directory> & Project::scripts() const
{
	return m_scripts;
}

void Project::parse_corpus(xml_node root, Directory *folder)
{
	int counter = 0;
	static const std::string_view folder_tag("VFolder");
	static const std::string_view file_tag("VFile");


	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == folder_tag)
		{
			String label;
			auto attr = node.attribute("label");
			if (attr) label = attr.value();
			auto subfolder = make_handle<Directory>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_corpus(node, sub);
		}
		else if (node.name() == file_tag)
		{
			auto attr = node.attribute("class");
			if (!attr) throw error("Invalid VFile node");
			std::string_view cls(attr.value());
			Handle<Document> vfile;
			String path(node.text().get());
			interpolate(path, m_directory);

			if (cls == "Annotation")
			{
				auto annot = make_handle<Annotation>(folder, std::move(path));
				vfile = recast<Document>(annot);
				emit(annotation_loaded, std::move(annot));
			}
			else if (cls == "Sound")
			{
				auto sound = make_handle<Sound>(folder, std::move(path));
				vfile = recast<Document>(sound);
				emit(sound_loaded, std::move(sound));
			}
			else
			{
				throw error("Invalid VFile type: %", cls);
			}

			register_file(vfile->path(), vfile);
			folder->append(vfile, false);
		}
		else
		{
			throw error("Invalid file system node in project");
		}
	}
}

void Project::register_file(const String &path, Handle<Document> file)
{
	m_database->add_metadata_to_file(file);

	if (m_files.find(path) == m_files.end())
	{
		m_files.insert({path, std::move(file)});
	}
}

void Project::parse_metadata(xml_node root)
{
	static const std::string_view changelog_tag("Changelog");
	static const std::string_view bookmarks_tag("Bookmarks");
	static const std::string_view uuid_tag("UUID");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == changelog_tag)
		{
			parse_changelog(node);
		}
			// Projects in version 1.x put the bookmarks in the metadata, so we must remain backward-compatible.
		else if (node.name() == bookmarks_tag)
		{
			parse_bookmarks(node, m_bookmarks.get());
		}
		else if (node.name() == uuid_tag)
		{
			// If we have a valid id, we use it for the the database.
			String id = node.text().get();

			if (!id.empty())
			{
				discard_database();
				m_uuid = std::move(id);
				open_database();
				// Force the database to be persistent, even if it doesn't exist (for instance, if the user deleted it).
				m_database_temp = false;
			}
		}

		// Note: for now, we don't parse the program version node, but it can be used in the future to ensure
		// backward compatibility, in case the XML format changes.
	}
}

void Project::parse_scripts(xml_node root, Directory *folder)
{
	static const std::string_view folder_tag("VFolder");
	static const std::string_view file_tag("VFile");
	static const std::string_view script_tag("Script");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == folder_tag)
		{
			String label;
			auto attr = node.attribute("label");
			if (attr) label = attr.value();
			auto subfolder = make_handle<Directory>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_scripts(node, sub);
		}
		else if (node.name() == file_tag)
		{
			auto attr = node.attribute("class");
			if (!attr) throw error("Invalid VFile node");
			std::string_view cls(attr.value());
			String path(node.text().get());
			interpolate(path, m_directory);

			if (cls == script_tag)
			{
				auto script = make_handle<Script>(folder, std::move(path));
				folder->append(script, false);
				register_file(script->path(), script);
//				emit(script_loaded, make_handle<Script>(script));
			}
			else
			{
				throw error("Invalid class \"%\" in VFile XML entry", node.name());
			}
		}
	}
}

void Project::parse_queries(xml_node root, Directory *folder)
{
	static const std::string_view folder_tag("VFolder");
	static const std::string_view file_tag("VFile");
	static const std::string_view text_query_tag("Query");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == folder_tag)
		{
			String label;
			auto attr = node.attribute("label");
			if (attr) label = attr.value();
			auto subfolder = make_handle<Directory>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_queries(node, sub);
		}
		else if (node.name() == file_tag)
		{
			auto attr = node.attribute("class");
			if (!attr) throw error("Invalid VFile node");
			std::string_view cls(attr.value());
			String path(node.text().get());
			interpolate(path, m_directory);

			if (cls == text_query_tag)
			{
				auto query = make_handle<Query>(folder, std::move(path));
				folder->append(query, false);
				register_file(query->path(), query);
			}
			else
			{
				throw error("Invalid query type \"%\" in VFile XML entry", cls);
			}
		}
	}

}

void Project::parse_data(xml_node root, Directory *folder)
{
	static const std::string_view folder_tag("VFolder");
	static const std::string_view file_tag("VFile");
	static const std::string_view data_tag("Dataset");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == folder_tag)
		{
			String label;
			auto attr = node.attribute("label");
			if (attr) label = attr.value();
			auto subfolder = make_handle<Directory>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_scripts(node, sub);
		}
		else if (node.name() == file_tag)
		{
			auto attr = node.attribute("class");
			if (!attr) throw error("Invalid VFile node");
			std::string_view cls(attr.value());
			String path(node.text().get());
			interpolate(path, m_directory);

			if (cls == data_tag)
			{
				auto dataset = make_handle<Spreadsheet>(folder, std::move(path));
				dataset->from_xml(node, m_directory);
				folder->append(dataset, false);
				register_file(dataset->path(), dataset);
//				emit(dataset_loaded, make_handle<AutoDataset>(std::move(dataset)));
			}
			else if (cls == std::string_view("Concordance"))
			{
				auto conc = make_handle<Concordance>(folder, std::move(path));
				folder->append(conc, false);
				register_file(conc->path(), conc);
			}
			else
			{
				throw error("Invalid class \"%\" in VFile XML entry", cls);
			}
		}
	}
}

void Project::parse_bookmarks(xml_node root, Directory *folder)
{
	static const std::string_view folder_tag("VFolder");
	static const std::string_view bookmark_tag("Bookmark");
	static const std::string_view match_tag("SearchMatch");
	static const std::string_view stamp_tag("AnnotationStamp");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == folder_tag)
		{
			String label;
			auto attr = node.attribute("label");
			if (attr) label = attr.value();
			auto subfolder = make_handle<Directory>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_bookmarks(node, sub);
		}
		else if (node.name() == bookmark_tag)
		{
			auto attr = node.attribute("type");

			// Dolmen 1 used SearchMatch instead of AnnotationStamp.
			if (attr && (attr.value() == match_tag || attr.value() == stamp_tag))
			{
				auto title_node = node.child("Title");
				auto notes_node = node.child("Notes");
				auto match_node = node.child("Match");
				auto left_node  = node.child("LeftContext");
				auto right_node = node.child("RightContext");
				auto file_node  = node.child("Annotation");
				auto start_node = node.child("Start");
				auto end_node   = node.child("End");

				auto layer_node = node.child("Layer");
				// Dolmen 1 used Tier instead of Layer.
				if (!layer_node) layer_node = node.child("Tier");

				// Note: Left and right are optional.
				if (!title_node || !notes_node || !match_node || !file_node || !start_node || !end_node || !layer_node) {
					throw error("Invalid annotation stamp");
				}

				String title(title_node.text().get());
				String notes(notes_node.text().get());
				String path(file_node.text().get());
				String match(match_node.text().get());

				String left, right;
				if (left_node) left = left_node.text().get();
				if (right_node) right = right_node.text().get();

				size_t layer = layer_node.text().as_uint();
				double start = start_node.text().as_double();
				double end = end_node.text().as_double();

				interpolate(path, directory());
				auto file = get_file_handle(path, "needed by bookmark");

				if (file->is_annotation())
				{
					auto annot = recast<Annotation>(file);
					auto bookmark = make_handle<TimeStamp>(folder, title, std::move(annot), layer, start, end, match, left, right);
					folder->append(std::move(bookmark), false);
				}
				else
				{
					throw error("Invalid file type in Bookmark \"%\": expected Annotation, got %", title, file->class_name());
				}
			}
			else
			{
				throw error("Invalid bookmark type");
			}
		}
	}
}

const Handle<Document> & Project::get_file_handle(const String &path, std::string_view msg)
{
	auto it = m_files.find(path);

	if (it == m_files.end()) {
		throw error("File \"%\" does not belong to the project: %", path, msg);
	}

	return it->second;
}

void Project::parse_changelog(xml_node root)
{
	m_changelog.from_xml(root);
}

const String &Project::directory() const
{
	return m_directory;
}

void Project::write_corpus(xml_node root)
{
	auto attr = root.append_attribute("size");
	attr.set_value(m_corpus->size());

	for (auto &file : *m_corpus) {
		file->to_xml(root);
	}

	m_corpus->discard_changes();
}

void Project::write_metadata(xml_node root)
{
	auto id_node = root.append_child("UUID");
	auto data = id_node.append_child(node_pcdata);
	data.set_value(uuid().data());

	auto version_node = root.append_child("ProgramVersion");
	auto major_attr = version_node.append_attribute("major");
	major_attr.set_value((int)PHON_VERSION_MAJOR);
	auto minor_attr = version_node.append_attribute("minor");
	minor_attr.set_value((int)PHON_VERSION_MINOR);
	auto micro_attr = version_node.append_attribute("micro");
	micro_attr.set_value((int)PHON_VERSION_MICRO);

	m_changelog.to_xml(root);
	m_changelog.reset_modifications();
}

void Project::write_bookmarks(xml_node root)
{
	for (auto &file : *m_bookmarks) {
		file->to_xml(root);
	}

	m_bookmarks->discard_changes();
}

void Project::write_scripts(xml_node root)
{
	for (auto &file : *m_scripts) {
		file->to_xml(root);
	}

	m_scripts->discard_changes();
}

void Project::write_data(xml_node root)
{
	for (auto &file : *m_data) {
		file->to_xml(root);
	}

	m_scripts->discard_changes();
}

void Project::write_queries(xml_node root)
{
	for (auto &file : *m_queries) {
		file->to_xml(root);
	}

	m_queries->discard_changes();
}

void Project::write()
{
	assert(has_path());
	xml_document doc;
	auto root = doc.append_child("Phonometrica");
	root.append_attribute("class").set_value("Project");
    if (!m_label.empty()) {
    	root.append_attribute("label").set_value(m_label.data());
    }

	// Metadata must come first, so that we can get the metadata file's UUID.
	auto meta_node = root.append_child("Metadata");
	write_metadata(meta_node);

	auto fs_node = root.append_child("Corpus");
	write_corpus(fs_node);

	auto queries_node = root.append_child("Queries");
	write_queries(queries_node);

	auto scripts_node = root.append_child("Scripts");
	write_scripts(scripts_node);

	auto data_node = root.append_child("Data");
	write_data(data_node);

	auto bookmarks_node = root.append_child("Bookmarks");
	write_bookmarks(bookmarks_node);

	write_xml(doc, m_path);
}

bool Project::add_file(String path, const Handle<Directory> &parent, FileType type, bool importing)
{
	if (m_files.find(path) != m_files.end())
	{
		return set_import_flag();
	}

	auto ext = filesystem::ext(path, true);

	Handle<Document> vfile;

	if ((ext == PHON_EXT_ANNOTATION || ext == ".textgrid") && (static_cast<int>(type)&static_cast<int>(FileType::Annotation)))
	{
	    auto annot = make_handle<Annotation>(parent.get(), std::move(path));
		vfile = recast<Document>(annot);
		parent->append(vfile);
		if (importing) {
			emit(annotation_imported, annot);
		}
		else {
			emit(annotation_loaded, annot);
		}
	}
	else if (Sound::supports_format(ext) && (static_cast<int>(type)&static_cast<int>(FileType::Sound)))
	{
	    auto sound = make_handle<Sound>(parent.get(), std::move(path));
		vfile = recast<Document>(sound);
		parent->append(vfile);
		if (importing) {
			emit(sound_imported, sound);
		}
		else {
			emit(sound_loaded, sound);
		}
	}
	else if (ext == PHON_EXT_QUERY)
	{
		Directory *p = m_queries.get();
		Handle<Query> query;

		if (static_cast<int>(type) & static_cast<int>(FileType::Query))
		{
			if (parent->toplevel() == p) {
				p = parent.get();
			}
		}
		else
		{
			return set_import_flag();
		}

		auto query_type = get_query_type(path);
		if (query_type == Query::Type::Text)
		{
			query = make_handle<Query>(p, std::move(path));
		}
		else
		{
			throw error("Cannot parse query file: unsupported query type");
		}

		vfile = recast<Document>(query);
		p->append(vfile);
	}
	else if (ext == PHON_EXT_CONCORDANCE)
	{
		Directory *p = m_scripts.get();

		if (static_cast<int>(type) & static_cast<int>(FileType::Dataset))
		{
			if (parent->toplevel() == p) {
				p = parent.get();
			}
		}
		else
		{
			return set_import_flag();
		}

		auto conc = make_handle<Concordance>(p, std::move(path));
		vfile = recast<Document>(conc);
		p->append(vfile);
	}
	else if (ext == ".csv")
	{
		Directory *p = m_data.get();

		if (static_cast<int>(type) & static_cast<int>(FileType::Dataset))
		{
			if (parent->toplevel() == p) {
				p = parent.get();
			}
		}
		else
		{
			return set_import_flag();
		}

		auto dataset = make_handle<Spreadsheet>(p, std::move(path));
		vfile = recast<Document>(dataset);
		p->append(vfile);
//		emit(dataset_loaded, std::move(dataset));
	}
	else if ((ext == PHON_EXT_SCRIPT || ext == ".phon-script"))
	{
		Directory *p = m_scripts.get();

		if (static_cast<int>(type) & static_cast<int>(FileType::Script))
		{
			if (parent->toplevel() == p) {
				p = parent.get();
			}
		}
		else
		{
			return set_import_flag();
		}

        auto script = make_handle<Script>(p, std::move(path));
		vfile = recast<Document>(script);
		p->append(vfile);
		//emit(script_loaded, make_handle<Handle<Script>>(std::move(script)));
	}
	else
	{
		return set_import_flag(); // Ignore unknown files

	}
	register_file(vfile->path(), vfile);

	return true;
}

bool Project::has_uuid() const
{
	return !m_uuid.empty();
}

const String &Project::uuid() const
{
	return m_uuid;
}

void Project::import_directory(String path)
{
	filesystem::nativize(path);
	add_folder(std::move(path), m_corpus, true);
	bind_annotations();
	// Try to find a sound that matches the annotation's name.
	set_default_bindings();
	m_modified = true;
}

String Project::import_file(String path)
{
	interpolate(path, directory());
	filesystem::nativize(path);

	// Assume that the file will be added to the corpus. add_file() will change the parent if necessary.
	if (add_file(path, m_corpus, FileType::Any, false))
	{
        m_modified = true;
        auto &f = m_files[path];
        if (f->is_annotation()) {
	        emit(annotation_imported, recast<Annotation>(f));
        }
        else if (f->is_sound()) {
        	emit(sound_imported, recast<Sound>(f));
        }
	}
	// Try to find a sound that matches the annotation's name.
	set_default_bindings();

	return path;
}

void Project::add_folder(String path, const Handle<Directory> &parent, bool importing)
{
	auto content = filesystem::list_directory(path);
	auto count = content.size();

	for (intptr_t counter = 1; counter <= count; ++counter)
	{
		auto &name = content[counter];
		auto file = filesystem::join(path, name);

		if (filesystem::is_directory(file))
		{
			auto subfolder = make_handle<Directory>(parent.get(), name);
			parent->append(subfolder);
			add_folder(file, subfolder, importing);
		}
		else
		{
			add_file(file, parent, FileType::Any, importing);
		}
	}
}

std::set<Property> Project::get_shared_properties(const DocList &files)
{
	auto properties = files.front()->properties();
	auto count = files.size();

	for (intptr_t i = 2; i <= count; ++i)
	{
		std::set<Property> temp_result;
		auto &file_properties = files[i]->properties();
		std::set_intersection(properties.begin(), properties.end(), file_properties.begin(), file_properties.end(),
							  std::inserter(temp_result, temp_result.end()));
		properties.swap(temp_result);
	}

	return properties;
}

void Project::open_database()
{
	String name(m_uuid);
	name.append(".db");
	auto path = filesystem::join(Settings::metadata_directory(), name);
	m_database_temp = !filesystem::exists(path);
	m_database = std::make_unique<MetaDatabase>(path, m_database_temp);
	m_database->notify_annotation_needs_sound.connect(&Project::bind_annotation, this);
}

void Project::remove(DocList &files)
{
	for (auto &file : files)
	{
        remove(file);
	}
}

void Project::remove(ElementList &files)
{
	for (auto &file : files)
	{
		if (file->is_folder())
		{
			remove(recast<Directory>(file));
		}
		else
		{
			remove(recast<Document>(file));
		}
	}
}

const Handle<Directory> & Project::data() const
{
	return m_data;
}

const Handle<Directory> &Project::queries() const
{
	return m_queries;
}


void Project::create_uuid()
{
	if (! has_uuid()) {
		m_uuid = utils::new_uuid();
	}
}

MetaDatabase & Project::database() const
{
	return *m_database;
}

void Project::bind_annotation(const Handle<Annotation> &annot, const String &sound_file)
{
	// The actual binding will occur in bind_annotations(), which is called once the project is loaded.
	m_accumulator.emplace_back(annot.get(), sound_file);
}

void Project::bind_annotations()
{
	for (auto &pair : m_accumulator)
	{
		auto annot = pair.first;
		auto &sound_path = pair.second;

		auto it = m_files.find(sound_path);

		if (it != m_files.end())
		{
			auto snd = recast<Sound>(it->second);
			// This method is only called when a project is loading, so we don't mutate the annotation
			annot->set_sound(snd, false);
		}
		else
		{
			auto msg = utils::format("Cannot bind annotation \"%\" to sound \"%\" because the sound is not part of the project",
					annot->path(), sound_path);
			notify_error(msg);
		}
	}

	m_accumulator.clear();
}

void Project::close()
{
	instance->about_to_close();
	instance->clear();
	instance->notify_closed();
}

DocList Project::get_corpus_files() const
{
	DocList files;
	files.reserve(m_files.size());

	for (auto &entry : m_files) {
		files.push_back(entry.second);
	}

	return files;
}

void Project::remove_empty_script()
{
	for (intptr_t i = m_scripts->size(); i > 0; i--)
	{
		auto &node = m_scripts->get(i);

		if (node->is_script())
		{
			auto script = raw_recast<Script>(node);

			if (!script->has_path()) {
				m_scripts->remove(node);
			}
		}
	}
}

void Project::initialize(Runtime &rt)
{
	Annotation::initialize(rt);
	Sound::initialize(rt);
	Dataset::initialize(rt);
	Spreadsheet::initialize(rt);
	TimeStamp::initialize(rt);
	Script::initialize(rt);
	Query::initialize(rt);

	auto open_project = [](Runtime &, std::span<Variant> args) -> Variant
	{
		auto &path = cast<String>(args[0]);
		Project::get()->open(path);
		return Variant();
	};

	auto close_project = [](Runtime &, std::span<Variant> args) -> Variant
	{
		Project::get()->close();
		Project::get()->reinitialize();
		return Variant();
	};

	auto add_folder = [](Runtime &, std::span<Variant> args) -> Variant
    {
	    auto &path = cast<String>(args[0]);
	    Project::get()->import_directory(path);
	    return Variant();
    };

    auto add_file = [](Runtime &, std::span<Variant> args) -> Variant
    {
        auto &path = cast<String>(args[0]);
	    Project::get()->import_file(path);
        return Variant();
    };

    auto refresh_project = [](Runtime &, std::span<Variant>) -> Variant
	{
    	Project::updated();
    	return Variant();
	};

    auto project_has_path = [](Runtime &, std::span<Variant>) -> Variant {
       	return Project::get()->has_path();
    };

    auto save_project1 = [](Runtime &, std::span<Variant>) -> Variant {
	    Project::get()->save();
        return Variant();
    };

    auto save_project2 = [](Runtime &, std::span<Variant> args) -> Variant {
    	auto &path = cast<String>(args[0]);
	    Project::get()->save(path);
        return Variant();
    };

    auto get_annotations = [](Runtime &rt, std::span<Variant> args) -> Variant {
    	Array<Variant> result;
    	std::vector<Handle<Annotation>> tmp;
    	auto &files = Project::get()->m_files;
    	for (auto &pair : files)
		{
    		if (pair.second->is_annotation())
			{
    		    tmp.push_back(recast<Annotation>(pair.second));
			}
		}
    	result.reserve(tmp.size());
    	std::sort(tmp.begin(), tmp.end(), [](const Handle<Annotation> &a1, const Handle<Annotation> &a2) { return a1->path() < a2->path(); });

    	for (auto &annot : tmp) {
		    result.append(std::move(annot));
	    }

    	return make_handle<List>(&rt, std::move(result));
    };

    auto get_annotation = [](Runtime &rt, std::span<Variant> args) -> Variant {
	    auto &files = Project::get()->m_files;
	    auto &path = cast<String>(args[0]);
	    auto file = files.find(path);

	    if (file == files.end()) {
		    return Variant();
	    }
	    else if (file->second->is_annotation()) {
			return recast<Annotation>(file->second);
	    }
	    else {
		    throw error("File \"%\" is not an annotation", path);
	    }
    };

    auto get_sounds = [](Runtime &rt, std::span<Variant> args) -> Variant {
    	Array<Variant> result;
    	std::vector<Handle<Sound>> tmp;
    	auto &files = Project::get()->m_files;
    	for (auto &pair : files)
		{
    		if (pair.second->is_sound())
			{
    		    tmp.push_back(recast<Sound>(pair.second));
			}
		}
    	result.reserve(tmp.size());
    	std::sort(tmp.begin(), tmp.end(), [](const Handle<Sound> &s1, const Handle<Sound> &s2) { return s1->path() < s2->path(); });

    	for (auto &sound : tmp) {
		    result.append(std::move(sound));
	    }

    	return make_handle<List>(&rt, std::move(result));
    };

    auto get_sound = [](Runtime &rt, std::span<Variant> args) -> Variant {
    	auto &files = Project::get()->m_files;
    	auto &path = cast<String>(args[0]);
		auto file = files.find(path);

		if (file == files.end()) {
			return Variant();
		}
		else if (file->second->is_sound()) {
			return recast<Sound>(file->second);
		}
		else {
			throw error("File \"%\" is not a sound", path);
		}
    };

    auto is_empty = [](Runtime &rt, std::span<Variant> args) -> Variant {
        return Project::get()->empty();
    };
 #define CLS(T) get_class<T>()
	rt.add_global("get_annotations", get_annotations, { });
	rt.add_global("get_annotation", get_annotation, { CLS(String) });
	rt.add_global("get_sounds", get_sounds, { });
	rt.add_global("get_sound", get_sound, { CLS(String) });

	// Create submodule for project.
	// FIXME: DO we put this in phon or global?
	auto proj = make_handle<Module>(&rt, "project");
	proj->define(&rt, "open", open_project, { CLS(String) });
	proj->define(&rt, "close", close_project, { });
	proj->define(&rt, "add_folder", add_folder, { CLS(String) });
	proj->define(&rt, "add_file", add_file, { CLS(String) });
	proj->define(&rt, "refresh", refresh_project, { });
	proj->define(&rt, "has_path", project_has_path, { });
	proj->define(&rt, "save", save_project1, { });
	proj->define(&rt, "save", save_project2, { CLS(String) });
	proj->define(&rt, "is_empty", is_empty, { });
	auto &phon = cast<Module>(rt["phon"]);
	phon.define("project", std::move(proj));
#undef CLS
}

void Project::clear()
{
    m_corpus->clear(false);
    m_data->clear(false);
    m_scripts->clear(false);
    m_bookmarks->clear(false);
    m_queries->clear(false);
    discard_database();
	m_uuid.clear();
    m_path = String();
    m_directory = String();
    m_files.clear();
    m_modified = false;
    assert(m_accumulator.empty());
}

String Project::label() const
{
	using namespace filesystem;

	if (!m_label.empty()) {
		return String::format("Project %s", m_label.data());
	}

    if (m_path.empty())
        return "Project";

    String label("Project ");
    label.append(base_name(strip_ext(path())));

    return label;
}

Handle<Document> Project::get(const String &path)
{
    auto it = m_files.find(path);
    if (it != m_files.end()) {
	    return it->second;
    }

    return Handle<Document>(nullptr);
}

void Project::modify()
{
	m_modified = true;
}

void Project::create(Runtime &rt)
{
	instance = std::make_unique<Project>(rt, String());
}

void Project::updated()
{
	get()->notify_update();
}

void Project::remove(const Handle<Directory> &folder)
{
    for (intptr_t i = folder->size(); i > 0; i--)
    {
    	auto &node = folder->get(i);
        if (node->is_document())
        {
            auto file = recast<Document>(node);
            remove(file);
        }
        else if (node->is_folder())
        {
            auto subfolder = recast<Directory>(node);
            remove(subfolder);
        }
        else
        {
            throw std::runtime_error("[Internal error] Unhandled case in Project::remove()");
        }
    }
    folder->detach();
}

void Project::remove(const Handle<Document> &folder)
{
    folder->detach();
    m_files.erase(folder->path());
    m_modified = true;
}

bool Project::is_root(const Directory *folder) const
{
    return folder == m_corpus.get() || folder == m_scripts.get() || folder == m_queries.get() || folder == m_data.get() || folder == m_bookmarks.get();
}

void Project::emit(const String &signal, Variant value)
{
    rt.push(rt[emit_name]);
    rt.push(signal);
    rt.push(std::move(value));
    rt.call(2);
}

void Project::emit(const String &signal)
{
    rt.push(rt[emit_name]);
    rt.push(signal);
    rt.call(1);
}

void Project::import_metadata(const String &path, const String &separator)
{
	auto csv = utils::parse_csv(path, separator);
	auto header = csv.take_first();
	const char *placeholders[] = { "%1", "%2", "%3", "%4", "%5", "%6", "%7", "%8", "%9" };

	// Scan each row
	for (auto &row : csv)
	{
		// Either file name or a regular expression starting with '^' and ending with '$'
		auto &filename = row.first();
		std::unique_ptr<Regex> re;

		if (filename.starts_with('^') && filename.ends_with('$'))
		{
			re = std::make_unique<Regex>(filename);
		}

		// Scan each column
		for (intptr_t j = 2; j <= header.size(); j++)
		{
			// Try to match each file
			for (auto &item : m_files)
			{
				auto base = filesystem::base_name(item.first);

				if ((re && re->match(base)) || (base == filename))
				{
					auto category = header[j].trim();
					auto value = row[j].trim();

					if (re)
					{
						auto count = re->count();
						value.replace("%%", re->capture(0));

						for (int i = 1; i <=9; i++)
						{
							if (count >= i)
								value.replace(placeholders[i-1], re->capture(i));
							else
								break;
						}
					}

					if (!category.empty() && !value.empty()) {
						tag_file(item.second, category, value);
					}
				}
			}
		}
	}

	bind_annotations();
	metadata_updated();
}

void Project::export_metadata(const String &path)
{
	auto paths = m_files.keys();
	std::sort(paths.begin(), paths.end());
	auto &categories = Property::get_categories();
	Array<Array<String>> csv;
	Array<String> header;
	header.append("File");
	header.append("%SOUND%");
	header.append("%DESCRIPTION%");

	for (auto cat : categories)
	{
		auto &type = Property::get_type(cat);
		if (type == typeid(double)) {
			cat.append(".num");
		}
		else if (type == typeid(bool)) {
			cat.append(".bool");
		}

		header.append(std::move(cat));
	}
	csv.append(std::move(header));

	for (auto &filename : paths)
	{
		auto file = m_files[filename];
		Array<String> row;
		row.append(filesystem::base_name(file->path()));
		if (file->is_annotation())
		{
			auto annot = dynamic_cast<Annotation*>(file.get());
			auto snd = annot->has_sound() ? annot->sound()->path() : String();
			row.append(std::move(snd));
		}
		else
		{
			row.append(String());
		}
		row.append(file->description());

		for (auto &cat : categories)
		{
			auto prop = file->get_property(cat);
			if (prop.valid()) {
				row.append(prop.value());
			}
			else {
				row.append(String());
			}
		}

		csv.append(std::move(row));
	}

	utils::write_csv(path, csv, "\t");
}

void Project::tag_file(Handle<Document> &file, const String &category, const String &value)
{

	if (category == "%SOUND%")
	{
		if (filesystem::exists(value) && file->is_annotation())
		{
			auto annot = recast<Annotation>(file);
			bind_annotation(annot, value);
		}

	}
	else if (category == "%DESCRIPTION%")
	{
		file->set_description(value);
	}
	else
	{
		file->add_property(Property::from_string(category, value));
	}
}

bool Project::empty() const
{
    return m_corpus->empty() && m_data->empty() && m_scripts->empty() && m_bookmarks->empty();
}

Array<Handle<Annotation>> Project::annotations() const
{
	Array<Handle<Annotation>> result;

	for (auto &item : m_files)
	{
		auto &vf = item.second;

		if (vf->is_annotation())
		{
			result.push_back(recast<Annotation>(vf));
		}
	}
	std::sort(result.begin(), result.end(), [](const Handle<Annotation> &a1, const Handle<Annotation> &a2) -> bool {
		return a1->path() < a2->path();
	});

	return result;
}

Array<Handle<Concordance>> Project::concordances() const
{
	Array<Handle<Concordance>> result;

	for (auto &item : m_files)
	{
		auto &vf = item.second;

		if (vf->is_concordance())
		{
			result.push_back(recast<Concordance>(vf));
		}
	}
	std::sort(result.begin(), result.end(), [](const Handle<Concordance> &c1, const Handle<Concordance> &c2) -> bool {
		return c1->label() < c2->label();
	});

	return result;
}

void Project::set_default_bindings()
{
	for (auto &item : m_files)
	{
		auto &vf = item.second;

		if (vf->is_annotation())
		{
			auto annot = raw_recast<Annotation>(vf);
			if (!annot->has_sound())
			{
				for (auto &ext : Sound::common_sound_formats())
				{
					auto path = filesystem::strip_ext(annot->path());
					path.append('.').append(ext);
					if (filesystem::exists(path))
					{
						add_file(path, m_corpus, FileType::Any, false);
						auto sound = recast<Sound>(m_files[path]);
						// Mutate the annotation, so that its metadata are saved.
						annot->set_sound(sound, true);
					}
				}
			}
		}
	}
}

void Project::reinitialize()
{
	create_uuid();
	open_database();
	initialized();
}

void Project::add_bookmark(Handle<Bookmark> bookmark)
{
	bookmark->set_parent(m_bookmarks.get());
	m_bookmarks->append(std::move(bookmark));
	notify_update();
}

void Project::interpolate(String &path, std::string_view project_dir)
{
	if (path.starts_with(VAR_PROJECT)) {
		path.replace(1, VAR_PROJECT.size(), project_dir);
	}
	else if (path.starts_with(VAR_APPDIR)) {
		path.replace(1, VAR_APPDIR.size(), filesystem::application_directory());
	}
	else if (path.starts_with(VAR_HOME)) {
		path.replace(1, VAR_HOME.size(), filesystem::user_directory());
	}

	filesystem::nativize(path);
}

void Project::compress(String &path, std::string_view project_dir)
{
	if (path.starts_with(project_dir))
	{
		path.replace(1, project_dir.size(), VAR_PROJECT);
	}
	else
	{
		auto app_dir = filesystem::application_directory();

		if (path.starts_with(app_dir))
		{
			path.replace(1, app_dir.size(), VAR_APPDIR);
		}
		else
		{
			auto user_dir = filesystem::user_directory();

			if (path.starts_with(user_dir)) {
				path.replace(1, user_dir.size(), VAR_HOME);
			}
		}
	}

	filesystem::genericize(path);
}

void Project::preinitialize(Runtime &rt)
{
	auto elem_type = rt.add_standard_type<Element>("Element");
	rt.add_standard_type<Directory>("Directory", elem_type.get());
	auto doc_type = rt.add_standard_type<Document>("Document", elem_type.get());
	rt.add_standard_type<Annotation>("Annotation", doc_type.get());
	rt.add_standard_type<Sound>("Sound", doc_type.get());
	rt.add_standard_type<Dataset>("Dataset", doc_type.get());
	rt.add_standard_type<Spreadsheet>("Spreadsheet", doc_type.get());
	rt.add_standard_type<Concordance>("Concordance", doc_type.get());
	rt.add_standard_type<Script>("Script", doc_type.get());
	rt.add_standard_type<Query>("Query", doc_type.get());
	auto bookmakr_type = rt.add_standard_type<Bookmark>("Bookmark", elem_type.get());
	rt.add_standard_type<TimeStamp>("TimeStamp", bookmakr_type.get());
}

void Project::add_query(Handle<Query> query)
{
	m_queries->append(std::move(query), true);
}

Query::Type Project::get_query_type(const String &path)
{
	xml_document doc;
	xml_node root;

	try
	{
		root = read_xml(doc, path);
	}
	catch (...)
	{
		throw error("Cannot open text query \"%\"", path);
	}

	if (root.name() != std::string_view("Phonometrica")) {
		throw error("Invalid XML project root in %", path);
	}

	auto attr = root.attribute("class");

	if (!attr) {
		throw error("Missing class attribute in query file");
	}
	String klass = attr.value();

	if (klass == "Query")
		return Query::Type::Text;
	else if (klass == "FormantQuery")
		return Query::Type::Formant;
	else if (klass == "IntensityQuery")
		return Query::Type::Intensity;
	else if (klass == "Pitch")
		return Query::Type::Pitch;
	else if (klass == "DurationQuery")
		return Query::Type::Duration;
	else
		throw error("Invalid class attribute in query file: %", klass);
}

void Project::set_label(const String &value)
{
	m_label = value;
	m_modified = true;
}

} // namespace phonometrica
