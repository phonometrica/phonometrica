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

namespace phonometrica {

// Abstract base class for all search nodes.
struct SearchNode
{
public:

	virtual ~SearchNode() = default;
};

using SearchNodePtr = std::unique_ptr<SearchNode>;

//----------------------------------------------------------------------------------------------------------------------

struct AndSearchNode : public SearchNode
{
	SearchNodePtr left, right;
};

struct OrSearchNode : public SearchNode
{
	SearchNodePtr left, right;
};

struct NotSearchNode : public SearchNode
{
	SearchNodePtr child;
};

struct ExprSearchNode : public SearchNode
{

};

} // namespace phonometrica

#endif // PHONOMETRICA_SEARCH_NODE_HPP
