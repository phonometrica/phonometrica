/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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

bool Anchor::exists(intptr_t layer) const
{
	for (auto e : incoming)
	{
		if (e->layer_index() == layer) {
			return true;
		}
	}
	for (auto e : outgoing)
	{
		if (e->layer_index() == layer) {
			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Event::~Event()
{
	if (valid())
	{
		m_start->outgoing.remove(this);
		if (!is_instant()) m_end->incoming.remove(this);
	}
}

bool Event::valid() const
{
	return m_start != nullptr && m_end != nullptr;
}

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

void Event::detach()
{
	detach_left();
	if (!is_instant()) detach_right();
}

void Event::detach_left()
{
	m_start->outgoing.remove(this);
	m_start = nullptr;
}

void Event::detach_right()
{
	m_end->incoming.remove(this);
	m_end = nullptr;
}

void Event::attach_left(Anchor *a)
{
	a->outgoing.append(this);
	m_start = a;
}

void Event::attach_right(Anchor *a)
{
	a->incoming.append(this);
	m_end = a;
}

void Event::attach(Anchor *left, Anchor *right)
{
	attach_left(left);
	attach_right(right);
}

double Event::center_time(double window_start, double window_end) const
{
	double start = (std::max)(window_start, start_time());
	double end = (std::min)(window_end, end_time());
	return start + (end - start) / 2;
}

bool Event::has_anchor(double time) const
{
	return start_time() == time || end_time() == time;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Layer::event_iterator Layer::find_event(double time)
{
	return std::lower_bound(events.begin(), events.end(), time, EventLess());
}


intptr_t Layer::find_index(double time)
{
	auto it = find_event(time);
	return (it == events.end()) ? 0 : intptr_t(it - events.begin());
}

std::shared_ptr<Layer> Layer::duplicate(intptr_t new_index)
{
	auto new_layer = std::make_shared<Layer>(new_index, this->label, this->has_instants);
	new_layer->events.reserve(this->count());

	for (auto &event : this->events)
	{
		auto left = event->start_anchor();
		auto right = event->end_anchor();
		auto new_event = std::make_shared<Event>(left, right, new_layer.get(), event->text());
		new_event->attach(left, right);
		new_layer->events.append(std::move(new_event));
	}

	return new_layer;
}

bool Layer::validate(Layer::event_iterator it) const
{
	return it != events.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AutoLayer AGraph::add_layer(intptr_t index, const String &label, bool has_instants)
{
	assert(index != 0);
	AutoLayer layer;

	if (index < 0)
	{
		index = m_layers.size() + 1;
		layer = std::make_shared<Layer>(index, label, has_instants);
		m_layers.append(layer);
	}
	else
	{
		layer = std::make_shared<Layer>(index, label, has_instants);
		m_layers.insert(index, layer);

		// Adjust indices.
		for (intptr_t i = 1; i <= m_layers.size(); i++)
		{
			m_layers[i]->index = i;
		}
	}

	return layer;
}

void AGraph::add_interval(intptr_t index, double start, double end, const String &text)
{
	auto start_anchor = get_anchor(start);
	auto end_anchor = get_anchor(end);
	assert(start_anchor);
	assert(end_anchor);

	check_free_anchor(start_anchor->outgoing, index);
	check_free_anchor(end_anchor->incoming, index);
	append_event(index, start_anchor, end_anchor, text);
}

void AGraph::add_instant(intptr_t index, double time, const String &text)
{
	auto anch = get_anchor(time);
	assert(anch);

	check_free_anchor(anch->incoming, index);
	check_free_anchor(anch->outgoing, index);
	append_event(index, anch, anch, text);
}

void AGraph::append_event(intptr_t layer_index, Anchor *start, Anchor *end, const String &text)
{
	assert(layer_index != 0);

	if (layer_index < 0) {
		layer_index = m_layers.size();
	}

	auto layer = m_layers[layer_index].get();
	auto e = std::make_shared<Event>(start, end, layer, text);

	start->outgoing.push_back(e.get());
	end->incoming.push_back(e.get());
	layer->append_event(std::move(e));
}

AnchorList::iterator AGraph::get_anchor_iter(double time)
{
	auto it = std::lower_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());

	if (it == m_anchors.end() || time < it->get()->time)
	{
		return m_anchors.insert(it, std::make_unique<Anchor>(time));
	}

	return it;
}

Anchor *AGraph::get_anchor(double time)
{
	return get_anchor_iter(time)->get();
}

void AGraph::check_free_anchor(const Array<Event *> &events, intptr_t index)
{
	for (auto e : events)
	{
		if (e->layer_index() == index) {
			throw error("[Internal error] event <%, %> already exists on layer %", e->start_time(), e->end_time(), index);
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
	clear(); // in case we are reloading a file

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

AutoEvent AGraph::previous_event(intptr_t layer, const AutoEvent &e) const
{
    double time = e->start_time();

    while (true)
    {
        // Find the previous anchor
        auto it = std::lower_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());
        assert(it != m_anchors.end());

        if (it == m_anchors.begin())
        {
            return AutoEvent();
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

AutoEvent AGraph::next_event(intptr_t layer, const AutoEvent &e) const
{
    double time = e->end_time();
    bool has_intervals = e->is_interval();

    while (true)
    {
        // Find the next anchor
        auto it = std::upper_bound(m_anchors.begin(), m_anchors.end(), time, AnchorLess());

        if (it == m_anchors.end())
        {
            return AutoEvent();
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

bool AGraph::change_start_time(AutoEvent &event, double new_time)
{
    // Change end time of the previous item: this will change the start time of this event as a side effect.
	auto previous = previous_event(event->layer_index(), event);

	if (previous)
    {
	    return change_time(previous, event, new_time);
    }

	return false;
}

bool AGraph::change_end_time(AutoEvent &event, double new_time)
{
	auto boundary = next_event(event->layer_index(), event);
	return change_time(event, boundary, new_time);
}

void AGraph::set_event_text(AutoEvent &event, const String &new_text)
{
    event->set_text(new_text);
    m_modified = true;
}

bool AGraph::change_time(AutoEvent &event, AutoEvent &right_boundary, double new_time)
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
	m_modified = false;
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
			append_event(layer, anchor1, anchor2, label);
		}
	}
}

AutoEvent AGraph::get_event(intptr_t layer, intptr_t event) const
{
	auto &events = m_layers.at(layer)->events;
	return events.at(event);
}

void AGraph::remove_layer(intptr_t index)
{
	auto layer = m_layers.take_at(index);

	for (auto &e : layer->events)
	{
		e->detach();
	}

	for (intptr_t i = index; i <= m_layers.size(); i++) {
		m_layers[i]->index--;
	}
}

void AGraph::clear_layer(intptr_t index)
{
	for (auto &e : m_layers[index]->events)
	{
		e->set_text(String());
	}
	set_modified(true);
}

void AGraph::add_anchor(intptr_t layer_index, double time, bool can_exist)
{
	auto layer = m_layers[layer_index].get();
	auto new_anchor = get_anchor(time);

	if (new_anchor->exists(layer_index) && !can_exist) {
		throw error("Anchor already exists at time % on layer %", time, layer_index);
	}

	if (layer->has_instants)
	{
		auto new_event = std::make_shared<Event>(new_anchor, new_anchor, layer);
		new_event->attach(new_anchor, new_anchor);
		auto it = layer->find_event(time);
		layer->events.insert(it, std::move(new_event));
	}
	else
	{
		// Split interval.
		auto it = layer->find_event(time);
		auto old_event = it->get();
		assert(old_event);
		it++;
		auto end_anchor = old_event->end_anchor();
		auto new_event = std::make_shared<Event>(new_anchor, end_anchor, layer);
		old_event->detach_right();
		old_event->attach_right(new_anchor);
		new_event->attach_left(new_anchor);
		new_event->attach_right(end_anchor);
		layer->events.insert(it, std::move(new_event));
	}

	set_modified(true);
}

bool AGraph::remove_anchor(intptr_t layer_index, double time)
{
	auto layer = m_layers[layer_index].get();

	if (!layer->has_instants)
	{
		if (time == 0 || time == layer->events.last()->end_time())
		{
			throw error("Cannot remove first or last anchor on an interval layer");
		}
	}

	auto it = get_anchor_iter(time);

	if (it == m_anchors.end()) {
		return false;
	}
	auto anchor = it->get();

	auto lambda = [=](Event *e) {
		return e->layer_index() == layer_index;
	};

	if (layer->has_instants)
	{
		auto &list = anchor->incoming;
		auto it = std::find_if(list.begin(), list.end(), lambda);
		assert(it != list.end());
		auto e = *it;
		e->detach();

		if (anchor->empty())
		{
			auto it = get_anchor_iter(anchor->time);
			m_anchors.remove(*it);
		}

		layer->events.remove(e->shared_from_this());
	}
	else
	{
		auto &list1 = anchor->incoming;
		auto &list2 = anchor->outgoing;
		auto it1 = std::find_if(list1.begin(), list1.end(), lambda);
		auto it2 = std::find_if(list2.begin(), list2.end(), lambda);
		assert(it1 != list1.end());
		assert(it2 != list2.end());
		auto e1 = *it1; auto e2 = *it2;
		// Merge e1 and e2 into e2. (Add a space between the labels if both are non empty.)
		auto text = e1->text();
		auto text2 = e2->text();
		if (!text.empty() && !text2.empty()) text.append(' ');
		text.append(text2);
		e2->set_text(text);

		auto first_anchor = e1->start_anchor();
		auto mid_anchor = e1->end_anchor();
		e1->detach();
		e2->detach_left();
		e2->attach_left(first_anchor);

		if (mid_anchor->empty())
		{
			auto it = get_anchor_iter(mid_anchor->time);
			m_anchors.remove(*it);
		}

		layer->events.remove(e1->shared_from_this());
	}

	set_modified(true);

	return true;
}

void AGraph::duplicate_layer(intptr_t index, intptr_t new_index)
{
	auto layer = m_layers[index].get();
	auto new_layer = layer->duplicate(new_index);
	m_layers.insert(new_index, std::move(new_layer));
	// Adjust indices.
	for (intptr_t i = 1; i <= m_layers.size(); i++)
	{
		m_layers[i]->index = i;
	}
	set_modified(true);
}

bool AGraph::anchor_exists(intptr_t layer_index, double time)
{
	auto it = get_anchor_iter(time);
	if (it == m_anchors.end()) {
		return false;
	}

	return (*it)->exists(layer_index);
}

String AGraph::get_layer_label(intptr_t index) const
{
	return m_layers.at(index)->label;
}

void AGraph::set_layer_label(intptr_t index, String value)
{
	m_layers.at(index)->label = std::move(value);
	set_modified(true);
}

AutoEvent AGraph::find_enclosing_event(const AutoEvent &e, intptr_t layer_index) const
{
	auto &layer = m_layers.at(layer_index);
	auto it = layer->find_event(e->center_time());

	if (it != layer->events.end())
	{
		if ((*it)->start_time() <= e->start_time() && (*it)->end_time() >= e->end_time())
		{
			return *it;
		}
	}

	return nullptr;
}

std::span<AutoEvent> AGraph::get_slice(intptr_t layer_index, double start_time, double end_time) const
{
	Array<AutoEvent> events;
	auto &layer = m_layers.at(layer_index);
	auto first_event = layer->find_event(start_time);
	auto last_event = first_event;

	while (last_event != layer->events.end() && (*last_event)->end_time() <= end_time) {
		last_event++;
	}

	return std::span<AutoEvent>(first_event, last_event);
}

AutoEvent AGraph::find_event_starting_at(intptr_t layer_index, double time) const
{
	// Find event whose left boundary is exactly at 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLessEqual());

	if (it == layer->events.end() || (*it)->start_time() != time) {
		return nullptr;
	}

	return *it;
}

AutoEvent AGraph::find_event_ending_at(intptr_t layer_index, double time) const
{
	// Find event whose right boundary is exactly at 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLessEqual());

	if (it != layer->events.end())
	{
		if ((*it)->is_instant()) {
			return *it;
		}
		if (it != layer->events.begin()) {
			it--;
			if ((*it)->end_time() == time) {
				return *it;
			}
		}
	}

	return nullptr;
}
// FIXME: should this be the same as the previous method?
AutoEvent AGraph::find_previous_event(intptr_t layer_index, double time) const
{
	// Find event whose right boundary is no greater than 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLess());

	if (it == layer->events.begin()) {
		return nullptr;
	}

	return *(--it);
}

AutoEvent AGraph::find_next_event(intptr_t layer_index, double time) const
{
	// Find event whose left boundary is no less than 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLess());

	if (it == layer->events.end() || ((*it)->is_instant() && (*it)->start_time() == time)) {
		return nullptr;
	}

	return *it;
}

intptr_t AGraph::get_event_index(intptr_t layer_index, double time) const
{
	// Find event whose left boundary is exactly at 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLessEqual());

	if (it == layer->events.end() || (*it)->start_time() != time) {
		return 0;
	}

	return intptr_t(it - layer->events.begin()) + 1;
}

intptr_t AGraph::time_to_event_index(intptr_t layer_index, double time) const
{
	// Find event that contains 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLessEqual());

	if (it == layer->events.end() || (layer->has_instants && (*it)->start_time() != time)) {
		return 0;
	}

	return intptr_t(it - layer->events.begin()) + 1;
}

AutoEvent AGraph::time_to_event(intptr_t layer_index, double time) const
{
	// Find event that contains 'time'.
	auto &layer = m_layers.at(layer_index);
	auto it = std::lower_bound(layer->events.begin(), layer->events.end(), time, EventLessEqual());

	if (it == layer->events.end() || (layer->has_instants && (*it)->start_time() != time)) {
		return nullptr;
	}

	return *it;
}

} // namespace phonometrica
