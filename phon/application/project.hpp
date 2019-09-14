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
 * Purpose: a project stores a collection of files and metadata. There is only one project open at any given time.    *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_PROJECT_HPP
#define PHONOMETRICA_PROJECT_HPP

#include <QObject>
#include <phon/application/sound.hpp>
#include <phon/application/annotation.hpp>
#include <phon/application/document.hpp>
#include <phon/application/script.hpp>
#include <phon/application/bookmark.hpp>
#include <phon/application/spreadsheet.hpp>
#include <phon/application/metadata.hpp>
#include <phon/application/database.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/error.hpp>
#include <phon/dictionary.hpp>

namespace phonometrica {

class Project final : public QObject
{
	Q_OBJECT

public:

	// This must not be called directly. It is only public for the benefit of std::make_unique<Project>().
	Project(Runtime &rt, String path);

	~Project();

	static void create(Runtime &rt);

	static void initialize(Runtime &rt);

	static Project* instance();

	Project(const Project &) = delete;
	Project(Project &&) = delete;


	void open(String path);

	static void close();

	bool modified() const;

	void modify();

	const String &path() const;

	void set_path(String value);

	bool has_path() const;

	void save();

	void save(String path);

	const std::shared_ptr<VFolder> & corpus() const;

	const std::shared_ptr<VFolder> & bookmarks() const;

	const std::shared_ptr<VFolder> & scripts() const;

	const std::shared_ptr<VFolder> & data() const;

	const std::shared_ptr<VFolder> & queries() const;

	const String &directory() const;

	void import_folder(String path);

	void import_file(String path);

	void remove(VFileList &files);

	void remove(std::shared_ptr<VFile> &folder);

	void remove(std::shared_ptr<VFolder> &folder);

	bool has_uuid() const;

	const String &uuid() const;

	static std::set<Property> get_shared_properties(const VFileList &files);

	MetaDatabase & database() const;

	void remove_empty_script();

	VFileList get_corpus_files() const;

	void clear();

	String label() const;

	std::shared_ptr<VFile> get(const String &path);

    void register_file(const String &path, std::shared_ptr<VFile> file);

    static void updated();

    bool is_root(const std::shared_ptr<VFolder> &folder) const;

    void import_metadata(const String &path);

    bool empty() const;

    Array<AutoAnnotation> annotations() const;

public slots:

	void bind_annotation(const AutoAnnotation &annot, const String &sound_file);

signals:

	// Start import progress dialog.
	void start_import(const String&, int);

	// Update import counter.
	void update_import(int);

	// Inform the UI that the project has been modified.
	void notify_update();

	// Infor the UI that the project has been closed.
	void notify_closed();

	// Send when only the metadata has been changed.
	void metadata_updated();

	// Inform the UI that an error occurred.
	void notify_error(const String&);

	void request_save();

private:

    void load();
	void write();

	void parse_corpus(xml_node root, VFolder *folder, bool emitting = false);
	void parse_metadata(xml_node root);
	void parse_scripts(xml_node root, VFolder *folder);
	void parse_bookmarks(xml_node root, VFolder *folder);
	void parse_changelog(xml_node root);
	void parse_data(xml_node root, VFolder *folder);

	void write_corpus(xml_node root);
	void write_metadata(xml_node root);
	void write_bookmarks(xml_node root);
	void write_scripts(xml_node root);
	void write_data(xml_node root);

	bool add_file(String path, const std::shared_ptr<VFolder> &parent);
	void add_folder(String path, const std::shared_ptr<VFolder> &parent);

	void bind_annotations();

	void open_database();

	void create_uuid();

	void discard_database();

	void reinitialize();

	const std::shared_ptr<VFile> & get_file_handle(const String &path, std::string_view msg);

	void trigger(const String &event, const char *tag, std::any value);

    void trigger(const String &event);

    void tag_file(std::shared_ptr<VFile> &file, const String &category, const String &value);

    void set_default_bindings();

    static std::unique_ptr<Project> the_instance;

	Runtime &rt;

	// Path on disk.
	String m_path;

	// Base directory for the project.
	String m_directory;

	// Map all registered file paths to a file object. This ensures that we don't create several
	// objects for the same file.
	Dictionary<std::shared_ptr<VFile>> m_files;

	// When loading a project, an annotation may be loaded before the sound file it is bound to,
	// so binding can't happen at that time. Instead, annotations are stored into this accumulator
	// and binding happens once the project is loaded.
	std::vector<std::pair<Annotation*,String>> m_accumulator;

	std::shared_ptr<VFolder> m_corpus;

	std::shared_ptr<VFolder> m_bookmarks;

	std::shared_ptr<VFolder> m_scripts;

	std::shared_ptr<VFolder> m_data;

	std::shared_ptr<VFolder> m_queries;

	// UUID for the project, so that it can be uniquely identified in the metadata database.
	String m_uuid;

	// Project metadata.
	Changelog m_changelog;

	std::unique_ptr<MetaDatabase> m_database;

	// Flag which checks if the database already existed when it was opened. If the project is not saved, then we discard the database.
	bool m_database_temp = true;

	// Register modifications.
	bool m_modified = false;

};

} // namespace phonometrica

#endif // PHONOMETRICA_PROJECT_HPP
