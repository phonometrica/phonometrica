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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/hashmap.hpp>
#include <phon/runtime/compiler/token.hpp>

namespace phonometrica {

static bool tokens_initialized = false;

static Hashmap<String, Token::Lexeme> token_codes;

static Array<String> token_names = {
        "unknown",
        "and",
        "as",
        "assert",
        "break",
        "class",
        "continue",
        "debug",
        "do",
        "downto",
        "else",
        "elsif",
        "end",
        "explicit",
        "false",
        "field",
        "for",
        "foreach",
        "function",
        "if",
        "in",
        "inherits",
        "local",
        "method",
        "nan",
        "not",
        "null",
        "option",
        "or",
        "pass",
        "print",
        "ref",
        "repeat",
        "return",
        "step",
        "super",
        "then",
        "this",
        "throw",
        "to",
        "true",
        "until",
        "while",
        "=",
        "&=",
        "-=",
        "%=",
		"+=",
		"^=",
		"/=",
		"*=",
        "@",
        "<=>",
        "&",
        "==",
        ">=",
        ">",
        "<=",
        "<",
        "-",
        "%",
        "!=",
        "+",
        "^",
		"/",
        "*",
        ",",
        ":",
        ".",
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        ";",
        "identifier",
        "integer literal",
        "float literal",
        "string literal",
        "end of line",
        "end of text"
};

Token::Token(const String &spelling, intptr_t line, bool ident) :
        spelling(spelling), line_no(line)
{
    auto it = token_codes.find(spelling);

    if (it == token_codes.end())
    {
        this->id = ident ? Lexeme::Identifier : Lexeme::Unknown;
        assert(ident || id != Lexeme::Unknown);
    }
    else
    {
        this->id = it->second;
    }
}

Token::Token(Lexeme type, const String &spelling, intptr_t line) :
        spelling(spelling), line_no(line), id(type)
{

}

String Token::to_string() const
{
    if (id == Lexeme::StringLiteral)
    {
        return String::format("\"%s\"", spelling.data());
    }
    else
    {
        return spelling;
    }
}

bool Token::is_block_end() const
{
    switch (id)
    {
    case Lexeme::End:
    case Lexeme::Else:
    case Lexeme::Elsif:
    case Lexeme::Eot:
        return true;
    default:
        return false;
    }
}

void Token::initialize()
{
    if (tokens_initialized)
    {
        throw error("[Internal error] Tokens must be initialized only once");
    }

    auto last_token = static_cast<int>(Lexeme::Eot);

    for (int i = 0; i <= last_token; ++i)
    {
        auto tok = static_cast<Lexeme>(i);
        auto &name = token_names[i + 1];
        token_codes[name] = tok;
    }

    tokens_initialized = true;
}

String Token::get_name(Lexeme c)
{
    return token_names[static_cast<int>(c) + 1];
}

String Token::get_name() const
{
    return Token::get_name(id);
}

} // namespace phonometrica