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
 * Created: 21/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
