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
 * Purpose: store metadata for files which can't store their own metadata (e.g. TextGrid).                            *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_DATABASE_HPP
#define PHONOMETRICA_DATABASE_HPP

#include <QObject>
#include <phon/string.hpp>
#include <phon/third_party/sqlite/sqlite3.h>
#include <phon/utils/memory.hpp>
#include <set>

namespace phonometrica {

class VFile;
class Annotation;
class Property;


class Database : public QObject
{
    Q_OBJECT
    
public:

	explicit Database(const String &path);

	~Database() override;

	void execute(std::string_view sql, bool notify = true);

	void parse(const String &sql);

	/* Read next row from the statement. Returns whether a row was successfully read */
	bool read_row();

	/* Get a field in the current row */
	inline String get_field(int j);

	String get_column_name(int j) const;

	int field_count() const;

	void finalize_statement();

	bool has_column(std::string_view col);

	void close();

	/* Check whether the query returned any rows and clean up */
	bool check_statement();

	void commit();

	String path() const { return m_path; }

protected:

	/* Database connection */
	sqlite3 *db;

	/* Current statement, if any */
	sqlite3_stmt *statement;

	String m_path;

	String escape_string(const String &str);
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class MetaDatabase final : public Database
{
    Q_OBJECT

public:

	MetaDatabase(const String &path, bool create_table);

	void create_main_table();

	bool has_file(const String &path);

	void add_category(const String &cat);

	void remove_category_from_file(const String &path, const String &cat);

	void add_file(VFile &file);

	void save_file_metadata(VFile &file);

	void add_metadata_to_file(std::shared_ptr<VFile> &file);

	std::set<String> get_categories();
	
signals:

	void notify_property(const Property &);

	// Send annotation and sound path. The project will associate the Annotation with
	// the corresponding Sound file whenever possible.
	void notify_annotation_needs_sound(const std::shared_ptr<Annotation> &, const String&);

private:

	String get_value(const String &path, const String &cat);

	String get_sound_path_if_exists(const VFile &file) const;
};


} // phonometrica
#endif // PHONOMETRICA_DATABASE_HPP
