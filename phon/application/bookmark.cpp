/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime.hpp>
#include <phon/application/bookmark.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Bookmark::Bookmark(Class *klass, Directory *parent) :
		Element(nullptr, parent)
{

}


Bookmark::Bookmark(Directory *parent, String title) :
		Element(nullptr, parent), m_title(std::move(title))
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

void Bookmark::set_notes(const String &value, bool mutate)
{
	m_notes = value;
	m_content_modified |= mutate;
}

bool Bookmark::quick_search(const String &text) const
{
	return m_title.to_lower().contains(text) || m_notes.to_lower().contains(text);
}

TimeStamp::TimeStamp(Directory *parent, String title, Handle<Annotation> annot, size_t layer,
					 double start, double end, String match, String left, String right) :
		Bookmark(parent, std::move(title)), m_annot(std::move(annot)), m_match(std::move(match)),
		m_left(std::move(left)), m_right(std::move(right))
{
	m_layer = layer;
	m_start = start;
	m_end = end;
}

void TimeStamp::to_xml(xml_node root)
{
	auto node = root.append_child("Bookmark");
	auto attr = node.append_attribute("type");
	attr.set_value(class_name().data());

	String path(m_annot->path());
	Project::compress(path, Project::get()->directory());

	add_data_node(node, "Title", m_title);
	add_data_node(node, "Notes", m_notes);
	add_data_node(node, "LeftContext", m_left);
	add_data_node(node, "Match", m_match);
	add_data_node(node, "RightContext", m_right);
	add_data_node(node, "Annotation", path);
	add_data_node(node, "Layer", String::convert(intptr_t(m_layer)));
	add_data_node(node, "Start", String::convert(m_start));
	add_data_node(node, "End", String::convert(m_end));
}

String TimeStamp::tooltip() const
{
	String s("<b>File:</b><br/>");
	s.append(filesystem::base_name(m_annot->path()));
	s.append("<br/><b>Match:</b><br/>");
	s.append(m_left);
	s.append(" <font color = \"red\"><strong>");
	s.append(m_match);
	s.append("</string></font> ");
	s.append(m_right);

	if (! m_notes.empty())
	{
		s.append("<br/><b>Notes:</b><br/>");
		s.append(m_notes);
	}

	return s;
}

void TimeStamp::initialize(Runtime &rt)
{

}

} // namespace phonometrica
