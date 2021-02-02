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

#include <phon/application/conc/metaconstraint.hpp>

namespace phonometrica {

DescMetaConstraint::DescMetaConstraint(DescMetaConstraint::Operator op, const String &value) :
	MetaConstraint(), op(op), value(value)
{
	if (op == Operator::Match || op == Operator::NotMatch) {
		regex = std::make_unique<Regex>(value);
	}
}

bool DescMetaConstraint::filter(const VFile *file) const
{
	switch (op)
	{
		case Operator::Equal:
			return file->description() == value;
		case Operator::NotEqual:
			return file->description() != value;
		case Operator::Contains:
			return file->description().contains(value);
		case Operator::NotContains:
			return !file->description().contains(value);
		case Operator::Match:
			return regex->match(value);
		case Operator::NotMatch:
			return !regex->match(value);
		default:
			return false;
	}
}

const char *DescMetaConstraint::op_to_name(Operator op)
{
	switch (op)
	{
		case Operator::Equal:
			return "equal";
		case Operator::NotEqual:
			return "notequal";
		case Operator::Contains:
			return "contains";
		case Operator::NotContains:
			return "notcontains";
		case Operator::Match:
			return "match";
		case Operator::NotMatch:
			return "notmatch";
		default:
			throw error("Invalid description operator");
	}
}

DescMetaConstraint::Operator DescMetaConstraint::name_to_op(std::string_view name)
{
	if (name == "contains")
		return Operator::Contains;
	if (name == "notcontains")
		return Operator::NotContains;
	if (name == "equal")
		return Operator::Equal;
	if (name == "notequal")
		return Operator::NotEqual;
	if (name == "match")
		return Operator::Match;
	if (name == "notmatch")
		return Operator::NotMatch;

	throw error("Invalid description operator name");
}

void DescMetaConstraint::to_xml(xml_node node)
{
	auto desc_node = node.append_child("Description");
	auto data_node = desc_node.append_child(node_pcdata);
	data_node.set_value(value.data());
	auto op_node = desc_node.append_attribute("operator");
	op_node.set_value(op_to_name(op));
}

bool TextMetaConstraint::filter(const VFile *file) const
{
	for (auto &prop : file->properties())
	{
		if (prop.category() == this->category && values.contains(prop.value())) {
			return true;
		}
	}

	return false;
}

void TextMetaConstraint::to_xml(xml_node node)
{
	auto prop_node = node.append_child("Property");
	auto type_attr = prop_node.append_attribute("type");
	type_attr.set_value("text");
	auto cat_attr = prop_node.append_attribute("category");
	cat_attr.set_value(category.data());

	for (auto &value : values) {
		add_data_node(prop_node, "Value", value);
	}
}

bool NumericMetaConstraint::filter(const VFile *file) const
{
	for (auto &prop : file->properties())
	{
		if (prop.category() != this->category) {
			continue;
		}
		if (check_value(prop.numeric_value())) {
			return true;
		}
	}

	return false;
}

bool NumericMetaConstraint::check_value(double num) const
{
	switch (op)
	{
		case Operator::Equal:
			return num == value.first;
		case Operator::NotEqual:
			return num != value.first;
		case Operator::Less:
			return num < value.first;
		case Operator::LessEqual:
			return num <= value.first;
		case Operator::Greater:
			return num > value.first;
		case Operator::GreaterEqual:
			return num >= value.first;
		case Operator::Range:
			return num >= value.first && num <= value.second;
	}

	return false;
}

void NumericMetaConstraint::to_xml(xml_node node)
{
	auto prop_node = node.append_child("Property");
	auto type_attr = prop_node.append_attribute("type");
	type_attr.set_value("numeric");
	auto cat_attr = prop_node.append_attribute("category");
	cat_attr.set_value(category.data());
	auto op_attr = prop_node.append_attribute("operator");
	op_attr.set_value(op_to_name(op));
	auto value_node = prop_node.append_child("Value");
	auto v1_node = value_node.append_child(node_pcdata);
	v1_node.set_value(String::convert(value.first).data());

	if (op == Operator::Range)
	{
		value_node = prop_node.append_child("Value");
		auto v2_node = value_node.append_child(node_pcdata);
		v2_node.set_value(String::convert(value.second).data());
	}
}

const char * NumericMetaConstraint::op_to_name(NumericMetaConstraint::Operator op)
{
	switch (op)
	{
		case Operator::Equal:
			return "eq";
		case Operator::NotEqual:
			return "neq";
		case Operator::Less:
			return "lt";
		case Operator::LessEqual:
			return "le";
		case Operator::Greater:
			return "gt";
		case Operator::GreaterEqual:
			return "ge";
		case Operator::Range:
			return "in";
	}

	throw error("Invalid operator for numeric meta-constraint");
}

NumericMetaConstraint::Operator NumericMetaConstraint::name_to_op(std::string_view name)
{
	if (name == "eq")
		return Operator::Equal;
	if (name == "neq")
		return Operator::NotEqual;
	if (name == "lt")
		return Operator::Less;
	if (name == "le")
		return Operator::LessEqual;
	if (name == "gt")
		return Operator::Greater;
	if (name == "ge")
		return Operator::GreaterEqual;
	if (name == "in")
		return Operator::Range;

	throw error("Invalid operator name for numeric meta-constraint");
}

bool BooleanMetaConstraint::filter(const VFile *file) const
{
	for (auto &prop : file->properties())
	{
		if (prop.category() == this->category && prop.boolean_value() == this->value) {
			return true;
		}
	}

	return false;
}

void BooleanMetaConstraint::to_xml(xml_node node)
{
	auto prop_node = node.append_child("Property");
	auto type_attr = prop_node.append_attribute("type");
	type_attr.set_value("boolean");
	auto cat_attr = prop_node.append_attribute("category");
	cat_attr.set_value(category.data());
	add_data_node(prop_node, "Value", value ? "true" : "false");
}
} // namespace phonometrica
