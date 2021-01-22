/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 10/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Parse a stream of token (produced by the query editor) into a abstract syntax tree which can be consumed   *
 * by a query object.                                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_PARSER_HPP
#define PHONOMETRICA_QUERY_PARSER_HPP

#include <phon/array.hpp>
#include <phon/application/search/search_node.hpp>

namespace phonometrica {

class QueryParser final
{
public:

	enum class Type
	{
		Null,
		LParen,
		Rparen,
		And,
		Or,
		Not,
		Constraint
	};

	struct Token
	{
		Type type;
		AutoSearchNode node;
	};

	explicit QueryParser(Array<Token> tokens) : tokens(std::move(tokens)) { }

	~QueryParser() = default;

	AutoSearchNode parse();

private:

	/*
	 * Grammar:
	 * expression := or_expression
	 * or_expression := and_expression ( 'OR' and_expression ) *
	 * and_expression := primary ( 'AND' primary ) *
	 * primary = '(' expression ')' | constraint | 'NOT' primary
	 */

	AutoSearchNode parse_expression();

	AutoSearchNode parse_or_expression();

	AutoSearchNode parse_and_expression();

	AutoSearchNode parse_primary();

	Token *nextToken();

	void accept() { readToken(); }

	void accept(Type t, const char *msg);

	void readToken() { the_token = nextToken(); }

	Array<Token> tokens;

	Token *the_token = nullptr;

	int token_index = 0;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_PARSER_HPP
