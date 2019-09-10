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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <algorithm>
#include <phon/application/search/search_node.hpp>

namespace phonometrica {

QueryMatchSet SearchOperator::filter(const QueryMatchSet &matches)
{
	if (opcode == Opcode::And)
	{
		auto results = lhs->filter(matches);
		return rhs->filter(results);
	}
	else
	{
		QueryMatchSet x = lhs->filter(matches);
		QueryMatchSet y = rhs->filter(matches);
		QueryMatchSet results;
		std::set_union(x.begin(), x.end(), y.begin(), y.end(), std::inserter(results, results.begin()));

		return results;
	}
}

String SearchOperator::to_string() const
{
	assert(opcode != Opcode::Null);

	String s("(");
	s.append(lhs->to_string());
	if (opcode == Opcode::And)
		s.append(" AND ");
	else
		s.append(" OR ");
	s.append(rhs->to_string());
	s.append(')');

	return s;
}

bool SearchConstraint::use_index() const
{
	return layer_index >= 0;
}

QueryMatchSet SearchConstraint::filter(const QueryMatchSet &matches)
{
	return matches;
}

String SearchConstraint::to_string() const
{
	String s("e");
	s.append(String::convert(intptr_t(index)));
	return s;
//	auto s = utils::format("(layer_index = %, layer_name = \"%\", case_sensitive = %, op = %, relation = %, value = \"%\")",
//	                       layer_index, layer_name, case_sensitive, static_cast<int>(op), static_cast<int>(relation), value);
//
//	return String(s);
}
} // namespace phonometrica
