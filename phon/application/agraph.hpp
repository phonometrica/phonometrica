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
 * Purpose: Implementation of the Annotation Graph format, see Bird & Liberman (2001). A formal framework for         *
 * linguistic annotation.                                                                                             *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_AGRAPH_HPP
#define PHONOMETRICA_AGRAPH_HPP

#include <memory>
#include <algorithm>
#include <phon/string.hpp>

namespace phonometrica {

class Event;

struct Layer;

// An anchor is a vertex in the annotation graph. It represents a time stamp in the
// audio stream. An anchor's time never changes. If events are moved or resized, they are
// reassociated with a new anchor at the appropriate time.
class Anchor
{
public:

    Anchor(double time) :
            time(time)
    {}

    // Unique time stamp.
    const double time;

    // Events starting from this anchor (pointers are not owned).
    Array<Event *> outgoing;

    // Events ending at this anchor (pointers are not owned).
    Array<Event *> incoming;

    bool operator==(const Anchor &other) const
    { return time == other.time; }

    bool operator<(const Anchor &other) const
    { return time < other.time; }

    bool empty()
    { return incoming.empty() && outgoing.empty(); }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// An event is an edge in the annotation graph. It represents a speech event anchored in the audio stream
class Event : public std::enable_shared_from_this<Event>
{
public:

    Event(Anchor *start, Anchor *end, Layer *layer_, String text = String()) :
            m_start(start), m_end(end), m_layer(layer_), m_text(std::move(text))
    { /* The annotation graph will anchor events after they are created */ }

    Anchor *start_anchor() const;

    void set_start_anchor(Anchor *start);

    Anchor *end_anchor() const;

    void set_end_anchor(Anchor *end);

    intptr_t layer_index() const;

    const String &text() const;

    void set_text(const String &txt);

    bool is_instant() const
    { return m_start == m_end; }

    bool is_interval() const
    { return m_start != m_end; }

    double start_time() const
    { return m_start->time; }

    double end_time() const
    { return m_end->time; }

    double center_time() const {
        return start_time() + (end_time() - start_time()) / 2;
    }


private:

    friend class AGraph;

    // Left anchor (non-owning pointer).
    Anchor *m_start;

    // Right anchor (non-owning pointer, with end => start).
    Anchor *m_end;

    // Layer this event belong to.
    Layer *m_layer;

    // For now, events have only 1 feature: a text field.
    String m_text;
};

using AutoEvent = std::shared_ptr<Event>;
using EventList = Array<AutoEvent>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Layer // ref-counted
{
    Layer(intptr_t index, String label, bool has_instants = false) :
            index(index), label(std::move(label)), has_instants(has_instants)
    {}

    ~Layer() = default;

    intptr_t index;
    String label;
    bool has_instants;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using AnchorList = Array<std::unique_ptr<Anchor>>;
using LayerList = Array<std::shared_ptr<Layer>>;
using EventList = Array<std::shared_ptr<Event>>;

class AGraph
{
public:

    AGraph() = default;

    ~AGraph() = default;

    AGraph(const AGraph &) = delete;

    void add_layer(intptr_t index = -1, const String &label = String(), bool has_instants = false);

    void add_interval(intptr_t index, double start, double end, const String &text);

    void add_instant(intptr_t index, double time, const String &text);

    EventList get_layer_events(intptr_t index) const;

    EventList get_layer_events(intptr_t index, double t1, double t2) const;

    void read_textgrid(const String &path);

    void write_textgrid(const String &path);

    Layer *get(intptr_t i) { return m_layers[i].get(); }

    const Layer *get(intptr_t i) const { return m_layers[i].get(); }

    bool empty() const
    { return anchors.empty(); }

    intptr_t layer_count() const
    { return m_layers.size(); }

    const LayerList &layers() const { return m_layers; }

    bool modified() const { return m_modified; }

    void set_modified(bool value) { m_modified = value; }

    std::shared_ptr<Event> previous_event(intptr_t layer, const std::shared_ptr<Event> &e) const;

    std::shared_ptr<Event> next_event(intptr_t layer, const std::shared_ptr<Event> &e) const;

    bool change_start_time(std::shared_ptr<Event> &event, double new_time);

    bool change_end_time(std::shared_ptr<Event> &event, double new_time);

    void set_event_text(std::shared_ptr<Event> &event, const String &new_text);

private:

    // Sorted list of anchors.
    AnchorList anchors;

    // Unsorted list of events.
    EventList events;

    // Sorted list of Layers.
    LayerList m_layers;

    // Has modifications?
    bool m_modified = false;

    void insert_event(intptr_t index, Anchor *start, Anchor *end, const String &text);

    // Get anchor at the given time. The anchor is created if it does not exist.
    Anchor *get_anchor(double time);

    // Check that there is no events in a given anchor's outgoing or incoming nodes on a given layer.
    void check_free_anchor(const Array<Event *> &events, intptr_t index);

    // Change end time of event, if possible
    bool change_time(std::shared_ptr<Event> &event, std::shared_ptr<Event> &right_boundary, double new_time);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Find event in a sorted list of events.
struct EventLess
{
    bool operator()(const std::shared_ptr<Event> &lhs, const std::shared_ptr<Event> &rhs) const
    {
        return lhs->end_time() < rhs->start_time();
    }

    bool operator()(const std::shared_ptr<Event> &e, double time) const
    {
        return e->end_time() < time;
    }
};

struct IntervalLessEqual
{
    bool operator()(const std::shared_ptr<Event> &lhs, const std::shared_ptr<Event> &rhs) const
    {
        return lhs->end_time() <= rhs->start_time();
    }
};

} // namespace phonometrica

#endif // PHONOMETRICA_AGRAPH_HPP
