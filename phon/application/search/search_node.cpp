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
 * Created: 03/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <algorithm>
#include <functional>
#include <phon/runtime/runtime.hpp>
#include <phon/application/search/search_node.hpp>
#include <phon/regex.hpp>

namespace phonometrica {

QueryMatchSet SearchOperator::filter(const AutoAnnotation &annot, const QueryMatchSet &matches)
{
	if (opcode == Opcode::And)
	{
		auto results = lhs->filter(annot, matches);
		return rhs->filter(annot, results);
	}
	else if (opcode == Opcode::Or)
	{
		QueryMatchSet x = lhs->filter(annot, matches);
		QueryMatchSet y = rhs->filter(annot, matches);
		QueryMatchSet results;
		std::set_union(x.begin(), x.end(), y.begin(), y.end(), std::inserter(results, results.begin()));

		return results;
	}
	else
	{
		// TODO: not operator
		return QueryMatchSet();
	}
}

String SearchOperator::to_string() const
{
	assert(opcode != Opcode::Null);

	if (opcode == Opcode::Not)
	{
		String s(" NOT ");
		return s.append(lhs->to_string());
	}

	String s("(");
	s.append(lhs->to_string());

	if (opcode == Opcode::And)
		s.append(" AND ");
	else if (opcode == Opcode::Or)
		s.append(" OR ");

	assert(rhs);
	s.append(rhs->to_string());
	s.append(')');

	return s;
}


//----------------------------------------------------------------------------------------------------------------------

SearchConstraint::SearchConstraint(int context_length, int index, int layer_index, const String &layer_name, bool case_sensitive,
                                   SearchConstraint::Opcode op, SearchConstraint::Relation rel, String value) :
		index(index), layer_index(layer_index),
		value_pattern(op == Opcode::Matches ? Regex(value, case_sensitive ? Regex::None : Regex::ICase) : Regex()),
		layer_pattern(layer_name.empty() ? nullptr : std::make_unique<Regex>(layer_name)),
		case_sensitive(case_sensitive), op(op), relation(rel), value(std::move(value))
{
	this->context_length = context_length;
}

QueryMatchSet SearchConstraint::filter(const AutoAnnotation &annotation, const QueryMatchSet &)
{
	return search(annotation);
}

String SearchConstraint::to_string() const
{
	String s("e");
	s.append(String::convert(intptr_t(index)));
	return s;
//	auto s = utils::format("(layer_index = %, layer_name = \"%\", case_sensitive = %, op = %, relation = %, value = \"%\")",
//	                       layer_index, layer_name, case_sensitive, static_cast<int>(op), static_cast<int>(relation), value);
//
//	return String(s);
}

QueryMatchSet SearchConstraint::search(const AutoAnnotation &annot)
{
	QueryMatchSet results;

	for (auto &layer : annot->layers())
	{
		QueryMatchSet subset;

		if (layer_pattern)
		{
			if (layer_pattern->match(layer->label))
			{
				if (op == Opcode::Matches)
					subset = find_matches(annot, layer->index, std::true_type());
				else
					subset = find_matches(annot, layer->index, std::false_type());
			}
		}
		else if (layer_index == 0 || layer_index == layer->index)
		{
			if (op == Opcode::Matches)
				subset = find_matches(annot, layer->index, std::true_type());
			else
				subset = find_matches(annot, layer->index, std::false_type());
		}

		for (auto it = subset.begin(); it != subset.end(); )
		{
			results.insert(std::move(subset.extract(it++)));
		}
	}

	return results;
}

QueryMatchSet SearchConstraint::find_matches(const AutoAnnotation &annot, int layer_index, std::true_type)
{
	EventList events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;
	static String sep(" ");
	int found = 0;

	for (intptr_t i = 1; i <= events.size(); i++)
	{
		auto &event = events[i];
		int match_index = 0;
		auto &label = event->text();
		auto it = label.begin();

		while (value_pattern.match(label, it))
		{
			auto match = value_pattern.capture(0);
			auto start = value_pattern.capture_start_iter(0);
			auto end = value_pattern.capture_end_iter(0);
			it = end;
			auto left = Annotation::left_context(events, i, start, context_length, sep);
			auto right = Annotation::right_context(events, i, end, context_length, sep);
			assert(left.grapheme_count() <= context_length);
			assert(right.grapheme_count() <= context_length);
			matches.insert(std::make_shared<Concordance>(annot, layer_index, event, match, ++match_index, left, right));
			found++;
		}
	}
	auto s = matches.size();

	return matches;
}

QueryMatchSet SearchConstraint::find_matches(const AutoAnnotation &annot, int layer_index, std::false_type)
{
	EventList events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;

	for (intptr_t i = 1; i <= events.size(); i++)
	{
		auto &event = events[i];
		int match_index = 0;
		auto &label = event->text();
		auto it = label.begin();

		if (case_sensitive)
		{
			while((it = label.find(value, it)) != label.end())
			{
				matches.insert(std::make_shared<QueryMatch>(annot, layer_index, event, value, ++match_index));
			}
		}
		else
		{
			while((it = label.ifind(value, it)) != label.end())
			{
				matches.insert(std::make_shared<QueryMatch>(annot, layer_index, event, value, ++match_index));
			}
		}
	}

	return matches;
}

} // namespace phonometrica
