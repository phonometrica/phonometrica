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
 * Created: 08/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

static const int BASE_COLUMN_COUNT = 4;

Concordance::Concordance(intptr_t target_count, Context ctx, intptr_t context_length, Array <AutoMatch> matches, VFolder *parent, const String &path) :
	Dataset(parent, path), m_matches(std::move(matches))
{
	m_target_count = (int) target_count;
	m_context_type = ctx;
	m_context_length = (int) context_length;
	m_context.reserve(m_matches.size());
	find_context();
}

const char *Concordance::class_name() const
{
	return "Concordance";
}

bool Concordance::empty() const
{
	return m_matches.empty();
}

String Concordance::get_header(intptr_t j) const
{
	switch (j)
	{
		case 1:
			return "File";
		case 2:
			return "Layer";
		case 3:
			return "Start time";
		case 4:
			return "End time";
		case 5:
			return "Left context";
		case 6:
			return "Match";
		case 7:
			return "Right context";
		default:
			return String();
	}
}

String Concordance::get_cell(intptr_t i, intptr_t j) const
{
	switch (j)
	{
		case 1:
			return m_matches[i]->annotation()->label();
		case 2:
			return String::convert(m_matches[i]->get_layer(1));
		case 3:
			return String::convert(m_matches[i]->get_event(1)->start_time());
		case 4:
			return String::convert(m_matches[i]->get_event(1)->end_time());
		case 5:
			return get_left_context(i);
		case 6:
			return m_matches[i]->get_value(1);
		case 7:
			return get_right_context(i);
		default:
			return String();
	}
}

void Concordance::set_cell(intptr_t i, intptr_t j, const String &value)
{

}

intptr_t Concordance::row_count() const
{
	return m_matches.size();
}

intptr_t Concordance::column_count() const
{
	int n = BASE_COLUMN_COUNT; // file, layer, start time, end time
	if (m_context_type != Context::None) {
		n += 2; // left and right context
	}

	return n + m_target_count;
}

void Concordance::load()
{

}

void Concordance::write()
{

}

bool Concordance::has_context() const
{
	return m_context_type != Context::None;
}

void Concordance::find_context()
{
	m_context.clear();

	switch (m_context_type)
	{
		case Context::Labels:
			find_label_context();
			break;
		case Context::KWIC:
			find_kwic_context();
			break;
		default:
			break;
	}
}

void Concordance::find_kwic_context()
{
	String sep(" ");

	for (auto &match : m_matches)
	{
		auto target = match->reference_target();
		std::pair<String, String> ctx;
		if (target)
		{
			auto &events = match->annotation()->get_layer_events(target->layer);
			auto i = match->annotation()->get_event_index(target->layer, target->event->start_time());
			assert(i != 0);
			auto offset = target->offset;
			ctx.first = Annotation::left_context(events, i, offset, m_context_length, sep);
			offset += target->value.size();
			ctx.second = Annotation::right_context(events, i, offset, m_context_length, sep);
		}
		m_context.append(std::move(ctx));
	}
}

void Concordance::find_label_context()
{
	for (auto &match : m_matches)
	{
		auto target = match->reference_target();
		std::pair<String, String> ctx;
		if (target)
		{
			auto &annot = *match->annotation();
			auto &events = annot.get_layer_events(target->layer);
			auto i = annot.get_event_index(target->layer, target->event->start_time());
			assert(i != 0);
			ctx.first = (i == 1) ? String() : events[i-1]->text();
			ctx.second = (i == events.size()) ? String() : events[i+1]->text();
		}
		m_context.append(std::move(ctx));
	}
}

String Concordance::get_left_context(intptr_t i) const
{
	return has_context() ? m_context[i].first : String();
}

String Concordance::get_right_context(intptr_t i) const
{
	return has_context() ? m_context[i].second : String();
}

} // namespace phonometrica
