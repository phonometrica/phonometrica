/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Purpose: tokens for Phonometrica's scripting language.                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TOKEN_HPP
#define PHONOMETRICA_TOKEN_HPP

#include <phon/string.hpp>

namespace phonometrica {

struct Token final
{
	// end of text character in ASCII
	static const char32_t ETX = 3;

	// Tokens of the language. Don't forget to update the token names.
	enum class Lexeme
	{
		Unknown = 0,
		And,
		As,
		Assert,
		Break,
		Class,
		Continue,
		Debug,
		Do,
		Downto,
		Else,
		Elsif,
		End,
		Explicit,
		False,
		Field,
		For,
		Foreach,
		Function,
		If,
		In,
		Inherits,
		Local,
		Method,
		Nan,
		Not,
		Null,
		Option,
		Or,
		Pass,
		Print,
		Ref,
		Repeat,
		Return,
		Step,
		Super,
		Then,
		This,
		Throw,
		To,
		True,
		Until,
		While,

		OpAssign,
		OpAssignConcat,
		OpAssignMinus,
		OpAssignMod,
		OpAssignPlus,
		OpAssignPower,
		OpAssignSlash,
		OpAssignStar,
		OpAt,
		OpCompare,
		OpConcat,
		OpEqual,
		OpGreaterEqual,
		OpGreaterThan,
		OpLessEqual,
		OpLessThan,
		OpMinus,
		OpMod,
		OpNotEqual,
		OpPlus,
		OpPower,
		OpSlash,
		OpStar,

		Comma,
		Colon,
		Dot,
		LParen,
		RParen,
		LCurl,
		RCurl,
		LSquare,
		RSquare,
		Semicolon,

		Identifier,
		IntegerLiteral,
		FloatLiteral,
		StringLiteral,

		Eol, // end of line
		Eot // end of text
	};
	
	Token() = default;

	Token(const Token &other) = default;

	Token(Token &&other) = default;

	Token(const String &spelling, intptr_t line, bool ident);

	Token(Lexeme type, const String &spelling, intptr_t line);

	~Token() = default;

	Token &operator=(const Token &) = default;

	Token &operator=(Token &&) = default;

	intptr_t size() const { return id == Lexeme::Eot ? 0 : spelling.size(); }

	bool is_eot() const {return id == Lexeme::Eot; }

	String to_string() const;

	bool is_block_end() const;

	bool is_separator() const { return id == Lexeme::Eol || id == Lexeme::Semicolon; }

	bool is(Lexeme c) const { return id == c; }

	static void initialize();

	static String get_name(Lexeme c);

	String get_name() const;

	String spelling;

	intptr_t line_no = 0;

	Lexeme id = Lexeme::Unknown;

};

} // namespace phonometrica

#endif // PHONOMETRICA_TOKEN_HPP
