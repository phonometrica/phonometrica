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

#include <phon/application/metadata.hpp>
#include <phon/error.hpp>
#include <phon/utils/helpers.hpp>

namespace phonometrica {


Version::Version(const QDateTime &date, const String &description, const String &number) :
		m_date(date), m_description(description), m_number(number)
{

}

QDateTime Version::date() const
{
	return m_date;
}

void Version::set_date(const QDateTime &date)
{
	m_date = date;
}

String Version::number() const
{
	return m_number;
}

void Version::set_number(const String &number)
{
	m_number = number;
}

String Version::description() const
{
	return m_description;
}

void Version::set_description(const String &desc)
{
	m_description = desc;
}

const Array<Author> &Version::authors() const
{
	return m_authors;
}

void Version::add_author(Author author)
{
	m_authors.emplace_back(std::move(author));
}

void Version::remove_author(const String &name)
{
	for (size_t i = 0; i < m_authors.size(); i++)
	{
		if (m_authors[i].name == name)
		{
			m_authors.remove_at(i);
			return;
		}
	}
}

void Version::remove_author(size_t index)
{
	m_authors.remove_at(index);
}

void Version::to_xml(xml_node root)
{
	auto node = root.append_child("Version");
	auto date = date_as_string();

	add_data_node(node, "Description", m_description);
	add_data_node(node, "NumberString", m_number);
	add_data_node(node, "Date", date.data());

	auto authors_node = node.append_child("Authors");
	for (auto &author : m_authors) {
		author.ToXml(authors_node);
	}
}

String Version::date_as_string() const
{
	return m_date.toString("%d/%m/%Y");
}

QDateTime Version::parse_date(const String &date)
{
	return QDateTime::fromString(date, "%d/%m/%Y");
}

//----------------------------------------------------------------------------------------------------------------------



void Changelog::add_version(Version version, bool modify)
{
	m_versions.emplace_back(std::move(version));
	mark_modified(modify);
}

Version &Changelog::get_version(size_t index)
{
	return m_versions[index];
}

size_t Changelog::version_count() const
{
	return m_versions.size();
}

void Changelog::remove_version(size_t index, bool modify)
{
	m_versions.remove_at(index);
	mark_modified(modify);
}

void Changelog::from_xml(xml_node root)
{
	static const std::string_view version_tag("Version");
	static const std::string_view author_tag("Author");

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() != version_tag) {
			throw error("Unexpected Changelog XML node");
		}

		auto desc_node = node.child("Description");
		auto date_node = node.child("Date");
		auto num_node = node.child("NumberString");
		auto authors_node = node.child("Authors");

		if (!desc_node || !date_node || !num_node || !authors_node) {
			throw error("Invalid version in changelog");
		}

		String description(desc_node.text().get());
		QDateTime date = Version::parse_date(date_node.text().get());
		String number(num_node.text().get());

		Version version(date, description, number);

		for (auto author_node = authors_node.first_child(); author_node; author_node = author_node.next_sibling())
		{
			if (author_node.name() != author_tag) {
				throw error("Unexpected XML node in author list");
			}

			auto name_node = author_node.child("Name");
			auto email_node = author_node.child("Email");

			if (!name_node || !email_node) {
				throw error("Invalid author XML node");
			}

			String name(name_node.text().get());
			String email(email_node.text().get());

			version.add_author({std::move(name), std::move(email)});
		}

		m_versions.emplace_back(std::move(version));
	}
}

void Changelog::to_xml(xml_node root)
{
	auto node = root.append_child("Changelog");

	for (auto &version : m_versions) {
		version.to_xml(node);
	}
}

bool Changelog::modified() const
{
	return m_modified;
}

void Changelog::mark_modified(bool value)
{
	m_modified |= value;
}

void Changelog::reset_modifications()
{
	m_modified = false;
}

void Author::ToXml(xml_node root)
{
	auto node = root.append_child("Author");
	add_data_node(node, "Name", this->name);
	add_data_node(node, "Email", this->email);
}

} // phonometrica