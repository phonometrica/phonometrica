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
 * Purpose: project metadata.                                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_METADATA_HPP
#define PHONOMETRICA_METADATA_HPP

#include <QDateTime>
#include <phon/string.hpp>
#include <phon/utils/xml.hpp>

namespace phonometrica {

struct Author
{
	String name;
	String email;

	void ToXml(xml_node root);
};

//----------------------------------------------------------------------------------------------------------------------

class Version
{
public:

	Version(const QDateTime &date, const String &description, const String &number = String());

	QDateTime date() const;
	String date_as_string() const;
	static QDateTime parse_date(const String &date);
	void set_date(const QDateTime &date);

	String number() const;
	void set_number(const String &number);

	String description() const;
	void set_description(const String &desc);

	const Array<Author> &authors() const;
	void add_author(Author author);
	void remove_author(const String &name);
	void remove_author(size_t index);

	void to_xml(xml_node root);

private:

	Array<Author> m_authors;

	QDateTime m_date;

	String m_description;

	String m_number;
};


//----------------------------------------------------------------------------------------------------------------------

class Changelog
{
public:
	Changelog() = default;

	bool modified() const;
	void reset_modifications();

	size_t version_count() const;
	Version &get_version(size_t index);
	void add_version(Version version, bool modify = true);
	void remove_version(size_t index, bool modify = true);

	void from_xml(xml_node root);
	void to_xml(xml_node root);

private:

	void mark_modified(bool value);

	Array<Version> m_versions;

	bool m_modified = false;
};

} // phonometrica

#endif // PHONOMETRICA_METADATA_HPP
