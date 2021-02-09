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

#include <phon/application/conc/constraint.hpp>

namespace phonometrica {

Constraint::Constraint(const Constraint &other) :
		layer_pattern(other.layer_pattern), target(other.target)
{
	this->op = other.op;
	this->use_regex = other.use_regex;
	this->case_sensitive = other.case_sensitive;
	this->layer_index = other.layer_index;
	if (other.regex) {
		regex = std::make_unique<Regex>(other.regex->pattern(), other.regex->flags());
	}
	if (other.layer_regex) {
		layer_regex = std::make_unique<Regex>(other.layer_regex->pattern(), other.regex->flags());
	}
}

Constraint::Constraint(Constraint &&other) :
		layer_pattern(other.layer_pattern), target(other.target),
		regex(std::move(other.regex)), layer_regex(std::move(other.layer_regex))
{
	this->op = other.op;
	this->use_regex = other.use_regex;
	this->case_sensitive = other.case_sensitive;
	this->layer_index = other.layer_index;
}
void Constraint::to_xml(xml_node root)
{
	auto node = root.append_child("Constraint");
	node.append_attribute("operator").set_value(op_to_name(op));
	auto layer_node = node.append_child("Layer");
	layer_node.append_attribute("type").set_value(use_index() ? "index" : "regex");
	layer_node.append_child(node_pcdata).set_value(use_index() ? String::convert(intptr_t(layer_index)).data() : layer_pattern.data());
	auto target_node = node.append_child("Target");
	target_node.append_attribute("regex").set_value(use_regex);
	target_node.append_attribute("case_sensitive").set_value(case_sensitive);
	target_node.append_child(node_pcdata).set_value(target.data());
}

Constraint::Operator Constraint::name_to_op(std::string_view name)
{
	if (name == "dominance")
		return Operator::Dominance;
	if (name == "strict dominance")
		return Operator::StrictDominance;
	if (name == "left-alignment")
		return Operator::LeftAlignment;
	if (name == "right-alignment")
		return Operator::RightAlignment;
	if (name == "precedence")
		return Operator::Precedence;
	if (name == "subsequence")
		return Operator::Subsequence;
	if (name == "none")
		return Operator::None;

	throw error("Invalid constraint operator name: %", name);
}

const char *Constraint::op_to_name(Operator op)
{
	switch (op)
	{
		case Operator::Dominance:
			return "dominance";
		case Operator::StrictDominance:
			return "strict dominance";
		case Operator::LeftAlignment:
			return "left-alignment";
		case Operator::RightAlignment:
			return "right-alignment";
		case Operator::Precedence:
			return "precedence";
		case Operator::Subsequence:
			return "subsequence";
		case Operator::None:
			return "none";
		default:
			throw error("Invalid constraint operator");
	}
}

Constraint &Constraint::operator=(Constraint other) noexcept
{
	swap(other);
	return *this;
}

void Constraint::swap(Constraint &other) noexcept
{
	std::swap(this->op, other.op);
	std::swap(this->use_regex, other.use_regex);
	std::swap(this->case_sensitive, other.case_sensitive);
	std::swap(this->layer_index, other.layer_index);
	std::swap(this->layer_pattern, other.layer_pattern);
	std::swap(this->target, other.target);
	std::swap(this->regex, other.regex);
	std::swap(this->layer_regex, other.layer_regex);
}

void Constraint::compile()
{
	if (use_regex && !regex)
	{
		if (case_sensitive) {
			regex = std::make_unique<Regex>(target);
		}
		else {
			regex = std::make_unique<Regex>(target, Regex::Caseless);
		}

		if (!use_index() && !layer_regex)
		{
			layer_regex = std::make_unique<Regex>(layer_pattern);
		}
	}
}

} // namespace phonometrica
