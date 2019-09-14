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

#include <cmath>
#include <phon/application/database.hpp>
#include <phon/application/sound.hpp>
#include <phon/application/annotation.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {


Database::Database(const String &path) :
	m_path(path)
{
	int result = sqlite3_open(path.data(), &db);

	if (result != SQLITE_OK)
	{
		String msg("Could not create the SQLite database ");
		msg.append(path);

		throw std::runtime_error(msg.data());
	}

	statement = nullptr;
}

Database::~Database()
{
	close();
}

void Database::execute(std::string_view sql, bool notify)
{
	char *error;
	if ((sqlite3_exec(db, sql.data(), nullptr, nullptr, &error) != SQLITE_OK) && notify)
	{
		auto msg = utils::format("[SQL error] An error occurred in sqlite3_exec() with the following query: %\nError message: %",
				sql, error);
		throw std::runtime_error(msg);
	}
}

void Database::parse(const String &sql)
{
	// According to SQLite's doc, passing the size of the string (including the NULL terminator)
	// avoids making a copy of the string
	int status = sqlite3_prepare_v2(db, sql.data(), sql.size() + 1, &statement, nullptr);

	if (status != SQLITE_OK)
	{
		auto msg = utils::format("[SQL error] An error occurred in sqlite3_prepare() with the following query: %", sql);
		throw std::runtime_error(msg);
	}
}

bool Database::read_row()
{
	if (statement)
	{
		int status = sqlite3_step(statement);

		if (status == SQLITE_ROW)
		{
			return true;
		}
		else if (status == SQLITE_DONE)
		{
			sqlite3_finalize(statement);
			statement = nullptr;
		}
	}

	return false;
}

String Database::get_field(int j)
{
	char *str = (char *) sqlite3_column_text(statement, j);

	if (str)
	{ return str; }
	else
	{ return String(); }
}

String Database::get_column_name(int j) const
{
	return sqlite3_column_name(statement, j);
}

int Database::field_count() const
{
	return sqlite3_column_count(statement);
}

void Database::finalize_statement()
{
	if (statement) {
		sqlite3_finalize(statement);
		statement = nullptr;
	}
}

void Database::close()
{
	finalize_statement();
	if (db) sqlite3_close(db);
	db = nullptr;
}

bool Database::check_statement()
{
	bool status = read_row();
	sqlite3_finalize(statement);
	statement = nullptr;

	return status;
}

void Database::commit()
{
	execute("COMMIT;");
}

String Database::escape_string(const String &str)
{
	String result("\"");
	result.append(str);
	result.append("\"");

	return result;
}

bool Database::has_column(std::string_view col)
{
    auto result = sqlite3_table_column_metadata(db, nullptr, "files", col.data(),
                                                nullptr, nullptr, nullptr, nullptr, nullptr);
    return result == SQLITE_OK;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MetaDatabase::MetaDatabase(const String &path, bool create_table) : Database(path)
{
	if (create_table)
	{
		create_main_table();
	}
}

void MetaDatabase::create_main_table()
{
	// Internal fields start with an underscore
	execute(R"__(CREATE TABLE files (_path TEXT PRIMARY KEY NOT NULL DEFAULT "",
_soundref TEXT NOT NULL DEFAULT "",
_type TEXT NOT NULL DEFAULT "",
_description TEXT NOT NULL DEFAULT "");)__");
}

void MetaDatabase::add_file(VFile &file)
{
	String type = file.class_name();
	auto &path = file.path();
	String sound_ref = get_sound_path_if_exists(file);

	Array<String> columns, values;

	columns.append("_path");
	columns.append("_soundref");
	columns.append("_type");
	columns.append("_description");

	values.append(escape_string(file.path()));
	values.append(escape_string(sound_ref));
	values.append(escape_string(type));
	values.append(escape_string(file.description()));

	for (const Property &p: file.properties())
	{
		auto category = p.category();

		add_category(category);
		columns.append(escape_string(category));
		values.append(escape_string(p.value()));
	}

	String col_stmt = String::join(columns, ", ");
	String val_stmt = String::join(values, ", ");

	auto query = utils::format("INSERT INTO files (%) VALUES (%)", col_stmt, val_stmt);
	execute(query);
}

void MetaDatabase::save_file_metadata(VFile &file)
{
	auto &path = file.path();
	String msg("Saving metadata for file ");
	msg.append(path);
	emit saving_metadata(msg);

	if (has_file(path))
	{
		Array<String> columns, values;

		columns.append("_description");
		columns.append("_soundref");

		values.append(file.description());
		values.append(get_sound_path_if_exists(file));

		// Update all properties: existing properties are overwritten
		// (even if they haven't changed). Other properties are emptied.
		auto categories = Property::get_categories();

		for (auto &cat : categories) {
            add_category(cat);
		}

		for (auto &p: file.properties())
		{
			columns.append(p.category());
			values.append(p.value());
			categories.erase(p.category());
		}

		// Make sure absent properties are empty
		for (auto &cat : categories)
		{
			columns.append(cat);
			values.append(String());
		}

		String col, val;
		Array<String> stmts;

		for (size_t i = 1; i <= values.size(); i++)
		{
			col = columns[i];
			val = values[i];
            auto stmt = escape_string(col);
            stmt.append(" = ");
            stmt.append(escape_string(val));
            stmts.append(std::move(stmt));
		}

		String updates = String::join(stmts, ", ");
		auto query = utils::format("UPDATE files SET % where _path = \"%\";",
		                           updates, path);
		execute(query);
	}
	else
	{
		add_file(file);
	}
}

void MetaDatabase::add_metadata_to_file(std::shared_ptr<VFile> &file)
{
	String sql("SELECT * FROM files WHERE _path = ");
	sql.append(escape_string(file->path())).append(';');

	parse(sql);

	if (read_row())
	{
		int count = field_count();

		for (int i = 0; i < count; ++i)
		{
			String name(get_column_name(i));
			String field(get_field(i));
			if (name == "_description")
			{
				file->set_description(field, false);
			}
			else if (name == "_soundref" && (!field.empty()) && file->is_annotation())
			{
				auto annot = downcast<Annotation>(file);
				emit notify_annotation_needs_sound(annot, field);
			}
			else if (!name.starts_with("_") && !field.empty())
			{
				Property p;
				bool ok = false;
				double num = std::nan("");

				// Boolean
				if (field == Property::true_string())
				{
					p = Property(name, true);
				}
				if (field == Property::false_string())
				{
					p = Property(name, false);
				}
				// Numeric
				else if (field == Property::undefined_string())
				{
					p = Property(name, std::nan(""));
				}
				else if ((num = field.to_float(&ok)) != std::nan("") && ok)
				{
					p = Property(name, num);
				}
				// Text
				else
				{
					p = Property(name, field);
				}
				file->add_property(p, false);
			}
		}
	}

	finalize_statement();
}

std::set<String> MetaDatabase::get_categories()
{
	std::set<String> categories;

	parse("SELECT _path FROM files WHERE rowid = 1;");

	if (read_row())
	{
		int count = field_count();

		for (int i = 0; i < count; ++i)
		{
			String value(get_column_name(i));

			if (!value.starts_with("_"))
			{
				categories.insert(value);
			}
		}
	}
	finalize_statement();

	return categories;
}

void MetaDatabase::add_category(const String &cat)
{
    if (!has_column(cat))
    {
        String sql("ALTER TABLE files ADD COLUMN \"");
        sql.append(cat)
                .append("\" TEXT NOT NULL DEFAULT \"\";");

        execute(sql, false);
    }
}

void MetaDatabase::remove_category_from_file(const String &path, const String &cat)
{
	String sql("UPDATE files SET ");

	sql.append(cat)
			.append("= \"\" WHERE _path = ")
			.append(escape_string(path))
			.append(';');

	execute(sql);
}

String MetaDatabase::get_value(const String &path, const String &cat)
{
	String sql("SELECT ");
	String value;

	sql.append(escape_string(cat))
			.append(" FROM files WHERE _path = ")
			.append(escape_string(path))
			.append(';');
	parse(sql);

	if (read_row())
	{ value = get_field(0); }
	finalize_statement();

	return value;
}

bool MetaDatabase::has_file(const String &path)
{
	auto sql = utils::format("SELECT * FROM files WHERE _path = \"%\";", path);
	parse(sql);

	return check_statement();
}

String MetaDatabase::get_sound_path_if_exists(const VFile &file) const
{
	if (auto annot = dynamic_cast<const Annotation*>(&file))
	{
		if (annot->has_sound())
		{
			return annot->sound()->path();
		}
	}

	return String();
}

} // phonometrica