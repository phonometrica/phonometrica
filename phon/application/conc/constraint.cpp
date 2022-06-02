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
	this->relation = other.relation;
	this->op = other.op;
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
	this->relation = other.relation;
	this->op = other.op;
	this->case_sensitive = other.case_sensitive;
	this->layer_index = other.layer_index;
}
void Constraint::to_xml(xml_node root)
{
	auto node = root.append_child("Constraint");
	node.append_attribute("operator").set_value(relation_to_name(relation));
	auto layer_node = node.append_child("Layer");
	layer_node.append_attribute("type").set_value(use_index() ? "index" : "regex");
	layer_node.append_child(node_pcdata).set_value(use_index() ? String::convert(intptr_t(layer_index)).data() : layer_pattern.data());
	auto target_node = node.append_child("Target");
	target_node.append_attribute("operator").set_value(operator_to_name(op));
	target_node.append_attribute("case_sensitive").set_value(case_sensitive);
	target_node.append_child(node_pcdata).set_value(target.data());
}

Constraint::Relation Constraint::name_to_relation(std::string_view name)
{
	if (name == "dominance")
		return Relation::Dominance;
	if (name == "strict dominance")
		return Relation::StrictDominance;
	if (name == "alignment")
		return Relation::Alignment;
	if (name == "left-alignment")
		return Relation::LeftAlignment;
	if (name == "right-alignment")
		return Relation::RightAlignment;
	if (name == "precedence")
		return Relation::Precedence;
	if (name == "subsequence")
		return Relation::Subsequence;
	if (name == "none")
		return Relation::None;

	throw error("Invalid constraint relation name: %", name);
}

const char *Constraint::relation_to_name(Relation rel)
{
	switch (rel)
	{
		case Relation::Dominance:
			return "dominance";
		case Relation::StrictDominance:
			return "strict dominance";
		case Relation::Alignment:
			return "alignment";
		case Relation::LeftAlignment:
			return "left-alignment";
		case Relation::RightAlignment:
			return "right-alignment";
		case Relation::Precedence:
			return "precedence";
		case Relation::Subsequence:
			return "subsequence";
		case Relation::None:
			return "none";
		default:
			throw error("Invalid constraint relation");
	}
}

Constraint::Operator Constraint::name_to_operator(std::string_view name)
{
	if (name == "equals")
		return Operator::Equals;
	if (name == "contains")
		return Operator::Contains;
	if (name == "matches")
		return Operator::Matches;

	throw error("Invalid constraint operator name: %", name);
}

const char *Constraint::operator_to_name(Constraint::Operator op)
{
	switch (op)
	{
		case Operator::Equals:
			return "equals";
		case Operator::Contains:
			return "contains";
		case Operator::Matches:
			return "matches";
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
	std::swap(this->relation, other.relation);
	std::swap(this->op, other.op);
	std::swap(this->case_sensitive, other.case_sensitive);
	std::swap(this->layer_index, other.layer_index);
	std::swap(this->layer_pattern, other.layer_pattern);
	std::swap(this->target, other.target);
	std::swap(this->regex, other.regex);
	std::swap(this->layer_regex, other.layer_regex);
}

void Constraint::compile()
{
	if (this->op == Operator::Matches && !regex)
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

bool Constraint::is_hierarchical(Constraint::Relation rel)
{
	return rel < Relation::Precedence;
}


} // namespace phonometrica
