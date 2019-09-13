/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 03/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: nodes in the AST representing a parsed query.                                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SEARCH_NODE_HPP
#define PHONOMETRICA_SEARCH_NODE_HPP

#include <memory>
#include <phon/regex.hpp>
#include <phon/application/search/query_match.hpp>

namespace phonometrica {

// Abstract base class for all search nodes.
class SearchNode
{
public:

	virtual ~SearchNode() = default;

	virtual QueryMatchSet filter(const AutoAnnotation &annotation, const QueryMatchSet &matches) = 0;

	virtual String to_string() const = 0;

};

using AutoSearchNode = std::shared_ptr<SearchNode>;


//----------------------------------------------------------------------------------------------------------------------

class SearchOperator final : public SearchNode
{
public:

	enum class Opcode
	{
		Null,
		Or,
		And,
		Not
	};

	explicit SearchOperator(Opcode op, AutoSearchNode lhs, AutoSearchNode rhs = nullptr) :
		opcode(op), lhs(std::move(lhs)), rhs(std::move(rhs)) { }

	QueryMatchSet filter(const AutoAnnotation &annot, const QueryMatchSet &matches) override;

	String to_string() const override;

private:

	AutoSearchNode lhs, rhs; // RHS is not used if the operator is Not.

	Opcode opcode;
};

//----------------------------------------------------------------------------------------------------------------------

class SearchConstraint final : public SearchNode
{
public:

	enum class Opcode : uint8_t
	{
		Matches,
		Equals
	};

	enum class Relation : uint8_t
	{
		None,
		Alignment,
		Dominance,
		Precedence
	};

	SearchConstraint(int context_length, int index, int layer_index, const String &layer_name, bool case_sensitive,
			Opcode op, Relation rel, String value);

	QueryMatchSet filter(const AutoAnnotation &annotation, const QueryMatchSet &matches) override;

	String to_string() const override;

	Opcode opcode() const { return op; }

	QueryMatchSet search(const AutoAnnotation &annot);


private:

	QueryMatchSet find_matches(const AutoAnnotation &annot, int layer_index, std::true_type use_regex);

	QueryMatchSet find_matches(const AutoAnnotation &annot, int layer_index, std::false_type use_regex);

	Regex value_pattern;

	// If this is null, use layer_index instead.
	std::unique_ptr<Regex> layer_pattern;

	// If the index is 0, all layers are matched. Otherwise we match the specific layer
	int layer_index;

	int index; // for debugging

	int context_length ;

	bool case_sensitive;

	Opcode op;

	Relation relation;

	String value; // Text or regex.
};

using AutoSearchConstraint = std::shared_ptr<SearchConstraint>;

} // namespace phonometrica

#endif // PHONOMETRICA_SEARCH_NODE_HPP
