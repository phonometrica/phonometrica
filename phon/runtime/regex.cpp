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
 * Created: 21/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/regex.hpp>
#include <phon/error.hpp>
#include <phon/third_party/utf8/utf8.h>

namespace phonometrica {

static const uint32_t OVECCOUNT = 30;
static const size_t ERROR_BUFFER_SIZE = 512;


Regex::Regex(const String &pattern) :
    Regex(pattern, None)
{

}

Regex::Regex(const String &pattern, int flags, Regex::Jit jit) :
    m_pattern(pattern), m_flags(flags), m_jit(jit)
{
	m_regex = pcre2_compile((PCRE2_SPTR) pattern.data(), pattern.size(),
	                        (uint32_t) flags|PCRE2_UTF, &m_error_code, &m_error_offset, nullptr);

	if (m_regex == nullptr) {
		throw error("compilation of regular expression failed at position %: %",
		            m_error_offset + 1, error_message(m_error_code));
	}

	this->jit(jit);
	m_match_data = pcre2_match_data_create(OVECCOUNT, nullptr);
}

Regex::Regex(const String &pattern, const String &flags) :
    Regex(pattern, parse_flags(flags))
{

}

String Regex::error_message(int error)
{
	PCRE2_UCHAR buffer[ERROR_BUFFER_SIZE];
	pcre2_get_error_message(error, buffer, ERROR_BUFFER_SIZE);

	return String::format("[Regex error] %s", reinterpret_cast<const char*>(buffer));
}

Regex::Regex(Regex &&other) noexcept :
		m_subject(std::move(other.m_subject)), m_pattern(std::move(other.m_pattern))
{
	m_regex = other.m_regex;
	m_match_data = other.m_match_data;
	m_error_code = other.m_error_code;
	m_error_offset = other.m_error_offset;
	m_flags = other.m_flags;
	m_rc = other.m_rc;
	m_jit = other.m_jit;

	other.m_regex = nullptr;
	other.m_match_data = nullptr;
}

Regex::~Regex()
{
	if (m_match_data) {
		pcre2_match_data_free(m_match_data);
	}

	if (m_regex) {
		pcre2_code_free(m_regex);
	}
}

String Regex::subject() const
{
    return m_subject;
}

int Regex::flags() const
{
	return m_flags;
}

bool Regex::match(const String &subject)
{
	return match(subject, subject.begin());
}

bool Regex::match(const String &subject, intptr_t from)
{
	if (subject.empty()) return false;
	auto it = subject.index_to_iter(from);
	return match(subject, it);
}

bool Regex::match(const String &subject, String::const_iterator from)
{
	m_subject = subject;
	size_t index = from - subject.begin();

	if (m_jit == NoJit) {
		m_rc = pcre2_match(m_regex, (PCRE2_SPTR) subject.data(), subject.size(), index, 0, m_match_data, nullptr);
	}
	else {
		m_rc = pcre2_jit_match(m_regex, (PCRE2_SPTR) subject.data(), subject.size(), index, 0, m_match_data, nullptr);
	}

	// -1 is used to indicate there is no match, so we don't want to trigger an error for that
	if (m_rc < -1) {
		auto msg = error_message(m_rc);
		throw error(msg);
	}

	return has_match();
}

bool Regex::has_match() const
{
 	return m_rc > 0;
}

intptr_t Regex::count() const
{
	return m_rc > 0 ? intptr_t(m_rc) - 1 : 0;
}

bool Regex::empty() const
{
    return m_regex == nullptr;
}

String Regex::capture(intptr_t nth) const
{
	check_capture(nth);
	PCRE2_SIZE *ovec = pcre2_get_ovector_pointer(m_match_data);

	const char *substring = subject().data() + ovec[2 * nth];
	auto len = intptr_t(ovec[2 * nth + 1] - ovec[2 * nth]);

	return String(substring, len);
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
	PCRE2_SIZE *ovec = pcre2_get_ovector_pointer(m_match_data);
	auto pos = (intptr_t) ovec[2 * nth];

	if (utf8) {
        pos = utf8::unchecked::distance(m_subject.begin(), m_subject.begin() + pos);
    }

	return pos + 1; // base 1
}

intptr_t Regex::capture_end(intptr_t nth, bool utf8) const
{
	check_capture(nth);
	PCRE2_SIZE *ovec = pcre2_get_ovector_pointer(m_match_data);
	auto pos = (intptr_t) ovec[2 * nth + 1];

	if (utf8) {
        pos = utf8::unchecked::distance(m_subject.begin(), m_subject.begin() + pos);
    }

	return pos + 1; // base 1
}

int Regex::parse_flags(const String &options)
{
	int flags = None;

	for (auto &s : options.split("|"))
	{
		if (s == "caseless") {
			flags |= Caseless;
		}
		else if (s == "multiline") {
			flags |= Multiline;
		}
		else if (s == "dotall") {
			flags |= DotAll;
		}
		else if (s == "extended") {
			flags |= Extended;
		}
		else if (s == "anchored") {
			flags |= Anchored;
		}
		else if (s == "dollar_endonly") {
			flags |= DollarEndOnly;
		}
		else if (s == "ungreedy") {
			flags |= Ungreedy;
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

Regex::Jit Regex::jit_flag() const
{
	return m_jit;
}

bool Regex::is_jit() const
{
	return m_jit != NoJit;
}

bool Regex::jit(Regex::Jit flag)
{
	bool ok = false;

	if (flag != NoJit) {
		ok = (pcre2_jit_compile(m_regex, flag) == 0);

		// Disable JIT on error.
		if (!ok) {
			m_jit = NoJit;
		}
	}

	return ok;
}

} // namespace phonometrica
