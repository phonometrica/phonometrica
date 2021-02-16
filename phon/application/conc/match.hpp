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
#include <phon/utils/xml.hpp>

namespace phonometrica {

class Match final
{
public:

	struct Target
	{
		Target(const AutoEvent &e, String value, intptr_t layer, intptr_t offset, bool is_ref);

		double start_time() const { return event->start_time(); }

		double end_time() const { return event->end_time(); }

		bool operator==(const Target &other) const;
		bool operator!=(const Target &other) const;
		bool operator<(const Target &other) const;

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

	Match() = default;

	Match(const Match &other);

	Match(const Handle<Annotation> &annot, std::unique_ptr<Target> t);

	const AutoEvent &get_event(intptr_t i) const;

	double get_start_time(intptr_t i) const;

	double get_end_time(intptr_t i) const;

	intptr_t get_layer(intptr_t i) const;

	intptr_t get_offset(intptr_t i) const;

	String get_value(intptr_t i) const;

	const Handle<Annotation> &annotation() const;

	Target & last_target();

	Target *reference_target() const;

	void to_xml(xml_node root) const;

	bool valid();

	void append(std::unique_ptr<Target> next);

	bool operator==(const Match &other) const;
	bool operator!=(const Match &other) const;
	bool operator<(const Match &other) const;

	bool update(bool &modified);

protected:

	Target *get(intptr_t i) const;

	// Annotation in which the match was found.
	Handle<Annotation> m_annot;

	// First (and possibly unique) target.
	std::unique_ptr<Target> m_target;
};

using AutoMatch = std::unique_ptr<Match>;


struct MatchLess
{
	bool operator()(const AutoMatch &lhs, const AutoMatch &rhs) const
	{
		return *lhs < *rhs;
	}
};

} // namespace phonometrica



#endif // PHONOMETRICA_MATCH_HPP
