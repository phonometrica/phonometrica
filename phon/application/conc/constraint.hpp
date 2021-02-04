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

	void to_xml(xml_node root);

	static Operator name_to_op(std::string_view name);

	static const char *op_to_name(Operator op);

	// Relation with the previous constraint, if any.
	Operator op;

	// Use regular expression or plain text search.
	bool use_regex;

	// Whether the match is case-sensitive.
	bool case_sensitive;

	// If true, use the layer pattern, otherwise use the layer index.
	bool by_name;

	// Layer index, if using SearchOperator.
	int layer_index;

	// Pattern to match a layer's name against. The name must match the pattern exactly.
	String layer_pattern;

	// Target text or pattern.
	String target;
};

} // namespace phonometrica

#endif // PHONOMETRICA_CONSTRAINT_HPP
