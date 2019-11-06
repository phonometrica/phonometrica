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
#include <phon/regex.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/application/search/search_node.hpp>
#include <phon/application/search/query.hpp>
#include <phon/analysis/weenink.hpp>
#include <phon/analysis/speech_utils.hpp>
#include <phon/gui/formant_search_box.hpp> // only for the settings


namespace phonometrica {

QueryMatchSet SearchOperator::filter(Settings *settings, const AutoAnnotation &annot, const QueryMatchSet &matches)
{
	if (opcode == Opcode::And)
	{
		auto results = lhs->filter(settings, annot, matches);
		return rhs->filter(settings, annot, results);
	}
	else if (opcode == Opcode::Or)
	{
		QueryMatchSet x = lhs->filter(settings, annot, matches);
		QueryMatchSet y = rhs->filter(settings, annot, matches);
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

QueryMatchSet SearchConstraint::filter(Settings *settings, const AutoAnnotation &annotation, const QueryMatchSet &)
{
	return search(settings, annotation);
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

QueryMatchSet SearchConstraint::search(Settings *settings, const AutoAnnotation &annot)
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
					subset = find_matches(settings, annot, layer->index, std::true_type());
				else
					subset = find_matches(settings, annot, layer->index, std::false_type());
			}
		}
		else if (layer_index == 0 || layer_index == layer->index)
		{
			if (op == Opcode::Matches)
				subset = find_matches(settings, annot, layer->index, std::true_type());
			else
				subset = find_matches(settings, annot, layer->index, std::false_type());
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

QueryMatchSet SearchConstraint::find_matches(Settings *settings, const AutoAnnotation &annot, int layer_index,
                                             std::true_type)
{
	auto &events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;
	static String sep(" ");
	auto type = settings->type;
	PHON_LOG("search for text matches (regex) in " << annot->path());

	for (intptr_t i = 1; i <= events.size(); i++)
	{
		auto &event = events[i];
		int match_index = 0;
		auto &label = event->text();
		auto it = label.begin();

		while (value_pattern.match(label, it))
		{
			auto matched_text = value_pattern.capture(0);
			auto start = value_pattern.capture_start_iter(0);
			auto end = value_pattern.capture_end_iter(0);
			it = end;
			AutoQueryMatch match;

			switch (type)
			{
				case Query::Type::CodingProtocol:
				{
					auto left = Annotation::left_context(events, i, start, context_length, sep);
					auto right = Annotation::right_context(events, i, end, context_length, sep);
					// For regular expressions only, we split the matched text into fields if there is a query protocol.
					auto count = value_pattern.count();
					assert(count == m_protocol->field_count());
					Array<String> fields(count);
					for (intptr_t c = 1; c <= count; c++) {
						fields.append(value_pattern.capture(c));
					}

					match = std::make_shared<CodingConcordance>(annot, layer_index, event, matched_text, ++match_index, left,
					                                            right, std::move(fields));
					break;
				}
				case Query::Type::Formants:
				{
					double max_frequency;
					int lpc_order;
					auto formants = get_formants(settings, annot.get(), event.get(), max_frequency, lpc_order);
					match = std::make_shared<FormantMeasurement>(annot, layer_index, event, matched_text, ++match_index,
							max_frequency, lpc_order, std::move(formants));
					break;
				}
				default:
				{
					auto left = Annotation::left_context(events, i, start, context_length, sep);
					auto right = Annotation::right_context(events, i, end, context_length, sep);
					match = std::make_shared<Concordance>(annot, layer_index, event, matched_text, ++match_index, left, right);
				}
			}

			matches.insert(std::move(match));
		}
	}

	return matches;
}

QueryMatchSet SearchConstraint::find_matches(Settings *settings, const AutoAnnotation &annot, int layer_index, std::false_type)
{
	auto &events = annot->get_layer_events(layer_index);
	QueryMatchSet matches;
	static String sep(" ");
	auto type = settings->type;
	PHON_LOG("search for text matches (plain text) in " << annot->path());

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
				switch (type)
				{
					case Query::Type::Formants:
					{
						double max_frequency;
						int lpc_order;
						auto formants = get_formants(settings, annot.get(), event.get(), max_frequency, lpc_order);
						auto match = std::make_shared<FormantMeasurement>(annot, layer_index, event, label, ++match_index,
						                                             max_frequency, lpc_order, std::move(formants));
						matches.insert(std::move(match));
						break;
					}
					default:
					{
						String::const_iterator end = it + value.size();
						auto left = Annotation::left_context(events, i, it, context_length, sep);
						auto right = Annotation::right_context(events, i, end, context_length, sep);
						matches.insert(std::make_shared<Concordance>(annot, layer_index, event, value, ++match_index, left, right));
						it = end;
					}
				}
			}
		}
		else
		{
			while((it = label.ifind(value, it)) != label.end())
			{
				switch (type)
				{
					case Query::Type::Formants:
					{
						double max_frequency;
						int lpc_order;
						auto formants = get_formants(settings, annot.get(), event.get(), max_frequency, lpc_order);
						auto match = std::make_shared<FormantMeasurement>(annot, layer_index, event, label, ++match_index,
						                                                  max_frequency, lpc_order, std::move(formants));
						matches.insert(std::move(match));
						break;
					}
					default:
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
		}
	}

	return matches;
}

Array<double>
SearchConstraint::get_formants(SearchNode::Settings *s, Annotation *annot, Event *event, double &max_freq, int &lpc_order)
{
	using namespace speech;

	auto sound = annot->sound();
	if (!sound) {
		throw error("Cannot measure formants in annotation \"%\" because it is not bound to any sound file", annot->path());
	}
	auto settings = dynamic_cast<FormantQuerySettings*>(s);
	assert(settings);

	if (settings->automatic)
	{
		std::tie(max_freq, lpc_order) = find_lpc_parameters(sound.get(), settings->nformant, settings->max_bandwidth,
				settings->win_size, event->start_time(), event->end_time(), settings->max_freq1, settings->max_freq2,
				settings->step, settings->lpc_order1, settings->lpc_order2);
	}
	else
	{
		max_freq = settings->max_freq;
		lpc_order = settings->lpc_order;
	}

	return measure_formants(s, sound.get(), event, max_freq, lpc_order);
}

Array<double>
SearchConstraint::measure_formants(SearchNode::Settings *s, Sound *sound, Event *event, double max_freq, int lpc_order)
{
	using namespace speech;
	auto settings = dynamic_cast<FormantQuerySettings*>(s);
	// For now, measure at the mid point.
	auto t = event->center_time();

	auto nformant = settings->nformant;
	intptr_t base = nformant;
	Array<double> formants(1, settings->field_count(), 0.0);

	// Did Weenink's method fail to find suitable parameters?
	if (max_freq == 0)
	{
		std::fill(formants.begin(), formants.end(), std::nan(""));
		return formants;
	}

	auto data = sound->get_formants(t, nformant, max_freq, settings->max_bandwidth, settings->win_size, lpc_order);

	// Put formants first, and optionally add the bandwidths (e.g. F1, F2, F3, B1, B2, B3)
	for (intptr_t i = 1; i <= nformant; i++)
	{
		formants(1, i) = data(i, 1);
		if (settings->bandwidth) {
			formants(1, base + i) = data(i, 2);
		}
	}
	if (settings->bandwidth) {
		base += nformant;
	}
	if (settings->erb)
	{
		for (intptr_t i = 1; i <= nformant; i++)
		{
			auto f = data(i, 1);
			if (std::isfinite(f)) f = hertz_to_erb(f);
			formants(1, base + i) = f;
		}

		base += nformant;
	}
	if (settings->bark)
	{
		for (intptr_t i = 1; i <= nformant; i++)
		{
			auto f = data(i, 1);
			if (std::isfinite(f)) f = hertz_to_bark(f);
			formants(1, base + i) = f;
		}
	}

	return formants;
}

} // namespace phonometrica
