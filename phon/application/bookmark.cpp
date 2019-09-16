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

#include <phon/application/bookmark.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Bookmark::Bookmark(VFolder *parent) :
		VNode(parent)
{

}


Bookmark::Bookmark(VFolder *parent, String title) :
		VNode(parent), m_title(std::move(title))
{

}

String Bookmark::label() const
{
	return m_title;
}

bool Bookmark::is_bookmark() const
{
	return true;
}

AnnotationStamp::AnnotationStamp(VFolder *parent, String title, std::shared_ptr<VFile> file, size_t layer,
								 double start, double end, String match, String left, String right) :
		Bookmark(parent, std::move(title)), m_file(std::move(file)), m_match(std::move(match)),
		m_left(std::move(left)), m_right(std::move(right))
{
	m_layer = layer;
	m_start = start;
	m_end = end;
}

const char *AnnotationStamp::class_name() const
{
	return "AnnotationStamp";
}

void AnnotationStamp::to_xml(xml_node root)
{
	auto node = root.append_child("Bookmark");
	auto attr = node.append_attribute("type");
	attr.set_value(class_name());

	String path(m_file->path());
	filesystem::compress(path, Project::instance()->directory());

	add_data_node(node, "Title", m_title);
	add_data_node(node, "Notes", m_notes);
	add_data_node(node, "LeftContext", m_left);
	add_data_node(node, "Match", m_match);
	add_data_node(node, "RightContext", m_right);
	add_data_node(node, "File", path);

//	std::ostringstream start;
//	start << std::setprecision(6) << std::fixed;

	add_data_node(node, "Layer", utils::format("%", m_layer).data());
	add_data_node(node, "Start", utils::format("%", m_start).data());
	add_data_node(node, "End", utils::format("%", m_end).data());
}
} // namespace phonometrica
