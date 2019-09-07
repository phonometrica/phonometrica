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

#include <phon/error.hpp>
#include <phon/application/search/query_parser.hpp>

namespace phonometrica {

QueryParser::QueryParser(String query) : m_lexer(std::move(query))
{
	read_token();
}

AutoSearchNode QueryParser::parse()
{
	return parse_expression();
}

AutoSearchNode QueryParser::parse_expression()
{
	if (check(Token::LParen))
	{
		accept();
		auto e = parse_expression();
		accept(Token::RParen, ")");

		return e;
	}
	else
	{
		return AutoSearchNode();
	}
}

void QueryParser::read_token()
{
	m_token = m_lexer.next();
}

void QueryParser::accept(Token::Code c, const char *msg)
{
	if (check(c))
	{
		accept();
	}
	else
	{
		throw error("Invalid token: expected '%s'", msg);
	}
}
} // namespace phonometrica