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
 * Purpose: project metadata.                                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
