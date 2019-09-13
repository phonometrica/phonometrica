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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/regex.hpp>
#include <phon/error.hpp>
#include <phon/third_party/utf8/utf8.h>
#include "regex.hpp"


namespace phonometrica {

Regex::Regex() noexcept:
    m_regex(nullptr), m_region(nullptr), m_flags(0), m_match_result(ONIG_MISMATCH)
{

}

Regex::Regex(const String &pattern) :
    Regex(pattern, None)
{

}

Regex::Regex(const String &pattern, int flags) :
    m_pattern(pattern)
{
    m_match_result = ONIG_MISMATCH;
    m_flags = flags;

    if (pattern.empty()) {
        throw error("[Regex error] Empty regular expression pattern");
    }

    const UChar *pattern_start = reinterpret_cast<const UChar*>(pattern.begin());
    const UChar *pattern_end = reinterpret_cast<const UChar*>(pattern.end());
    OnigErrorInfo einfo;
    int result = onig_new(&m_regex, pattern_start, pattern_end,
                          ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL, &einfo);

    if (result != ONIG_NORMAL)
    {
        auto msg = error_message(result, &einfo);
        throw error(msg);
    }

    m_region = onig_region_new();
}

Regex::Regex(const String &pattern, const String &flags) :
    Regex(pattern, parse_flags(flags))
{

}

String Regex::error_message(int error, OnigErrorInfo *einfo)
{
    char msg[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str((UChar* )msg, error, &einfo);

    return msg;
}

Regex::Regex(Regex &&other) noexcept :
        m_pattern(std::move(other.m_pattern)), m_subject(std::move(other.m_subject))
{
	m_regex = other.m_regex;
    m_region = other.m_region;
    other.m_regex = nullptr;
    other.m_region = nullptr;
    m_match_result = other.m_match_result;
	m_flags = other.m_flags;
}

Regex::~Regex()
{
    if (m_region) onig_region_free(m_region, 1);
    if (m_regex) onig_free(m_regex);
}

String Regex::subject() const
{
    return m_subject;
}

int Regex::flags() const
{
	return m_flags;
}

bool Regex::match(const String &subject, intptr_t from)
{
    if (subject.empty()) return false;
	return match(subject, subject.index_to_iter(from));
}

bool Regex::match(const String &subject, String::const_iterator from)
{
	m_subject = subject;
    onig_region_clear(m_region);

    const UChar *str = reinterpret_cast<const UChar*>(subject.begin());
    const UChar *end = reinterpret_cast<const UChar*>(subject.end());
    const UChar *start = reinterpret_cast<const UChar*>(from);

    m_match_result = onig_search(m_regex, str, end, start, end, m_region, ONIG_OPTION_DEFAULT);

    return has_match();
}

bool Regex::has_match() const
{
    return m_match_result != ONIG_MISMATCH;
}

intptr_t Regex::count() const
{
    assert(m_regex && m_region);
    return has_match() ? onig_number_of_captures(m_regex) : 0;
}

bool Regex::empty() const
{
    return m_regex == nullptr;
}

String Regex::capture(intptr_t nth) const
{
    check_capture(nth);
    auto start = m_region->beg[nth];
    auto len = m_region->end[nth] - start;
    const char *s = m_subject.data() + start;

    return String(s, len);
}

void Regex::check_capture(intptr_t nth) const
{
    if (has_match() && nth > count()) {
		throw error("Invalid capture index % (regex has % captures)", nth, count());
	}
}

intptr_t Regex::capture_start(intptr_t nth, bool utf8) const
{
    check_capture(nth);
    auto pos = m_region->beg[nth];

    if (utf8) {
        pos = utf8::unchecked::distance(m_subject.begin(), m_subject.begin() + pos);
    }

    return pos + 1; // base 1
}

intptr_t Regex::capture_end(intptr_t nth, bool utf8) const
{
    check_capture(nth);
    auto pos = m_region->end[nth];

    if (utf8) {
        pos = utf8::unchecked::distance(m_subject.begin(), m_subject.begin() + pos);
    }

    return pos + 1; // base 1
}

int Regex::parse_flags(const String &options)
{
	int flags = None;

	for (auto &s : options.split("|")) {
        if (s == "icase") {
            flags |= ICase;
		}
		else if (s == "multiline") {
			flags |= Multiline;
		}
        else if (s == "extend") {
            flags |= Extend;
		}
        else if (s == "greedy") {
            flags |= Greedy;
		}
        else if (s == "capture") {
            flags |= Capture;
        }
        else if (s == "nocapture") {
            flags |= NoCapture;
        }
        else if (s == "none") {
            //flags |= None;
        }
        else {
            throw error("Unknown regex flag: \"%\"", s);
        }
	}

	return flags;
}

String::const_iterator Regex::capture_start_iter(intptr_t nth) const
{
	return m_subject.begin() + capture_start(nth, false) - 1;
}

String::const_iterator Regex::capture_end_iter(intptr_t nth) const
{
    return m_subject.begin() + capture_end(nth, false) - 1;
}

String Regex::pattern() const
{
    return m_pattern;
}


} // namespace phonometrica
