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

#include <phon/application/search/search_node.hpp>

namespace phonometrica {


void SearchOperator::set_constraints(Array <AutoSearchNode> nodes)
{
	children = std::move(nodes);
}

QueryMatchSet SearchOperator::filter(const QueryMatchSet &matches)
{
	if (opcode == Opcode::And)
	{
		QueryMatchSet results = matches;

		for (auto &child : children)
		{
			results = child->filter(results);
		}

		return results;
	}
	else
	{
		QueryMatchSet results;

		for (auto &child : children)
		{
			for (auto &match : child->filter(matches))
			{
				results.insert(match);
			}
		}

		return results;
	}
}

String SearchOperator::to_string() const
{
	intptr_t count = children.size();
	if (count == 1) return children.first()->to_string();

	String s("(");
	String infix = (opcode == Opcode::And) ? String(" AND ") : String (" OR ");

	for (intptr_t i = 1; i <= count; i++)
	{
		s.append(children[i]->to_string());
		if (i < count) s.append(infix);
	}
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
	auto s = utils::format("(layer_index = %, layer_name = \"%\", case_sensitive = %, opcode = %, relation = %, value = \"%\")",
			layer_index, layer_name, case_sensitive, static_cast<int>(opcode), static_cast<int>(relation), value);

	return String(s);
}
} // namespace phonometrica
