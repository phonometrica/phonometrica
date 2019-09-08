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
#include <phon/application/search/query_match.hpp>

namespace phonometrica {

// Abstract base class for all search nodes.
class SearchNode
{
public:

	virtual ~SearchNode() = default;

	virtual QueryMatchSet filter(const QueryMatchSet &matches) = 0;

	virtual String to_string() const = 0;

};

using AutoSearchNode = std::shared_ptr<SearchNode>;


//----------------------------------------------------------------------------------------------------------------------

class SearchOperator final : public SearchNode
{
public:

	enum class Opcode
	{
		And,
		Or
	};

	explicit SearchOperator(Opcode op) : opcode(op) { }

	QueryMatchSet filter(const QueryMatchSet &matches) override;

	void set_constraints(Array<AutoSearchNode> nodes);

	String to_string() const override;

private:

	Array<AutoSearchNode> children;

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

	SearchConstraint(int layer_index, String layer_name, bool case_sensitive, Opcode op, Relation rel, String value) :
		layer_index(layer_index), layer_name(std::move(layer_name)), case_sensitive(case_sensitive),
		opcode(op), relation(rel), value(std::move(value))
	{ }

	QueryMatchSet filter(const QueryMatchSet &matches) override;

	String to_string() const override;

private:

	bool use_index() const;

	String layer_name;

	int layer_index;

	bool case_sensitive;

	Opcode opcode;

	Relation relation;

	String value; // Text or regex.
};

using AutoSearchConstraint = std::shared_ptr<SearchConstraint>;

} // namespace phonometrica

#endif // PHONOMETRICA_SEARCH_NODE_HPP
