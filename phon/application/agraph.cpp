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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cmath>
#include <iostream>
#include <phon/error.hpp>
#include <phon/application/agraph.hpp>
#include <phon/application/praat.hpp>
#include <phon/hashmap.hpp>

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
	layer->append_event(std::move(e));
}

Anchor *AGraph::get_anchor(double time)
{
	auto it = std::lower_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());

	if (it == m_anchors.end() || time < it->get()->time)
	{
		auto a = m_anchors.insert(it, std::make_unique<Anchor>(time));
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

const EventList & AGraph::get_layer_events(intptr_t index) const
{
	return m_layers[index]->events;
}

EventList AGraph::get_layer_events(intptr_t index, double t1, double t2) const
{
	EventList result;
	auto &events = get_layer_events(index);
	auto from = std::lower_bound(events.begin(), events.end(), t1, EventLess());

	if (from == events.end()) {
		return result;
	}
	auto to = std::upper_bound(events.begin(), events.end(), t2, EventLess());
	result.reserve(to - from);

	for (auto it = from; it < to; it++) {
		result.append(*it);
	}

	return result;
}


void AGraph::read_textgrid(const String &path)
{
	File infile(path);

	do
	{
		auto line = infile.read_line();
		auto view = line.to_wide();
		praat::TierHeader header;

		if (praat::parse_tier_header(infile, line, header))
		{
			add_layer(-1, header.label, header.has_points);
			m_layers.last()->events.reserve(header.size);
		}
		else if (!m_layers.empty())
		{
			praat::Interval interval;

			if (praat::parse_interval(infile, line, interval))
			{
				add_interval(-1, interval.xmin, interval.xmax, interval.text);
			}
			else
			{
				praat::Point point;

				if (praat::parse_point(infile, line, point))
				{
					add_instant(-1, point.time, point.text);
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
	double start_time = m_anchors.front()->time;
	double end_time = m_anchors.back()->time;
	intptr_t layer_count = m_layers.size();

	outfile.write("File type = \"ooTextFile\"\nObject class = \"TextGrid\"\n\n");
	outfile.format("xmin = %.16f\nxmax = %.16f\n", start_time, end_time);
	outfile.format("tiers? <exists>\nsize = %lu\nitem []:\n", layer_count);

	for (intptr_t i = 1; i <= layer_count; ++i)
	{
		auto layer = m_layers[i].get();
		auto &layer_events = get_layer_events(i);
		String kind = layer->has_instants ? "TextTier" : "IntervalTier";

		// Write tier header
		outfile.format("    item [%lu]:\n", i);
		outfile.format("        class = \"%s\"\n", kind.data());
		outfile.format("        name = \"%s\"\n", layer->label.data());

		if (layer_events.empty()) {
			outfile.write("        xmin = 0\n        xmax = 0\n");
		}
		else {
			start_time = layer_events.front()->start_time();
			end_time = layer_events.back()->end_time();
			outfile.format("        xmin = %.16f\n        xmax = %.16f\n", start_time, end_time);
		}

		// Write tier events
		if (layer->has_instants)
		{
			outfile.format("        points: size = %lu\n", layer_events.size());

			for (intptr_t j = 1; j <= layer_events.size(); j++)
			{
				auto e = layer_events[j];
				auto text = e->text();
				text.replace("\"", "\"\"");

				outfile.format("        points [%lu]:\n", j);
				outfile.format("            number = %.16f\n", e->start_time());
				outfile.format("            mark = \"%s\"\n", text.data());
			}
		}
		else
		{
			outfile.format("        intervals: size = %lu\n", layer_events.size());

			for (intptr_t j = 1; j <= layer_events.size(); ++j)
			{
				auto e = layer_events[j];
				auto text = e->text();
				text.replace("\"", "\"\"");

				outfile.format("        intervals [%lu]:\n", j);
				outfile.format("            xmin = %.16f\n", e->start_time());
				outfile.format("            xmax = %.16f\n", e->end_time());
				outfile.format("            text = \"%s\"\n", text.data());
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
        auto it = std::lower_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());
        assert(it != m_anchors.end());

        if (it == m_anchors.begin())
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
        auto it = std::upper_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());

        if (it == m_anchors.end())
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

void AGraph::to_xml(xml_node graph_node)
{
	auto anchors_node = graph_node.append_child("Anchors");
	Hashmap<Anchor*, intptr_t> anchor_map;

	for (intptr_t i = 1; i <= m_anchors.size(); i++)
	{
		auto &anchor = m_anchors[i];
		auto anchor_node = anchors_node.append_child("Anchor");
		auto attr = anchor_node.append_attribute("id");
		attr.set_value(i);
		auto time = String::convert(anchor->time);
		add_data_node(anchor_node, "Time", time);
		anchor_map[anchor.get()] = i;
	}

	auto layers_node = graph_node.append_child("Layers");

	for (auto &layer : layers())
	{
		auto layer_node = layers_node.append_child("Layer");
		auto attr = layer_node.append_attribute("index");
		attr.set_value(layer->index);
		attr = layer_node.append_attribute("label");
		attr.set_value(layer->label.data());
		attr = layer_node.append_attribute("instants");
		attr.set_value(layer->has_instants);
	}

	auto events_node = graph_node.append_child("Events");

	for (auto &layer : layers())
	{
		for (auto &event : layer->events)
		{
			auto event_node = events_node.append_child("Event");
			auto attr = event_node.append_attribute("layer");
			attr.set_value(event->layer_index());
			auto start_node = event_node.append_child("Start");
			attr = start_node.append_attribute("anchor");
			attr.set_value(anchor_map[event->m_start]);
			auto end_node = event_node.append_child("End");
			attr = end_node.append_attribute("anchor");
			attr.set_value(anchor_map[event->m_end]);
			add_data_node(event_node, "Text", event->text().data());
		}
	}
}

void AGraph::from_xml(xml_node graph_node)
{
	static std::string_view anchors_tag = "Anchors";
	static std::string_view layers_tag = "Layers";
	static std::string_view events_tag = "Events";
	clear();

	for (auto node = graph_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == anchors_tag)
		{
			parse_anchors(node);
		}
		else if (node.name() == layers_tag)
		{
			parse_layers(node);
		}
		else if (node.name() == events_tag)
		{
			parse_events(node);
		}
	}
}

void AGraph::clear()
{
	m_anchors.clear();
	m_layers.clear();
}

void AGraph::parse_anchors(xml_node anchors_node)
{
	static std::string_view anchor_tag = "Anchor";
	AnchorList anchors;

	for (auto node = anchors_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == anchor_tag)
		{
			auto attr = node.attribute("id");
			auto id = String::to_int(attr.value());
			double time = String::to_float(node.child_value("Time"));
			anchors.append(std::make_unique<Anchor>(time));

			if (anchors.size() != id) {
				throw error("Inconsistent anchor ID: (expected %, got %)", anchors.size(), id);
			}
		}
	}

	m_anchors = std::move(anchors);
}

void AGraph::parse_layers(xml_node layers_node)
{
	static std::string_view layer_tag = "Layer";
	LayerList layers;

	for (auto node = layers_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == layer_tag)
		{
			auto attr = node.attribute("index");
			intptr_t index = String::to_int(attr.value());
			attr = node.attribute("label");
			String label = attr.value();
			attr = node.attribute("instants");
			bool instants = String::to_bool(attr.value(), true);
			layers.append(std::make_shared<Layer>(index, std::move(label), instants));

			if (index != layers.size()) {
				throw error("Inconsistent layer index (expected %, got %)", layers.size(), index);
			}
		}
	}

	m_layers = std::move(layers);
}

void AGraph::parse_events(xml_node events_node)
{
	static std::string_view event_tag = "Event";

	for (auto node = events_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == event_tag)
		{
			auto attr = node.attribute("layer");
			intptr_t layer = String::to_int(attr.value());
			auto start_node = node.child("Start");
			attr = start_node.attribute("anchor");
			intptr_t id1 = String::to_int(attr.value());
			auto anchor1 = m_anchors[id1].get();
			auto end_node = node.child("End");
			attr = end_node.attribute("anchor");
			intptr_t id2 = String::to_int(attr.value());
			auto anchor2 = m_anchors[id2].get();
			auto label = node.child_value("Text");
			insert_event(layer, anchor1, anchor2, label);
		}
	}
}

AutoEvent AGraph::get_event(intptr_t layer, intptr_t event) const
{
	auto &events = m_layers[layer]->events;
	return events[event];
}

} // namespace phonometrica
