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
#include <phon/application/search/query_lexer.hpp>

namespace phonometrica {

QueryLexer::QueryLexer(String query) : m_query(std::move(query))
{
	m_pos = m_query.begin();
	m_char = 0;
	read_char();
}

Token QueryLexer::next()
{
	read_char();

	switch (m_char)
	{
		case U'#':
		{
			accept();
			String buffer;

			while (isdigit((int)m_char))
			{
				buffer.append(m_char);
				read_char();
			}

			bool ok = false;
			auto n = (int) buffer.to_int(&ok);
			if (!ok) throw error("[Query error] Invalid query number after '#'");

			return { Token::Number, n };
		}
		case U'A':
		case U'a':
		{
			accept();
			if (m_char == U'N' || m_char == 'n')
			{
				accept();
				if (m_char == U'D' || m_char == 'd')
				{
					accept();
					return { Token::And, 0 };
				}
			}

			throw error("[Query error] Invalid token. Did you mean 'AND'?");
		}
		case U'O':
		case U'o':
		{
			accept();
			if (m_char == 'R' || m_char == U'r')
			{
				accept();
				return { Token::Or, 0 };
			}

			throw error("[Query error] Invalid token. Did you mean 'OR'?");
		}
		case U'N':
		case U'n':
		{
			accept();
			if (m_char == U'O' || m_char == U'o')
			{
				accept();
				if (m_char == U'T' || m_char == U't')
				{
					return { Token::Not, 0 };
				}
			}

			throw error("[Query error] Invalid token. Did you mean 'NOT'?");
		}
		case U'(':
		{
			accept();
			return { Token::LParen, 0 };
		}
		case U')':
		{
			accept();
			return { Token::RParen, 0 };
		}
		case 0x03:
			return { Token::Eot, 0 };
		default:
			throw error("[Query error] Invalid query string \"%\"", m_query);
	}

	return { Token::Eot, 0 };
}

void QueryLexer::skip_white()
{
	while (isspace((int)m_char))
	{
		read_char();
	}
}

void QueryLexer::read_char()
{
	if (m_pos == m_query.end())
	{
		m_char = 0x03; // end of text
	}
	else
	{
		m_char = m_query.next_codepoint(m_pos);
		skip_white();
	}
}
} // namespace phonometrica