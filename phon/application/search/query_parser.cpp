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
	return parse_expression();
}

AutoSearchNode QueryParser::parse_expression()
{
	return parse_or_expression();
}

AutoSearchNode QueryParser::parse_or_expression()
{
	auto lhs = parse_and_expression();

	while (the_token->type == Type::Or)
	{
		accept();
		auto rhs = parse_and_expression();
		lhs = std::make_shared<SearchOperator>(SearchOperator::Opcode::Or, std::move(lhs), std::move(rhs));
	}

	return lhs;
}

AutoSearchNode QueryParser::parse_and_expression()
{
	auto lhs = parse_primary();

	while (the_token->type == Type::And)
	{
		accept();
		auto rhs = parse_primary();
		lhs = std::make_shared<SearchOperator>(SearchOperator::Opcode::And, std::move(lhs), std::move(rhs));
	}

	return lhs;
}

AutoSearchNode QueryParser::parse_primary()
{
	if (the_token->type == Type::LParen)
	{
		accept();
		auto e = parse_expression();
		accept(Type::Rparen, "closing parenthesis");

		return e;
	}
	else if (the_token->type == Type::Not)
	{
		accept();
		return std::make_shared<SearchOperator>(SearchOperator::Opcode::Not, parse_primary());
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