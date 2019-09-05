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
 * Purpose: transforms a query represented as string into a stream of tokens to be consumed by the query parser.      *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_LEXER_HPP
#define PHONOMETRICA_QUERY_LEXER_HPP

#include <phon/string.hpp>

namespace phonometrica {


struct Token final
{
	enum Code
	{
		Invalid,
		And,    // Boolean AND
		Or,     // Boolean OR
		Not,    // Boolean NOT
		LParen, // "("
		RParen, // ")"
		Number, // # followed by n digits
		Eot     // end of text
	};

	bool is(Code c) const { return code == c; }

	Code code = Invalid;
	int number = 0;
};


//----------------------------------------------------------------------------------------------------------------------

class QueryLexer final
{
public:

	explicit QueryLexer(String query);

	Token next();

private:

	void skip_white();

	void read_char();

	void accept() { read_char(); }

	String m_query;

	String::const_iterator m_pos;

	char32_t m_char;
};


} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_LEXER_HPP
