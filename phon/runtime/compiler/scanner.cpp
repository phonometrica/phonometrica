/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 18/07/2019                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/runtime/compiler/scanner.hpp>

namespace phonometrica {

Scanner::Scanner() :
    m_source(std::make_shared<SourceCode>())
{
    m_line_no = 0;
    m_pos = nullptr;
    m_char = 0;
}

void Scanner::load_file(const String &path)
{
    reset();
    m_source->load_file(path);
    read_char();
}

void Scanner::load_string(const String &code)
{
    reset();
    m_source->load_code(code);
    read_char();
}

void Scanner::reset()
{
    m_pos = nullptr;
    m_line_no = 0;
    m_line.clear();
    m_char = 0;
}

void Scanner::rewind()
{
    if (m_char != Token::ETX)
    {
        // Move back to the beginning of the string
        m_pos = m_line.begin();
        read_char();
    }
    else
    {
        m_pos = nullptr;
    }
}

void Scanner::read_char()
{
    // Never read past the end of the source
    assert(m_char != Token::ETX);

    if (m_pos == m_line.end() || m_pos == nullptr)
    {
        read_line();
        rewind();
    }
    else
    {
        get_char();
    }
}

void Scanner::get_char()
{
    m_char = m_line.next_codepoint(m_pos);
}

void Scanner::set_line(intptr_t index)
{
    m_line = m_source->get_line(index);
}

void Scanner::read_line()
{
    if (m_line_no == m_source->size())
    {
        m_line.clear();
        m_char = Token::ETX;
    }
    else
    {
        set_line(++m_line_no);
    }
}

void Scanner::skip_white()
{

    while (check_space(m_char))
    {
        read_char();
    }
}

void Scanner::skip()
{
    read_char();
}

void Scanner::accept()
{
    m_spelling.append(m_char);
    read_char();
}

void Scanner::scan_digits()
{
    // Allow '_' as a group separator
    while (isdigit(m_char) || m_char == '_')
    {
        if (m_char == '_')
            skip();
        else
            accept();
    }
}

void Scanner::scan_string(char32_t end)
{
    skip();

    while (m_char != end && m_char != Token::ETX)
    {
        if (m_char == '\\')
        {
            // Skip for now. It may be restored if it's not a special character.
            skip();

            if (m_char == 'n')
            {
                m_char = '\n'; // line feed (new line)
            }
            else if (m_char == 't')
            {
                m_char = '\t'; // horizontal tab
            }
            else if (m_char == 'r')
            {
                m_char = '\r'; // carriage return
            }
            else if (m_char == '\\')
            {
                m_char = '\\'; // backslash
            }
            else if (m_char == '\'')
            {
                m_char = '\''; // single quote
            }
            else if (m_char == '"')
            {
                m_char = '\"'; // double quote
            }
            else if (m_char == 'v')
            {
                m_char = '\v'; // vertical tab
            }
            else if (m_char == 'a')
            {
                m_char = '\a'; // audible bell
            }
            else if (m_char == 'b')
            {
                m_char = '\b'; // backspace
            }
            else if (m_char == 'f')
            {
                m_char = '\f'; // form feed (new page)
            }
            else
            {
                // Restore
                m_spelling.push_back('\\');
            }
        }
        accept();
    }

    // If we haven't reached the end of the text, ignore string terminating character
    if (m_char == end)
    { skip(); }
}


// Read one token from the source code
Token Scanner::read_token()
{
    RETRY:
    m_spelling.clear();
    skip_white();

    // An identifier must start with a Unicode "alphabetic character". This includes characters such as
    // Chinese '漢' or Korean '한'.
    if (String::is_letter(m_char))
    {
        accept();

        while (String::is_letter(m_char) || isdigit(m_char) || m_char == U'_')
        {
            accept();
        }

        // Variable can end with '$'. This is used for "special" symbols, normally used for implementation details.
        // Special methods, such as the init$ constructor, end with this character. Private instance members should also
        // end with '$', although this is not enforced.
        if (m_char == U'$')
        {
            accept();
            // Allow '$'*, so that users can for instance create their own `init$$` symbol if they want to.
            while (m_char == U'$')
            { accept(); }
        }

        return Token(m_spelling, m_line_no, true);
    }

    // Scan a number.
    if (isdigit(m_char))
    {
        accept();
        scan_digits();

        if (m_char == U'.')
        {
            accept();
            scan_digits();
			return Token(Token::Lexeme::FloatLiteral, m_spelling, m_line_no);
        }

        return Token(Token::Lexeme::IntegerLiteral, m_spelling, m_line_no);
    }

    switch (m_char)
    {
    case U'=':
    {
        accept();

        if (m_char == U'=')
        {
            accept();
            return Token(m_spelling, m_line_no, false);
        }
        else
        {
            return Token(m_spelling, m_line_no, false);
        }
    }
    case U'\n':
	{
		accept();
		// We return the previous line because after accept() we are already pointing to the beginning of the following line.
		return Token(Token::Lexeme::Eol, String(), m_line_no - 1);
	}
    case U'"':
    {
        scan_string(U'"');
        return Token(Token::Lexeme::StringLiteral, m_spelling, m_line_no);
    }
    case U'\'':
	{
		scan_string(U'\'');
		return Token(Token::Lexeme::StringLiteral, m_spelling, m_line_no);
	}
    case Token::ETX:
    {
        // Don't accept token since we reached the end.
        return Token(Token::Lexeme::Eot, "EOT", m_line_no);
    }
    case U'(':
    {
	    accept();
    	return Token(Token::Lexeme::LParen, "(", m_line_no);
    }
    case U')':
    {
	    accept();
	    return Token(Token::Lexeme::RParen, ")", m_line_no);
    }
    case U'{':
    {
	    accept();
	    return Token(Token::Lexeme::LCurl, "{", m_line_no);
    }
    case U'}':
    {
	    accept();
	    return Token(Token::Lexeme::RCurl, "}", m_line_no);
    }
    case U'[':
    {
	    accept();
	    return Token(Token::Lexeme::LSquare, "[", m_line_no);
    }
    case U']':
    {
	    accept();
	    return Token(Token::Lexeme::RSquare, "]", m_line_no);
    }
    case U'+':
    {
    	accept();
    	if (m_char == U'=')
    	{
    		accept();
    		return Token(Token::Lexeme::OpAssignPlus, "+=", m_line_no);
    	}
    	return Token(Token::Lexeme::OpPlus, "+", m_line_no);
    }
    case U'-':
    {
	    accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignMinus, "-=", m_line_no);
		}
	    return Token(Token::Lexeme::OpMinus, "-", m_line_no);
    }
    case U'*':
    {
	    accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignStar, "*=", m_line_no);
		}
	    return Token(Token::Lexeme::OpStar, "*", m_line_no);
    }
    case U'/':
    {
	    accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignSlash, "/=", m_line_no);
		}
	    return Token(Token::Lexeme::OpSlash, "/", m_line_no);
    }
    case U'^':
	{
		accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignPower, "^=", m_line_no);
		}
		return Token(Token::Lexeme::OpPower, "^", m_line_no);
	}
   	case U'%':
	{
		accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignMod, "%=", m_line_no);
		}
		return Token(Token::Lexeme::OpMod, "%", m_line_no);
	}
    case U'&':
    {
	    accept();
		if (m_char == U'=')
		{
			accept();
			return Token(Token::Lexeme::OpAssignConcat, "&=", m_line_no);
		}
	    return Token(Token::Lexeme::OpConcat, "&", m_line_no);
    }
    case U',':
    {
	    accept();
	    return Token(Token::Lexeme::Comma, ",", m_line_no);
    }
    case U';':
    {
	    accept();
	    return Token(Token::Lexeme::Semicolon, ";", m_line_no);
    }
    case U':':
    {
	    accept();
	    return Token(Token::Lexeme::Colon, ":", m_line_no);
    }
    case U'.':
    {
        accept();
        return Token(Token::Lexeme::Dot, ".", m_line_no);
    }
    case U'#':
    {
        // Skip comment and read next token
        do skip(); while (m_char != '\n' && m_char != Token::ETX);
        if (m_char != Token::ETX) skip(); // handle case when the file ends with a comment without a new line.
        goto RETRY;
    }
    case U'!':
    {
        accept();

        if (m_char == U'=')
        {
            accept();
            return Token(Token::Lexeme::OpNotEqual, m_spelling, m_line_no);
        }

        report_error("invalid token");
        break; // never reached.
    }
    case U'<':
    {
        accept();

        if (m_char == U'=')
        {
            accept();

            if (m_char == U'>')
            {
                accept();
                return Token(Token::Lexeme::OpCompare, m_spelling, m_line_no);
            }
            else
            {
                return Token(Token::Lexeme::OpLessEqual, m_spelling, m_line_no);
            }
        }
        else
        {
            return Token(Token::Lexeme::OpLessThan, m_spelling, m_line_no);
        }
    }
    case U'>':
    {
        accept();

        if (m_char == U'=')
        {
            accept();
            return Token(Token::Lexeme::OpGreaterEqual, m_spelling, m_line_no);
        }
        else
        {
            return Token(Token::Lexeme::OpGreaterThan, m_spelling, m_line_no);
        }
    }
	case U'@':
	{
		accept();
		return Token(Token::Lexeme::OpAt, m_spelling, m_line_no);
	}

    default:
        break;
    }

    report_error("invalid token");

    return Token();
}

void Scanner::report_error(const std::string &hint, intptr_t offset, const char *error_type)
{
	assert(m_line_no != 0);
	String line = m_source->get_line(m_line_no);
	// These must be computed before trimming, since the iterator will be invalidated
	auto step_back = intptr_t(m_pos > line.begin());
	auto left_space = intptr_t(m_pos - line.begin());

	line.rtrim();

	// normalize tabs
	auto old_size = line.size();
	line.replace("\t", "    ");
	auto additional_padding = line.size() - old_size;

	// Set spacing to the location of the error
	auto beginning = step_back; // move 1 char back, unless we are at the beginning
	intptr_t count = left_space + additional_padding - offset - beginning;
	String filler;
	filler.fill(U' ', count);

	auto message = utils::format("[% error] File \"%\" at line %\n%\n%^",
	                             error_type, m_source->filename(), m_line_no, line, filler);

	if (!hint.empty())
	{
		message.append("\nHint: ");
		message.append(hint);
	}

	throw RuntimeError(m_line_no, message);
}

bool Scanner::check_space(char32_t c)
{
	switch (c)
	{
		case ' ':
		case '\t':
		case '\r':
		case '\f':
		case '\v':
			return true;
		default:
			return false;
	}
}

} // namespace phonometrica