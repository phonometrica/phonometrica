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
 * Created: 10/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/application/search/query_parser.hpp>

namespace phonometrica {

AutoSearchNode QueryParser::parse()
{
	readToken();
	return parseExpression();
}

AutoSearchNode QueryParser::parseExpression()
{
	return parseOrExpression();
}

AutoSearchNode QueryParser::parseOrExpression()
{
	auto lhs = parseAndExpression();

	while (the_token->type == Type::Or)
	{
		accept();
		auto rhs = parseAndExpression();
		lhs = std::make_shared<SearchOperator>(SearchOperator::Opcode::Or, std::move(lhs), std::move(rhs));
	}

	return lhs;
}

AutoSearchNode QueryParser::parseAndExpression()
{
	auto lhs = parsePrimary();

	while (the_token->type == Type::And)
	{
		accept();
		auto rhs = parsePrimary();
		lhs = std::make_shared<SearchOperator>(SearchOperator::Opcode::And, std::move(lhs), std::move(rhs));
	}

	return lhs;
}

AutoSearchNode QueryParser::parsePrimary()
{
	if (the_token->type == Type::LParen)
	{
		accept();
		auto e = parseExpression();
		accept(Type::Rparen, "closing parenthesis");

		return e;
	}
	else if (the_token->type == Type::Constraint)
	{
		auto node = std::move(the_token->node);
		accept();

		return node;
	}

	throw error("[Internal error] Invalid primary expression in query parser");
}

QueryParser::Token *QueryParser::nextToken()
{
	static Token invalid = { Type::Null, nullptr };

	if (token_index < tokens.size())
	{
		return &tokens[++token_index];
	}

	return &invalid;
}

void QueryParser::accept(QueryParser::Type t, const char *msg)
{
	if (the_token->type == t)
	{
		accept();
	}
	else
	{
		throw error("[Internal error] Invalid query token: expected a %", msg);
	}
}


} // namespace phonometrica