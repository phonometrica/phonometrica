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

#include <algorithm>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/bookmark.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/utils/text.hpp>
#include <phon/runtime/object.hpp>

namespace phonometrica {

std::unique_ptr<Project> Project::the_instance;

static String event_module_name("event");
static String emit_name("emit");
static String annotation_loaded("__evt_annotation_loaded");
static String sound_loaded("__evt_sound_loaded");
static String document_loaded("__evt_document_loaded");
static String script_loaded("__evt_script_loaded");
static String dataset_loaded("__evt_dataset_loaded");
static String project_loaded("__evt_project_loaded");

void Project::open(String path)
{
    close();
    set_path(std::move(path));
    load();
}

Project::Project(Runtime &rt, String path) :
		rt(rt),
		m_corpus(std::make_shared<VFolder>(nullptr, "Corpus")),
		m_bookmarks(std::make_shared<VFolder>(nullptr, "Bookmarks")),
		m_scripts(std::make_shared<VFolder>(nullptr, "Scripts")),
		m_data(std::make_shared<VFolder>(nullptr, "Data")),
		m_queries(std::make_shared<VFolder>(nullptr, "Queries"))
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

	emit initialized();
}

Project::~Project()
{
	discard_database();
}

Project *Project::instance()
{
	return the_instance.get();
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
	// Inform the views so that they can save unsaved work.
	emit request_save();

	// Save content before writing the project, because write() will reset modifications on all VFiles.
	m_corpus->save_content();
	m_scripts->save_content();
	m_data->save_content();
	m_queries->save_content();

	write();

	m_modified = false;
	// Once the project has been written to disk, its database must not be discarded.
	m_database_temp = false;

	emit notify_update();
}

void Project::load()
{
	assert(!m_path.empty());
	assert(m_accumulator.empty());

	xml_document doc;
	xml_node root = read_xml(doc, m_path);

	static const std::string_view project_tag("Phonometrica");
	static const std::string_view class_tag("Project");
	static const std::string_view corpus_tag("Corpus");
	static const std::string_view meta_tag("Metadata");
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

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == corpus_tag)
		{
			int size = 0;
			auto attr = node.attribute("size");
			if (attr)
			{
				size = attr.as_int();
				emit start_import("Importing files...", size);
			}

			parse_corpus(node, m_corpus.get(), true);
		}
		else if (node.name() == meta_tag)
		{
			parse_metadata(node);
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
	emit notify_update();
	trigger(project_loaded);
}

const std::shared_ptr<VFolder> & Project::corpus() const
{
	return m_corpus;
}

const std::shared_ptr<VFolder> & Project::bookmarks() const
{
	return m_bookmarks;
}

const std::shared_ptr<VFolder> & Project::scripts() const
{
	return m_scripts;
}

void Project::parse_corpus(xml_node root, VFolder *folder, bool emitting)
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
			auto subfolder = std::make_shared<VFolder>(folder, std::move(label));
			auto sub = subfolder.get();
			folder->append(std::move(subfolder), false);

			parse_corpus(node, sub);
		}
		else if (node.name() == file_tag)
		{
			auto attr = node.attribute("class");
			if (!attr) throw error("Invalid VFile node");
			std::string_view cls(attr.value());
			std::shared_ptr<VFile> vfile;
			String path(node.text().get());
			filesystem::interpolate(path, m_directory);

			if (cls == "Annotation")
			{
				auto annot = std::make_shared<Annotation>(folder, std::move(path));
				vfile = upcast<VFile>(annot);
                trigger(annotation_loaded, annot->class_name(), annot);
			}
			else if (cls == "Sound")
			{
				auto sound = std::make_shared<Sound>(folder, std::move(path));
				vfile = upcast<VFile>(sound);
				trigger(sound_loaded, sound->class_name(), sound);
			}
			else if (cls == "Document")
			{
				auto doc = std::make_shared<Document>(folder, std::move(path));
				vfile = upcast<VFile>(doc);
				trigger(document_loaded, doc->class_name(), doc);
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

		if (emitting) {
			emit update_import(++counter);
		}
	}
}

void Project::register_file(const String &path, std::shared_ptr<VFile> file)
{
	m_database->add_metadata_to_file(file);

	if (m_files.find(path) == m_files.end())
	{
		m_files.insert(path, std::move(file));
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

void Project::parse_scripts(xml_node root, VFolder *folder)
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
			auto subfolder = std::make_shared<VFolder>(folder, std::move(label));
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
			filesystem::interpolate(path, m_directory);

			if (cls == script_tag)
			{
				auto script = std::make_shared<Script>(folder, std::move(path));
				script->from_xml(node, m_directory);
				folder->append(script, false);
				register_file(script->path(), script);
				trigger(script_loaded, script->class_name(), script);
			}
			else
			{
				throw error("Invalid class \"%\" in VFile XML entry", node.name());
			}
		}
	}
}

void Project::parse_data(xml_node root, VFolder *folder)
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
			auto subfolder = std::make_shared<VFolder>(folder, std::move(label));
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
			filesystem::interpolate(path, m_directory);

			if (cls == data_tag)
			{
				auto dataset = std::make_shared<Spreadsheet>(folder, std::move(path));
				dataset->from_xml(node, m_directory);
				folder->append(dataset, false);
				register_file(dataset->path(), dataset);
				trigger(dataset_loaded, dataset->class_name(), dataset);
			}
			else
			{
				throw error("Invalid class \"%\" in VFile XML entry", node.name());
			}
		}
	}
}

void Project::parse_bookmarks(xml_node root, VFolder *folder)
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
			auto subfolder = std::make_shared<VFolder>(folder, std::move(label));
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

				filesystem::interpolate(path, directory());
				auto file = get_file_handle(path, "needed by bookmark");

				if (file->is_annotation())
				{
					auto annot = downcast<Annotation>(file);
					auto bookmark = std::make_shared<AnnotationStamp>(folder, title, std::move(annot), layer, start, end, match, left, right);
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

const std::shared_ptr<VFile> & Project::get_file_handle(const String &path, std::string_view msg)
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

void Project::write()
{
	assert(has_path());
	xml_document doc;
	auto root = doc.append_child("Phonometrica");
	auto attr = root.append_attribute("class");
    attr.set_value("Project");

	// Metadata must come first, so that we can get the metadata file's UUID.
	auto meta_node = root.append_child("Metadata");
	write_metadata(meta_node);

	auto fs_node = root.append_child("Corpus");
	write_corpus(fs_node);

	auto scripts_node = root.append_child("Scripts");
	write_scripts(scripts_node);

	auto data_node = root.append_child("Data");
	write_data(data_node);

	auto bookmarks_node = root.append_child("Bookmarks");
	write_bookmarks(bookmarks_node);

	write_xml(doc, m_path);
}

bool Project::add_file(String path, const std::shared_ptr<VFolder> &parent)
{
	if (m_files.find(path) != m_files.end()) {
		return false;
	}

	auto ext = filesystem::ext(path, true);

	std::shared_ptr<VFile> vfile;

	if (ext == PHON_EXT_ANNOTATION || ext == ".textgrid")
	{
	    auto annot = std::make_shared<Annotation>(parent.get(), std::move(path));
		vfile = upcast<VFile>(annot);
		parent->append(vfile);
        trigger(annotation_loaded, annot->class_name(), annot);
	}
	else if (Sound::supports_format(ext))
	{
	    auto sound = std::make_shared<Sound>(parent.get(), std::move(path));
		vfile = upcast<VFile>(sound);
		parent->append(vfile);
//		trigger(sound_loaded, sound->class_name(), sound);
	}
	else if (ext == ".csv")
	{
		// Datasets are added to the data folder.
		auto dataset = std::make_shared<Spreadsheet>(m_data.get(), std::move(path));
		vfile = upcast<VFile>(dataset);
		m_data->append(vfile);
//		trigger(dataset_loaded, dataset->class_name(), dataset);
	}
	else if (ext == ".phon" || ext == ".phon-script")
	{
		// Scripts are not added to the parent folder, but to the scripts folder.
        auto script = std::make_shared<Script>(m_scripts.get(), std::move(path));
		vfile = upcast<VFile>(script);
		m_scripts->append(vfile);
//		trigger(script_loaded, dataset->class_name(), script);
	}
//	else if (ext == ".txt")
//	{
//	    auto doc = std::make_shared<Document>(parent.get(), std::move(path));
//		vfile = upcast<VFile>(doc);
//		parent->append(vfile);
////		trigger(document_loaded, doc->class_name(), doc);
//	}
	else
	{
		return false; // Ignore unknown files
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

void Project::import_folder(String path)
{
	filesystem::nativize(path);
	add_folder(std::move(path), m_corpus);
	bind_annotations();
	// Try to find a sound that matches the annotation's name.
	set_default_bindings();
	m_modified = true;
}

String Project::import_file(String path)
{
	filesystem::interpolate(path, directory());
	filesystem::nativize(path);

	// Assume that the file will be added to the corpus. add_file() will change the parent if necessary.
	if (add_file(path, m_corpus)) {
        m_modified = true;
	}
	// Try to find a sound that matches the annotation's name.
	set_default_bindings();

	return path;
}

void Project::add_folder(String path, const std::shared_ptr<VFolder> &parent)
{
	auto content = filesystem::list_directory(path);
	bool toplevel = (parent == m_corpus);
	auto count = content.size();

	if (toplevel) {
		emit start_import("Import folder", count);
	}

	for (size_t counter = 1; counter <= count; ++counter)
	{
		auto &name = content[counter];
		auto file = filesystem::join(path, name);

		if (filesystem::is_directory(file))
		{
			auto subfolder = std::make_shared<VFolder>(parent.get(), name);
			parent->append(subfolder);
			add_folder(file, subfolder);
		}
		else
		{
			add_file(file, parent);
		}

		if (toplevel) {
			emit update_import(counter);
		}
	}
}

std::set<Property> Project::get_shared_properties(const VFileList &files)
{
	std::set<Property> properties;

	std::set<Property> temp_result;
	properties = files.front()->properties();
	auto count = files.size();

	for (size_t i = 1; i < count; ++i)
	{
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
	connect(m_database.get(), &MetaDatabase::notify_annotation_needs_sound, this, &Project::bind_annotation);
}

void Project::remove(VFileList &files)
{
	for (auto &file : files)
	{
        remove(file);
	}
}

const std::shared_ptr<VFolder> & Project::data() const
{
	return m_data;
}

const std::shared_ptr<VFolder> &Project::queries() const
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

void Project::bind_annotation(const AutoAnnotation &annot, const String &sound_file)
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
			auto snd = downcast<Sound>(it->second);
			// This method is only called when a project is loading, so we don't mutate the annotation
			annot->set_sound(snd, false);
		}
		else
		{
			auto msg = utils::format("Cannot bind annotation \"%\" to sound \"%\" because the sound is not part of the project",
					annot->path(), sound_path);
			emit notify_error(msg);
		}
	}

	m_accumulator.clear();
}

void Project::close()
{
	the_instance->clear();
	emit the_instance->notify_update();
	emit the_instance->notify_closed();
}

VFileList Project::get_corpus_files() const
{
	VFileList files;
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
			auto script = raw_cast<Script>(node);

			if (!script->has_path()) {
				m_scripts->remove(node);
			}
		}
	}
}

void Project::initialize(Runtime &rt)
{
	auto open_project = [](Runtime &rt)
	{
		auto path = rt.to_string(1);
		Project::instance()->open(path);
        rt.push_null();
	};

	auto close_project = [](Runtime &rt)
	{
		Project::instance()->close();
		Project::instance()->reinitialize();
		rt.push_null();
	};

	auto add_folder = [](Runtime &rt)
    {
	    auto path = rt.to_string(1);
	    Project::instance()->import_folder(path);
	    rt.push_null();
    };

    auto add_file = [](Runtime &rt)
    {
        auto path = rt.to_string(1);
        Project::instance()->import_file(path);
        rt.push_null();
    };

    auto refresh_project = [](Runtime &rt)
	{
    	Project::updated();
    	rt.push_null();
	};

    auto project_has_path = [](Runtime &rt) {
        rt.push_boolean(Project::instance()->has_path());
    };

    auto save_project = [](Runtime &rt) {
        if (rt.arg_count() > 0)
        {
            auto path = rt.to_string(1);
            Project::instance()->save(std::move(path));
        }
        else
        {
            Project::instance()->save();
        }
        rt.push_null();
    };

    auto get_annotations = [](Runtime &rt) {
    	Array<Variant> result;
    	auto &files = Project::instance()->m_files;
    	for (auto &pair : files)
		{
    		if (pair.second->is_annotation())
			{
    		    rt.new_user_data(Annotation::meta(), "Annotation", downcast<Annotation>(pair.second));
    			result.append(std::move(rt.get(-1)));
			}
		}

    	rt.push(std::move(result));
    };

    auto get_annotation = [](Runtime &rt) {
	    auto &files = Project::instance()->m_files;
	    auto path = rt.to_string(1);
	    auto file = files.find(path);

	    if (file == files.end())
	    {
		    rt.push_null();
	    }
	    else if (file->second->is_annotation())
	    {
			rt.new_user_data(Annotation::meta(), "Annotation", downcast<Annotation>(file->second));
	    }
	    else
	    {
		    throw error("File \"%\" is not an annotation", path);
	    }
    };

    auto get_sounds = [](Runtime &rt) {
    	Array<Variant> result;
    	auto &files = Project::instance()->m_files;
    	for (auto &pair : files)
		{
    		if (pair.second->is_sound())
			{
    		    rt.new_user_data(Sound::meta(), "Sound", downcast<Sound>(pair.second));
    			result.append(std::move(rt.get(-1)));
			}
		}

    	rt.push(std::move(result));
    };

    auto get_sound = [](Runtime &rt) {
    	auto &files = Project::instance()->m_files;
    	auto path = rt.to_string(1);
		auto file = files.find(path);

		if (file == files.end())
		{
			rt.push_null();
		}
		else if (file->second->is_sound())
		{
			rt.new_user_data(Sound::meta(), "Sound", downcast<Sound>(file->second));
		}
		else
		{
			throw error("File \"%\" is not a sound", path);
		}
    };

    auto is_empty = [](Runtime &rt) {
        rt.push_boolean(Project::instance()->empty());
    };

	rt.get_global("phon");
	{
		rt.add_global_function("get_annotations", get_annotations, 0);
		rt.add_global_function("get_annotation", get_annotation, 1);
		rt.add_global_function("get_sounds", get_sounds, 0);
		rt.add_global_function("get_sound", get_sound, 1);

        rt.push(new Object(rt, PHON_COBJECT, rt.object_meta));
        {
            rt.add_method("open", open_project, 1);
            rt.add_method("close", close_project, 0);
            rt.add_method("add_folder", add_folder, 1);
            rt.add_method("add_file", add_file, 1);
            rt.add_method("refresh", refresh_project, 0);
            rt.add_method("has_path", project_has_path, 0);
            rt.add_method("save", save_project, 0);
            rt.add_method("is_empty", is_empty, 0);
        }
        rt.set_field(-2, "project");
	}
	rt.pop();

    Annotation::initialize(rt);
    Sound::initialize(rt);
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

    if (m_path.empty())
        return "Untitled project";

    String label("Project ");
    label.append(base_name(strip_ext(path())));

    return label;
}

std::shared_ptr<VFile> Project::get(const String &path)
{
    auto it = m_files.find(path);
    if (it != m_files.end())
    	return it->second;

    return nullptr;
}

void Project::modify()
{
	m_modified = true;
}

void Project::create(Runtime &rt)
{
    the_instance = std::make_unique<Project>(rt, String());
}

void Project::updated()
{
    emit instance()->notify_update();
}

void Project::remove(std::shared_ptr<VFolder> &folder)
{
    for (intptr_t i = folder->size(); i > 0; i--)
    {
    	auto &node = folder->get(i);
        if (node->is_file())
        {
            auto file = downcast<VFile>(node);
            remove(file);
        }
        else if (node->is_folder())
        {
            auto subfolder = downcast<VFolder>(node);
            remove(subfolder);
        }
        else
        {
            throw std::runtime_error("[Internal error] Unhandled case in Project::remove()");
        }
    }
    folder->detach();
}

void Project::remove(std::shared_ptr<VFile> &file)
{
    file->detach();
    m_files.remove(file->path());
    m_modified = true;
}

bool Project::is_root(const std::shared_ptr<VFolder> &folder) const
{
    return folder == m_corpus || folder == m_scripts || folder == m_data || folder == m_bookmarks;
}

void Project::trigger(const String &event, const char *tag, std::any value)
{
    rt.get_global(event_module_name);
    rt.get_field(-1, emit_name);
    rt.push_null();
    rt.push(event);
    rt.new_user_data(Annotation::meta(), tag, std::move(value));
    rt.call(2);
    rt.pop();
}

void Project::trigger(const String &event)
{
    rt.get_global(event_module_name);
    rt.get_field(-1, emit_name);
    rt.push_null();
    rt.push(event);
    rt.call(1);
    rt.pop();
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
	emit metadata_updated();
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

void Project::tag_file(std::shared_ptr<VFile> &file, const String &category, const String &value)
{

	if (category == "%SOUND%")
	{
		if (filesystem::exists(value) && file->is_annotation())
		{
			auto annot = downcast<Annotation>(file);
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

Array<AutoAnnotation> Project::annotations() const
{
	Array<AutoAnnotation> result;

	for (auto &item : m_files)
	{
		auto &vf = item.second;

		if (vf->is_annotation())
		{
			result.push_back(std::dynamic_pointer_cast<Annotation>(vf));
		}
	}

	return result;
}

void Project::set_default_bindings()
{
	for (auto &item : m_files)
	{
		auto &vf = item.second;

		if (vf->is_annotation())
		{
			auto annot = std::dynamic_pointer_cast<Annotation>(vf);
			if (!annot->has_sound())
			{
				for (auto &ext : Sound::common_sound_formats())
				{
					auto path = filesystem::strip_ext(annot->path());
					path.append('.').append(ext);
					if (filesystem::exists(path))
					{
						add_file(path, m_corpus);
						auto sound = std::dynamic_pointer_cast<Sound>(m_files[path]);
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
	emit initialized();
}

void Project::add_bookmark(AutoBookmark bookmark)
{
	bookmark->set_parent(m_bookmarks.get());
	m_bookmarks->append(std::move(bookmark));
	emit notify_update();
}

} // namespace phonometrica
