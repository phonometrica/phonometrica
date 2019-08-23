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
 * Created: 21/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: regular expression object, built on top of oniguruma.                                                     *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_REGEX_HPP
#define PHONOMETRICA_REGEX_HPP

#include <phon/string.hpp>
#include <oniguruma.h>

namespace phonometrica {

class Regex final
{
public:
	enum Option {
        None           = ONIG_OPTION_NONE,
        ICase          = ONIG_OPTION_IGNORECASE,
        Multiline      = ONIG_OPTION_MULTILINE,
        Extend         = ONIG_OPTION_EXTEND,
        Greedy         = ONIG_OPTION_FIND_LONGEST,
        Capture        = ONIG_OPTION_CAPTURE_GROUP,
        NoCapture      = ONIG_OPTION_DONT_CAPTURE_GROUP
	};

    Regex() noexcept;

    explicit Regex(const String &pattern);

    Regex(const String &pattern, int flags);

    Regex(const String &pattern, const String &flags);

	Regex(Regex &&other) noexcept;

	~Regex();

	String pattern() const;

	String subject() const;

	int flags() const;

	bool match(const String &subject, intptr_t from = 1);
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

private:

    String error_message(int error, OnigErrorInfo *einfo);

    void check_capture(intptr_t nth) const;

    int parse_flags(const String &options);

    regex_t *m_regex;

    OnigRegion *m_region;

    int  m_flags;

    int m_match_result;

    String m_pattern;

    String m_subject;
};

} // namespace phonometrica

#endif // PHONOMETRICA_REGEX_HPP
