/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/project.hpp>
#include <phon/application/conc/text_query.hpp>
#include <phon/utils/xml.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

TextQuery::TextQuery(VFolder *parent, const String &path) :
		Query(parent, path)
{
	if (!path.empty()) {
		load();
	}
}

void TextQuery::load()
{
	xml_document doc;
	xml_node root;

	try
	{
		root = read_xml(doc, m_path);
	}
	catch (...)
	{
		throw error("Cannot open text query \"%\"", m_path);
	}

	if (root.name() != std::string_view("Phonometrica")) {
		throw error("Invalid XML project root in %", m_path);
	}

	auto attr = root.attribute("class");

	if (!attr || attr.as_string() != std::string_view(class_name())) {
		throw error("Expected a text query, got a % file instead", attr.as_string());
	}

	attr = root.attribute("label");
	if (attr) {
		set_label(attr.value(), false);
	}
	else {
		set_label(filesystem::base_name(m_path), false);
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == std::string_view("Metadata"))
		{
			metadata_from_xml(node);
		}
		else if (node.name() == std::string_view("MetaConstraints"))
		{
			metaconstraints_from_xml(node);
		}
		else if (node.name() == std::string_view("Constraints"))
		{
			constraints_from_xml(node);
		}
	}

	m_loaded = true;
}

void TextQuery::metaconstraints_from_xml(xml_node root)
{
	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == std::string_view("Description"))
		{
			auto attr = node.attribute("operator");
			auto op = DescMetaConstraint::name_to_op(attr.value());
			String value = node.text().get();
			add_metaconstraint(std::make_unique<DescMetaConstraint>(op, std::move(value)), false);
		}
		else if (node.name() == std::string_view("FileSelection"))
		{
			for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
			{
				if (subnode.name() != std::string_view("File")) {
					throw error("Invalid XML node in text query file selection");
				}
				String path = subnode.text().get();
				auto vfile = Project::get()->get(path);
				auto annot = downcast<Annotation>(vfile);

				if (!annot) {
					throw error("Invalid annotation in text query file selection");
				}
				selected_annotations.append(std::move(annot));
			}
		}
	}
}

void TextQuery::constraints_from_xml(xml_node root)
{

}

void TextQuery::write()
{
	xml_document doc;

	auto root = doc.append_child("Phonometrica");
	auto attr = root.append_attribute("class");
	attr.set_value(class_name());
	attr = root.append_attribute("label");
	attr.set_value(m_label.data());
	auto metadata_node = root.append_child("Metadata");
	metadata_to_xml(metadata_node);

	auto meta_node = root.append_child("MetaConstraints");
	auto file_sel_node = meta_node.append_child("FileSelection");
	for (auto &file : selected_annotations)
	{
		add_data_node(file_sel_node, "File", file->path());
	}
	for (auto &mc : m_metaconstraints)
	{
		mc->to_xml(meta_node);
	}

	// Options
	auto option_node = root.append_child("Options");
	auto ctx_node = option_node.append_child("Context");
	auto type_attr = ctx_node.append_attribute("type");
	switch (m_context)
	{
		case Context::Labels:
		{
			type_attr.set_value("labels");
			ctx_node.append_attribute("ref").set_value(m_ref_constraint);
		} break;
		case Context::KWIC:
		{
			type_attr.set_value("kwic");
			ctx_node.append_attribute("ref").set_value(m_ref_constraint);
			ctx_node.append_attribute("length").set_value(m_context_length);
		} break;
		default:
			type_attr.set_value("none");
	}

	// Constraints
	auto data_node = root.append_child("Constraints");
	for (auto &constraint : m_constraints)
	{
		constraint.to_xml(data_node);
	}

	write_xml(doc, m_path);
}

int TextQuery::context_length() const
{
	return m_context_length;
}

void TextQuery::set_context_length(int context_length)
{
	m_context_length = context_length;
}

TextQuery::Context TextQuery::context() const
{
	return m_context;
}

void TextQuery::set_context(TextQuery::Context context)
{
	m_context = context;
}

int TextQuery::reference_constraint() const
{
	return m_ref_constraint;
}

void TextQuery::set_reference_constraint(int value)
{
	m_ref_constraint = value;
}

} // namespace phonometrica
