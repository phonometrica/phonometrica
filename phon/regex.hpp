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
 * Purpose: regular expression object, built on top of oniguruma.                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
