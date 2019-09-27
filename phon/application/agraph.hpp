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
 * Purpose: Implementation of the Annotation Graph format, see Bird & Liberman (2001). A formal framework for          *
 * linguistic annotation.                                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_AGRAPH_HPP
#define PHONOMETRICA_AGRAPH_HPP

#include <memory>
#include <algorithm>
#include <phon/string.hpp>
#include <phon/utils/xml.hpp>

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
    double time;

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

    bool exists(intptr_t layer) const;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// An event is an edge in the annotation graph. It represents a speech event anchored in the audio stream
class Event final : public std::enable_shared_from_this<Event>
{
public:

    Event(Anchor *start, Anchor *end, Layer *layer, String text = String()) :
            m_start(start), m_end(end), m_layer(layer), m_text(std::move(text))
    { /* The annotation graph will anchor events after they are created */ }

    ~Event();

	bool valid() const;

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

    double center_time(double window_start, double window_end) const;

	void detach();

    void detach_left();

    void detach_right();

    void attach_left(Anchor *a);

    void attach_right(Anchor *a);

    void attach(Anchor *left, Anchor *right);

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
	using event_iterator = Array<AutoEvent>::iterator;

    Layer(intptr_t index, String label, bool has_instants = false) :
            index(index), label(std::move(label)), has_instants(has_instants)
    {}

    ~Layer() = default;

    intptr_t index;
    String label;
    bool has_instants;

    intptr_t count() const { return events.size(); }

    void insert_event(intptr_t i, AutoEvent e) { events.insert(i, std::move(e)); }

    void append_event(AutoEvent e) { events.append(std::move(e)); }

    event_iterator find_event(double time);

    bool validate(event_iterator it) const;

    std::shared_ptr<Layer> duplicate(intptr_t new_index);

private:

	friend class AGraph;

    // Sorted list of events.
    Array<AutoEvent> events;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using AutoLayer = std::shared_ptr<Layer>;
using LayerList = Array<AutoLayer>;
using AnchorList = Array<std::unique_ptr<Anchor>>;

class AGraph
{
public:

    AGraph() = default;

    ~AGraph() = default;

    AGraph(const AGraph &) = delete;

    AutoLayer add_layer(intptr_t index = -1, const String &label = String(), bool has_instants = false);

    void add_interval(intptr_t index, double start, double end, const String &text);

    void add_instant(intptr_t index, double time, const String &text);

    const EventList & get_layer_events(intptr_t index) const;

    EventList get_layer_events(intptr_t index, double t1, double t2) const;

    void read_textgrid(const String &path);

    void write_textgrid(const String &path);

    AutoLayer &get(intptr_t i) { return m_layers.at(i); }

    const AutoLayer &get(intptr_t i) const { return m_layers.at(i); }

    bool empty() const
    { return m_anchors.empty(); }

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

    void to_xml(xml_node graph_node);

    void from_xml(xml_node graph_node);

    AutoEvent get_event(intptr_t layer, intptr_t event) const;

    void remove_layer(intptr_t index);

    void clear_layer(intptr_t index);

    void add_anchor(intptr_t layer_index, double time);

    void remove_anchor(intptr_t layer_index, double time);

	void duplicate_layer(intptr_t index, intptr_t new_index);

	bool anchor_exists(intptr_t layer_index, double time);

private:

	void append_event(intptr_t layer_index, Anchor *start, Anchor *end, const String &text);

	// Get anchor at the given time. The anchor is created if it does not exist.
	Anchor *get_anchor(double time);

	AnchorList::iterator get_anchor_iter(double time);

	// Check that there is no events in a given anchor's outgoing or incoming nodes on a given layer.
	void check_free_anchor(const Array<Event *> &events, intptr_t index);

	// Change end time of event, if possible.
	bool change_time(std::shared_ptr<Event> &event, std::shared_ptr<Event> &right_boundary, double new_time);

	// Erase content.
	void clear();

	void parse_anchors(xml_node anchors_node);

	void parse_layers(xml_node layers_node);

	void parse_events(xml_node events_node);

	// Sorted list of anchors.
    AnchorList m_anchors;

    // Sorted list of Layers.
    LayerList m_layers;

    // Has modifications?
    bool m_modified = false;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Find event in a sorted list of events.
struct EventLess
{
    bool operator()(const AutoEvent &lhs, const AutoEvent &rhs) const
    {
        return lhs->end_time() < rhs->start_time();
    }

    bool operator()(const AutoEvent &e, double time) const
    {
        return e->end_time() < time;
    }

    bool operator()(double time, const AutoEvent &e) const
    {
    	return time < e->start_time();
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
