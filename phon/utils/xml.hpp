/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
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
 * Purpose: XML processing.                                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_XML_HPP
#define PHONOMETRICA_XML_HPP

#include <phon/third_party/pugixml/pugixml.hpp>

namespace phonometrica {

using namespace pugi;

static inline
void add_data_node(xml_node root, std::string_view name, std::string_view data)
{
	auto node = root.append_child(name.data());
	auto data_node = node.append_child(node_pcdata);
	data_node.set_value(data.data());
}


static inline
xml_node read_xml(xml_document &doc, const String &path)
{
	xml_node root;

#if PHON_WINDOWS
	auto wpath = path.to_wide();
	auto result = doc.load_file(wpath.data());
#else
	auto result = doc.load_file(path.data());
#endif

	if (!result) {
		throw error(result.description());
	}

	do {
		root = doc.first_child();
	}
	while (root.type() == node_declaration || root.type() == node_doctype);

	return root;
}


static inline
void write_xml(const xml_document &doc, const String &path)
{
#if PHON_WINDOWS
	auto wpath = path.to_wide();
	doc.save_file(wpath.data());
#else
	doc.save_file(path.data());
#endif
}

} // namespace phonometrica

#endif // PHONOMETRICA_XML_HPP
