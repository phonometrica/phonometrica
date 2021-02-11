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
 * Purpose: a Match represents a line of concordance. Each match has one or more targets: For simple queries with      *
 * a single constraint, there is a single target; for complex queries with two or more constraints, the match contains *
 * a linked list of targets.                                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MATCH_HPP
#define PHONOMETRICA_MATCH_HPP

#include <phon/application/annotation.hpp>

namespace phonometrica {

class Match final
{
public:

	struct Target
	{
		Target(const AutoEvent &e, String value, intptr_t layer, intptr_t offset, bool is_ref);

		// Event where the match occurred.
		AutoEvent event;

		// Matched text.
		String value;

		// Index of the layer in which the match was found.
		int layer;

		// Offset where the match occurred in the event.
		int offset;

		// Does this target come from a reference constraint?
		bool is_reference;

		// Next target in a complex query (null in a simple query).
		std::unique_ptr<Target> next;
	};

	Match(const AutoAnnotation &annot, std::unique_ptr<Target> t);

	const AutoEvent &get_event(intptr_t i) const;

	intptr_t get_layer(intptr_t i) const;

	intptr_t get_offset(intptr_t i) const;

	String get_value(intptr_t i) const;

	const AutoAnnotation &annotation() const;

	Target & last_target();

	Target *reference_target() const;

protected:

	Target *get(intptr_t i) const;

	// Annotation in which the match was found.
	AutoAnnotation m_annot;

	// First (and possibly unique) target.
	std::unique_ptr<Target> m_target;
};

using AutoMatch = std::unique_ptr<Match>;

} // namespace phonometrica



#endif // PHONOMETRICA_MATCH_HPP
