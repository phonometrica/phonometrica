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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Text match in an annotation.                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_MATCH_HPP
#define PHONOMETRICA_TEXT_MATCH_HPP

#include <phon/application/conc/match.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

class TextMatch : public Match
{
public:

	TextMatch() = default;

	double start_time() const;

	double end_time() const;

	double duration() const { return 1000 * (end_time() - start_time()); }

	bool operator<(const TextMatch &other) const;

	const AutoAnnotation &annotation() const { return m_annot; }

	size_t hash() const;

	String left(intptr_t len) const;

	String right(intptr_t len) const;

private:

	// Annotation in which the match was found.
	AutoAnnotation m_annot;

};

} // namespace phonometrica

#endif // PHONOMETRICA_TEXT_MATCH_HPP
