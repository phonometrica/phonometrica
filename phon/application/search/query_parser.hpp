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
 * Purpose: build an abstract syntax tree (AST) from a query string.                                                  *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_PARSER_HPP
#define PHONOMETRICA_QUERY_PARSER_HPP

#include <phon/application/search/search_node.hpp>
#include <phon/application/search/query_lexer.hpp>

namespace phonometrica {

class QueryParser final
{
public:

	explicit QueryParser(String query);

	std::unique_ptr<SearchNode> parse();

private:

	void read_token();

	void accept() { read_token(); }

	void accept(Token::Code c, const char *msg);

	bool check(Token::Code c) const { return m_token.is(c); }

	std::unique_ptr<SearchNode> parse_expression();

	QueryLexer m_lexer;

	Token m_token;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_PARSER_HPP
