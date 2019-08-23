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
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <iostream>
#include <phon/error.hpp>
#include <phon/application/agraph.hpp>
#include <phon/application/praat.hpp>
#include "agraph.hpp"


namespace phonometrica { 

struct AnchorLess
{
	bool operator()(const std::unique_ptr<Anchor> &lhs, const std::unique_ptr<Anchor> &rhs) const
	{
		return lhs->time < rhs->time;
	}

	bool operator()(const std::unique_ptr<Anchor> &lhs, double time) const
	{
		return lhs->time < time;
	}

	bool operator()(double time, const std::unique_ptr<Anchor> &lhs) const
	{
		return time < lhs->time;
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



Anchor *Event::start_anchor() const
{
	return m_start;
}

void Event::set_start_anchor(Anchor *start)
{
	m_start = start;
}

Anchor *Event::end_anchor() const
{
	return m_end;
}

void Event::set_end_anchor(Anchor *end)
{
	m_end = end;
}

intptr_t Event::layer_index() const
{
	return m_layer->index;
}

const String & Event::text() const
{
	return m_text;
}

void Event::set_text(const String &txt)
{
	m_text = txt;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AGraph::add_layer(intptr_t index, const String &label, bool has_instants)
{
	assert(index != 0);

	if (index < 0)
	{
		index = m_layers.size() + 1;
		m_layers.append(std::make_shared<Layer>(index, label, has_instants));
	}
	else
	{
		m_layers.insert(m_layers.begin() + index, std::make_shared<Layer>(index, label, has_instants));
	}
}

void AGraph::add_interval(intptr_t index, double start, double end, const String &text)
{
	auto start_anchor = get_anchor(start);
	auto end_anchor = get_anchor(end);
	assert(start_anchor);
	assert(end_anchor);

	check_free_anchor(start_anchor->outgoing, index);
	check_free_anchor(end_anchor->incoming, index);
	insert_event(index, start_anchor, end_anchor, text);
}

void AGraph::add_instant(intptr_t index, double time, const String &text)
{
	auto anch = get_anchor(time);
	assert(anch);

	check_free_anchor(anch->incoming, index);
	check_free_anchor(anch->outgoing, index);
	insert_event(index, anch, anch, text);
}

void AGraph::insert_event(intptr_t index, Anchor *start, Anchor *end, const String &text)
{
	assert(index != 0);

	if (index < 0) {
		index = m_layers.size();
	}

	auto layer = m_layers[index].get();
	auto e = std::make_shared<Event>(start, end, layer, text);

	start->outgoing.push_back(e.get());
	end->incoming.push_back(e.get());
	events.push_back(std::move(e));
}

Anchor *AGraph::get_anchor(double time)
{
	auto it = std::lower_bound(anchors.begin(), anchors.end(), time, AnchorLess());

	if (it == anchors.end() || time < it->get()->time)
	{
		auto a = anchors.insert(it, std::make_unique<Anchor>(time));
		return a->get();
	}
	else
	{
		return it->get();
	}
}

void AGraph::check_free_anchor(const Array<Event *> &events, intptr_t index)
{
	for (auto e : events)
	{
		if (e->layer_index() == index) {
			throw error("Anchor already exists on layer %", index + 1);
		}
	}
}

EventList AGraph::get_layer_events(intptr_t index) const
{
	EventList events;
	intptr_t anchor_index = 1;

	if (anchors.empty()) {
		return events;
	}

	intptr_t anchor_count = anchors.size();
	auto anchor = anchors.front().get();

	while (anchor && anchor_index <= anchor_count)
	{
		bool found = false;

		for (auto e : anchor->outgoing)
		{
			// Does this anchor belong to our layer?
			if (e->layer_index() == index)
			{
				events.push_back(e->shared_from_this());

				if (e->is_instant()) {
					// Try next anchor. This may be the last anchor since an instant's start and end point to the same
					// anchor.
					if (++anchor_index < anchor_count) {
						anchor = anchors[anchor_index].get();
					}
					else {
						anchor = nullptr; // done
					}
				}
				else
				{
					// Use right anchor as a start point
					anchor = e->end_anchor();
					auto it = std::lower_bound(anchors.begin(), anchors.end(), anchor->time, AnchorLess());
					assert(it != anchors.end());
					anchor_index = intptr_t(it - anchors.begin());
				}

				found = true;
				break; // There can only be one event on this node on a given layer
			}
		}

		// Scan the next anchor
		if (! found)
		{
			if (++anchor_index < anchor_count) {
				anchor = anchors[anchor_index].get();
			}
			else {
				anchor = nullptr; // done
			}
		}
	}

	return events;
}

EventList AGraph::get_layer_events(intptr_t index, double t1, double t2) const
{
	EventList events;
	intptr_t anchor_index = 1;

	if (anchors.empty()) {
		return events;
	}

	intptr_t anchor_count = anchors.size();
	auto anchor = anchors[1].get();

	while (anchor && anchor_index <= anchor_count)
	{
		bool found = false;

		for (auto e : anchor->outgoing)
		{
			// Is this anchor in our time window?
			if (e->end_time() < t1)
				continue;
			if (e->start_time() > t2)
				break; // done

			// Does this anchor belong to our layer?
			if (e->layer_index() == index)
			{
				events.push_back(e->shared_from_this());

				if (e->is_instant()) {
					// Try next anchor. This may be the last anchor since an instant's start and end point to the same
					// anchor.
					if (++anchor_index < anchor_count) {
						anchor = anchors[anchor_index].get();
					}
					else {
						anchor = nullptr; // done
					}
				}
				else
				{
					// Use right anchor as a start point
					anchor = e->end_anchor();
					auto it = std::lower_bound(anchors.begin(), anchors.end(), anchor->time, AnchorLess());
					assert(it != anchors.end());
					anchor_index = intptr_t(it - anchors.begin());
				}

				found = true;
				break; // There can only be one event on this node on a given layer
			}
		}

		// Scan the next anchor
		if (! found)
		{
			if (++anchor_index <= anchor_count) {
				anchor = anchors[anchor_index].get();
			}
			else {
				anchor = nullptr; // done
			}
		}
	}

	return events;
}


void AGraph::read_textgrid(const String &path)
{
	File infile(path);
	//std::cerr << "\nParsing textgrid " << path.str() << "\n\n";

	do
	{
		auto line = infile.read_line();
		praat::TierHeader header;

		if (praat::parse_tier_header(infile, line, header))
		{
			add_layer(-1, header.label, header.has_points);
//			std::cerr << "Parsed tier " << header.label << std::endl;
		}
		else if (!m_layers.empty())
		{
			praat::Interval interval;

			if (praat::parse_interval(infile, line, interval))
			{
				add_interval(-1, interval.xmin, interval.xmax, interval.text);
//				std::cerr << "Parsed interval \"" << interval.xmin << " to " << interval.xmax << "\"\n";
//				std::cerr << "Label: " << interval.text << std::endl;
			}
			else
			{
				praat::Point point;

				if (praat::parse_point(infile, line, point))
				{
					add_instant(-1, point.time, point.text);
					//std::cerr << "Parsed point \"" << text.str() << "\"\n";
				}
			}
		}
	}
	while (!infile.at_end());
}

void AGraph::write_textgrid(const String &path)
{
	File outfile(path, File::Write, Encoding::Utf8);

	// Write TextGrid header
	double start_time = anchors.front()->time;
	double end_time = anchors.back()->time;
	intptr_t layer_count = m_layers.size();

	outfile.write("File type = \"ooTextFile\"\nObject class = \"TextGrid\"\n\n");
	outfile.format("xmin = %.16f\nxmax = %.16f\n", start_time, end_time);
	outfile.format("tiers? <exists>\nsize = %lu\nitem []:\n", layer_count);

	for (intptr_t i = 1; i <= layer_count; ++i)
	{
		auto the_layer = m_layers[i].get();
		auto layer_events = get_layer_events(i);
		String kind = the_layer->has_instants ? "TextTier" : "IntervalTier";

		// Write tier header
		outfile.format("    item [%lu]:\n", i);
		outfile.format("        class = \"%s\"\n", kind.data());
		outfile.format("        name = \"%s\"\n", the_layer->label.data());

		if (layer_events.empty()) {
			outfile.write("        xmin = 0\n        xmax = 0\n");
		}
		else {
			start_time = layer_events.front()->start_time();
			end_time = layer_events.back()->end_time();
			outfile.format("        xmin = %.16f\n        xmax = %.16f\n", start_time, end_time);
		}

		// Write tier events
		if (the_layer->has_instants)
		{
			outfile.format("        points: size = %lu\n", layer_events.size());

			for (intptr_t j = 1; j <= layer_events.size(); j++)
			{
				auto e = layer_events[j];

				outfile.format("        points [%lu]:\n", j);
				outfile.format("            number = %.16f\n", e->start_time());
				outfile.format("            mark = \"%s\"\n", e->text().data());
			}
		}
		else
		{
			outfile.format("        intervals: size = %lu\n", layer_events.size());

			for (intptr_t j = 1; j <= layer_events.size(); ++j)
			{
				auto e = layer_events[j];

				outfile.format("        intervals [%lu]:\n", j);
				outfile.format("            xmin = %.16f\n", e->start_time());
				outfile.format("            xmax = %.16f\n", e->end_time());
				outfile.format("            text = \"%s\"\n", e->text().data());
			}
		}
	}

	m_modified = false;
}

std::shared_ptr<Event> AGraph::previous_event(intptr_t layer, const std::shared_ptr<Event> &e) const
{
    double time = e->start_time();

    while (true)
    {
        // Find the previous anchor
        auto it = std::lower_bound(anchors.begin(), anchors.end(), time, AnchorLess());
        assert(it != anchors.end());

        if (it == anchors.begin())
        {
            return std::shared_ptr<Event>();
        }
		it--;

        auto anchor = it->get();

        for (auto event : anchor->outgoing)
        {
            if (event->layer_index() == layer)
            {
                return event->shared_from_this();
            }
        }

        time = anchor->time;
    }
}

std::shared_ptr<Event> AGraph::next_event(intptr_t layer, const std::shared_ptr<Event> &e) const
{
    double time = e->end_time();
    bool has_intervals = e->is_interval();

    while (true)
    {
        // Find the next anchor
        auto it = std::upper_bound(anchors.begin(), anchors.end(), time, AnchorLess());

        if (it == anchors.end())
        {
            return std::shared_ptr<Event>();
        }

        auto anchor = it->get();

        for (auto event : anchor->incoming)
        {
            if (event->layer_index() == layer)
            {
                return event->shared_from_this();
            }
        }

        // The incoming and outgoing nodes are the same for instants
        if (has_intervals)
        {
            for (auto event : anchor->outgoing)
            {
                if (event->layer_index() == layer)
                {
                    return event->shared_from_this();
                }
            }
        }

        time = anchor->time;
    }
}

bool AGraph::change_start_time(std::shared_ptr<Event> &event, double new_time)
{
    // Change end time of the previous item: this will change the start time of this event as a side effect.
	auto previous = previous_event(event->layer_index(), event);

	if (previous)
    {
	    return change_time(previous, event, new_time);
    }

	return false;
}

bool AGraph::change_end_time(std::shared_ptr<Event> &event, double new_time)
{
	auto boundary = next_event(event->layer_index(), event);
	return change_time(event, boundary, new_time);
}

void AGraph::set_event_text(std::shared_ptr<Event> &event, const String &new_text)
{
    event->set_text(new_text);
    m_modified = true;
}

bool AGraph::change_time(std::shared_ptr<Event> &event, std::shared_ptr<Event> &right_boundary, double new_time)
{
    if (right_boundary && new_time >= right_boundary->end_time())
    {
        return false; // the new anchor must be before the next event
    }

    // this works for both instants and intervals. For intervals, the end of the previous interval
    // corresponds to the start of the current interval. For instants, we ensure that we don't move past
    // the previous instant.
    auto left_boundary = previous_event(event->layer_index(), event);

    if (left_boundary && new_time <= left_boundary->end_time())
	{
    	return false;
	}

    bool is_instant = event->is_instant();
    auto old_anchor = event->end_anchor();
    auto layer_index = event->layer_index();

    old_anchor->incoming.remove_first(event.get());
    Event *next_event = nullptr;

    for (intptr_t i = 1; i <= old_anchor->outgoing.size(); i++)
    {
        auto e = old_anchor->outgoing[i];

        if (e->layer_index() == layer_index)
        {
            next_event = e;
            old_anchor->outgoing.remove_at(i);
            break;
        }
    }

    assert(next_event);
    auto new_anchor = get_anchor(new_time);
    new_anchor->incoming.append(event.get());
    new_anchor->outgoing.append(next_event);
    event->m_end = new_anchor;
    next_event->m_start = new_anchor;
    if (is_instant) event->m_start = new_anchor;
    m_modified = true;

    return true;
}

} // namespace phonometrica
