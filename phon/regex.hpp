/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 21/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: regular expression object, built on top of PCRE2.                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_REGEX_HPP
#define PHONOMETRICA_REGEX_HPP

#include <phon/string.hpp>
#include <pcre2.h>

namespace phonometrica {

class Regex final
{
public:
	enum Option {
		None           = 0,
		Caseless       = PCRE2_CASELESS,
		Multiline      = PCRE2_MULTILINE,
		DotAll         = PCRE2_DOTALL,
		Extended       = PCRE2_EXTENDED,
		Anchored       = PCRE2_ANCHORED,
		DollarEndOnly  = PCRE2_DOLLAR_ENDONLY,
		Ungreedy       = PCRE2_UNGREEDY,
	};

	enum Jit : uint8_t {
		NoJit       = 0,
		Complete    = PCRE2_JIT_COMPLETE,
		PartialSoft = PCRE2_JIT_PARTIAL_SOFT,
		PartialHard = PCRE2_JIT_PARTIAL_HARD
	};

    Regex() = default;

    explicit Regex(const String &pattern);

    Regex(const String &pattern, int flags, Jit jit = Complete);

    Regex(const String &pattern, const String &flags);

	Regex(Regex &&other) noexcept;

	~Regex();

	String pattern() const;

	String subject() const;

	int flags() const;

	bool match(const String &subject);
	bool match(const String &subject, intptr_t from);
	bool match(const String &subject, String::const_iterator from);

	bool has_match() const;

	intptr_t count() const;

    bool empty() const;

	String capture(intptr_t nth) const;

	// Get the beginning and end indices of a capture. If utf8 is true,
	// the returned index should be interpreted as 1-based code point index.
	// Otherwise, it is a 1-based code unit index.
	intptr_t capture_start(intptr_t nth, bool utf8 = true) const;
	intptr_t capture_end(intptr_t nth, bool utf8 = true) const;

	String::const_iterator capture_start_iter(intptr_t nth) const;
	String::const_iterator capture_end_iter(intptr_t nth) const;

	Jit jit_flag() const;

	bool is_jit() const;

	bool jit(Jit flag);

private:

    String error_message(int error);

    void check_capture(intptr_t nth) const;

    int parse_flags(const String &options);

	pcre2_code *m_regex = nullptr;

	pcre2_match_data *m_match_data = nullptr;

	String m_subject;

	String m_pattern;

	size_t m_error_offset = 0;

	int m_error_code = 0;

	int m_flags = 0;

	int m_rc = 0;

	Jit m_jit = Jit::NoJit;
};

} // namespace phonometrica

#endif // PHONOMETRICA_REGEX_HPP
