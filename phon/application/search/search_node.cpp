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
 * Created: 03/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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

SearchConstraint::SearchConstraint(AutoProtocol p, int context_length, int index, int layer_index, const String &layer_name,
		bool case_sensitive, SearchConstraint::Opcode op, SearchConstraint::Relation rel, String value) :
		m_protocol(std::move(p)), index(index), layer_index(layer_index),
		value_pattern(op == Opcode::Matches ? Regex(value, case_sensitive ? Regex::None : Regex::Caseless) : Regex()),
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
	String s("#");
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
#if PHON_MACOS
            results.insert(*it); it++;
#else
			results.insert(std::move(subset.extract(it++)));
#endif
		}
	}

	return results;
}

QueryMatchSet SearchConstraint::find_matches(const AutoAnnotation &annot, int layer_index, std::true_type)
{
	auto &events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;
	static String sep(" ");
	PHON_LOG("search for text matches in " << annot->path());
	std::string_view p = annot->path();

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

			// For regular expressions only, we split the match into fields if there is a query protocol.
			std::shared_ptr<Concordance> conc;
			if (m_protocol)
			{
				auto count = value_pattern.count();
				assert(count == m_protocol->field_count());
				Array<String> fields(count);
				for (intptr_t c = 1; c <= count; c++) {
					fields.append(value_pattern.capture(c));
				}

				conc = std::make_shared<ProtocolConcordance>(annot, layer_index, event, match, ++match_index, left,
						right, std::move(fields));
			}
			else
			{
				conc = std::make_shared<Concordance>(annot, layer_index, event, match, ++match_index, left, right);
			}

			matches.insert(std::move(conc));
		}
	}

	return matches;
}

QueryMatchSet SearchConstraint::find_matches(const AutoAnnotation &annot, int layer_index, std::false_type)
{
	auto &events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;
	static String sep(" ");

	for (intptr_t i = 1; i <= events.size(); i++)
	{
		auto &event = events[i];
		int match_index = 0;
		auto &label = event->text();
		auto it = label.begin();
		intptr_t grapheme_count = case_sensitive ? 0 : value.grapheme_count();

		if (case_sensitive)
		{
			while((it = label.find(value, it)) != label.end())
			{
				String::const_iterator end = it + value.size();
				auto left = Annotation::left_context(events, i, it, context_length, sep);
				auto right = Annotation::right_context(events, i, end, context_length, sep);
				matches.insert(std::make_shared<Concordance>(annot, layer_index, event, value, ++match_index, left, right));
				it = end;
			}
		}
		else
		{
			while((it = label.ifind(value, it)) != label.end())
			{
				String::const_iterator end = it;
				label.advance(end, grapheme_count);
				String text(it, intptr_t(end-it));
				auto left = Annotation::left_context(events, i, it, context_length, sep);
				auto right = Annotation::right_context(events, i, end, context_length, sep);
				matches.insert(std::make_shared<Concordance>(annot, layer_index, event, text, ++match_index, left, right));
				it = end;
			}
		}
	}

	return matches;
}

} // namespace phonometrica
