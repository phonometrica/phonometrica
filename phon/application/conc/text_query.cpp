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
	using str = std::string_view;

	try
	{
		root = read_xml(doc, m_path);
	}
	catch (...)
	{
		throw error("Cannot open text query \"%\"", m_path);
	}

	if (root.name() != str("Phonometrica")) {
		throw error("Invalid XML project root in %", m_path);
	}

	auto attr = root.attribute("class");

	if (!attr || attr.as_string() != str(class_name())) {
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
		if (node.name() == str("Metadata"))
		{
			metadata_from_xml(node);
		}
		else if (node.name() == str("MetaConstraints"))
		{
			parse_metaconstraints_from_xml(node);
		}
		else if (node.name() == str("Constraints"))
		{
			parse_constraints_from_xml(node);
		}
		else if (node.name() == str("Options"))
		{
			parse_options_from_xml(node);
		}
		else
		{
			throw error("Invalid node in text query: %", node.name());
		}
	}

	m_loaded = true;
}

void TextQuery::parse_metaconstraints_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Description"))
		{
			auto attr = node.attribute("operator");
			auto op = DescMetaConstraint::name_to_op(attr.value());
			String value = node.text().get();
			add_metaconstraint(std::make_unique<DescMetaConstraint>(op, std::move(value)), false);
		}
		else if (node.name() == str("FileSelection"))
		{
			for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
			{
				if (subnode.name() != str("File")) {
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
		else if (node.name() == str("Property"))
		{
			auto cat_attr = node.attribute("category");
			if (!cat_attr) {
				throw error("Missing category in text query property");
			}
			String category = cat_attr.value();
			auto type_attr = node.attribute("type");
			if (!type_attr) {
				throw error("Missing type in text query property");
			}
			if (type_attr.value() == str("text"))
			{
				Array<String> values;
				for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
				{
					if (subnode.name() != str("Value")) {
						throw error("Expected a Value node in text property");
					}
					values.append(subnode.text().get());
				}
				m_metaconstraints.append(std::make_unique<TextMetaConstraint>(category, std::move(values)));
			}
			else if (type_attr.value() == str("numeric"))
			{
				auto op_node = node.attribute("operator");
				auto op = NumericMetaConstraint::name_to_op(op_node.value());
				auto value = std::make_pair<double,double>(0, 0);

				auto first_node = node.first_child();
				if (!first_node || first_node.name() != str("Value")) {
					throw error("Expected a Value node in numeric property");
				}
				String first_value = first_node.text().get();
				bool ok;
				value.first = first_value.to_float(&ok);
				if (!ok) {
					throw error("Invalid numeric value in numeric property");
				}
				if (op == NumericMetaConstraint::Operator::InclusiveRange || op == NumericMetaConstraint::Operator::ExclusiveRange)
				{
					auto second_node = first_node.next_sibling();
					if (!second_node || second_node.name() != str("Value")) {
						throw error("Expected a second Value node in numeric property with range operator");
					}
					String second_value = second_node.text().get();
					value.second = second_value.to_float(&ok);
					if (!ok) {
						throw error("Invalid numeric value in numeric property");
					}
				}
				m_metaconstraints.append(std::make_unique<NumericMetaConstraint>(category, op, value));
			}
			else if (type_attr.value() == str("boolean"))
			{
				auto subnode = node.first_child();
				if (!subnode || subnode.name() != str("Value")) {
					throw error("Expected a Value node in Boolean property");
				}
				bool value = subnode.text().as_bool();
				m_metaconstraints.append(std::make_unique<BooleanMetaConstraint>(category, value));
			}
			else
			{
				throw error("Invalid property type in text query: %", node.name());
			}
		}
		else
		{
			throw error("Invalid metaconstraint in text query: %", node.name());
		}
	}
}

void TextQuery::parse_constraints_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() != str("Constraint"))
		{
			throw error("Expected a Constraint node in text query, got: %", node.name());
		}

		auto op_attr = node.attribute("operator");
		if (!op_attr) {
			throw error("Missing operator in Constraint node");
		}
		auto op = Constraint::name_to_op(op_attr.value());
		int layer_index = -1;
		bool case_sensitive = false, use_regex = true;
		String layer_pattern, target;

		for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
		{
			if (subnode.name() == str("Layer"))
			{
				auto attr = subnode.attribute("type");
				if (!attr) {
					throw error("Missing type attribute in Layer node");
				}
				if (attr.name() == str("index"))
				{
					String text = subnode.text().get();
					bool ok;
					layer_index = text.to_int(&ok);
					if (!ok) {
						throw error("Invalid index layer_index in Constraint node");
					}
				}
				else
				{
					layer_pattern = subnode.text().get();
				}
			}
			else if (subnode.name() == str("Target"))
			{
				auto attr = subnode.attribute("regex");
				if (!attr) {
					throw error("Missing regex attribute in Target Node");
				}
				use_regex = attr.value() == str("true");

				attr = subnode.attribute("case_sensitive");
				if (!attr) {
					throw error("Missing case_sensitive attribute in Target Node");
				}
				case_sensitive = attr.value() == str("true");

				target = subnode.text().get();
			}
			else
			{
				throw error("Invalid Constraint subnode in text query: %", subnode.name());
			}
		}

		Constraint c;
		c.case_sensitive = case_sensitive;
		c.use_regex = use_regex;
		c.op = op;
		c.layer_index = layer_index;
		c.layer_pattern = layer_pattern;
		c.target = target;
		m_constraints.append(std::move(c));
	}
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

void TextQuery::parse_options_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Context"))
		{
			auto type_attr = node.attribute("type");
			if (!type_attr) {
				throw error("Missing type attribute in Context node");
			}
			auto type = str(type_attr.value());

			if (type == str("none"))
			{
				m_context = Context::None;
			}
			else
			{
				auto ref_attr = node.attribute("ref");
				if (!ref_attr) {
					throw error("Missing ref attribute in Context node");
				}

				String text = ref_attr.value();
				bool ok;
				m_ref_constraint = (int) text.to_int(&ok);
				if (!ok || m_ref_constraint < 0) {
					throw error("Invalid index for reference constraint in Context node: %", text);
				}

				if (type == str("kwic"))
				{
					auto len_attr = node.attribute("length");
					if (!len_attr) {
						throw error("Missing length attribute in Context node");
					}
					text = len_attr.value();
					m_context_length = (int) text.to_int(&ok);
					if (!ok || m_context_length < 0) {
						throw error("Invalid length in Context node: %", text);
					}
					m_context = Context::KWIC;
				}
				else if (type == str("labels"))
				{
					m_context = Context::Labels;
				}
				else
				{
					throw error("Invalid type in Context node: %", type);
				}
			}
		}
		else
		{
			throw error("Invalid option in text query: %", node.name());
		}

	}
}

} // namespace phonometrica
