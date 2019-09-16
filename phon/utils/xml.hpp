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
