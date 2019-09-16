/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 10/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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