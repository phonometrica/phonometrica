/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Purpose: A search constraint, used to extract a concordance.                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONSTRAINT_HPP
#define PHONOMETRICA_CONSTRAINT_HPP

#include <phon/string.hpp>
#include <phon/utils/xml.hpp>
#include <phon/regex.hpp>

namespace phonometrica {


struct Constraint final
{
	enum class Operator : uint8_t
	{
		Equals,     // exact string
		Contains,   // substring
		Matches,    // regular expression
		None        // invalid operator
	};

	enum class Relation : uint8_t
	{
		Dominance = 0,
		StrictDominance,
		Alignment,
		LeftAlignment,
		RightAlignment,
		Precedence,
		Subsequence,
		None
	};

	Constraint() = default;
	Constraint(const Constraint &other);
	Constraint(Constraint &&other);

	Constraint &operator=(Constraint other) noexcept;

	void swap(Constraint &other) noexcept;

	void to_xml(xml_node root);

	static Relation name_to_relation(std::string_view name);

	static const char *relation_to_name(Relation rel);

	static Operator name_to_operator(std::string_view name);

	static const char *operator_to_name(Operator op);

	bool use_index() const { return layer_index >= 0; }

	void compile();

	static bool is_hierarchical(Relation rel);

	// Search Operator
	Operator op = Operator::None;

	// Relation with the previous constraint, if any.
	Relation relation = Relation::None;

	// Whether the match is case-sensitive.
	bool case_sensitive = false;

	// Layer index: if this is 0, we search everywhere; if it's a valid index, we use it, otherwise we use the layer's name.
	int layer_index = -1;

	// Pattern to match a layer's name against. The name must match the pattern exactly.
	String layer_pattern;

	// Target text or pattern.
	String target;

	// Cached regexes
	std::unique_ptr<Regex> regex, layer_regex;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CONSTRAINT_HPP
