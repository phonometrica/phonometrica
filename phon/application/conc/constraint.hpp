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
		Dominance = 0,
		StrictDominance,
		LeftAlignment,
		RightAlignment,
		Precedence,
		Subsequence,
		None
	};

	Constraint() = default;
	Constraint(const Constraint &) = default;
	Constraint(Constraint &&) = default;

	Constraint &operator=(Constraint other) noexcept;

	void swap(Constraint &other) noexcept;

	void to_xml(xml_node root);

	static Operator name_to_op(std::string_view name);

	static const char *op_to_name(Operator op);

	bool use_index() const { return layer_index >= 0; }

	void compile();

	// Relation with the previous constraint, if any.
	Operator op = Operator::None;

	// Use regular expression or plain text search.
	bool use_regex = true;

	// Whether the match is case-sensitive.
	bool case_sensitive = false;

	// Layer index: if this is 0, we search everywhere; if it's a valid index, we use it, otherwise we use the layer's name.
	int layer_index = -1;

	// Pattern to match a layer's name against. The name must match the pattern exactly.
	String layer_pattern;

	// Target text or pattern.
	String target;

	// Cached regexes
//	std::unique_ptr<Regex> regex, layer_regex;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CONSTRAINT_HPP
