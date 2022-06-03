/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Purpose: time-aligned annotation.                                                                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ANNOTATION_HPP
#define PHONOMETRICA_ANNOTATION_HPP

#include <phon/application/sound.hpp>
#include <phon/application/agraph.hpp>
#include <phon/error.hpp>

namespace phonometrica {

class Runtime;
class Object;

class Annotation final : public Document
{
public:

	enum Type {
		Undefined,
		Native,
		TextGrid,
		WaveSurfer
	};

	// Constructor used to create a new annotation from a sound file.
	Annotation() :
		Annotation(nullptr, String())
	{ m_type = Native; }

	explicit Annotation(Directory *parent, String path = String());

	void set_path(String path, bool mutate) override;

	bool has_sound() const;

	const Handle<Sound> &sound() const;

	void set_sound(const Handle<Sound> &value, bool mutate = true);

	const EventList &get_layer_events(intptr_t i) const;

	bool is_textgrid() const { return m_type == TextGrid; }

	bool is_native() const { return m_type == Native; }

	static void initialize(Runtime &rt);

	const LayerList &layers() const { return m_graph.layers(); }

	AGraph &graph() { return m_graph; }

	intptr_t size() const { return m_graph.layer_count(); }

	bool modified() const override;

	void set_event_text(AutoEvent &event, const String &new_text);

	String left_context(intptr_t layer, intptr_t event, intptr_t offset, intptr_t length, const String &separator = String()) const;

	String right_context(intptr_t layer, intptr_t event, intptr_t offset, intptr_t length, const String &separator = String()) const;

	void write_as_native(const String &path = String());

	void write_as_textgrid(const String &path = String());

	AutoEvent get_event(intptr_t layer, intptr_t event) const;

	intptr_t layer_count() const;

	void create_layer(intptr_t index, const String &name, bool has_instants);

	void remove_layer(intptr_t index);

	void clear_layer(intptr_t index);

	void discard_changes() override;

	std::span<AutoEvent> get_slice(intptr_t layer_index, double start_time, double end_time) const;

	void duplicate_layer(intptr_t index, intptr_t new_index);

	bool layer_has_instants(intptr_t index) const;

	String get_layer_label(intptr_t index) const;

	void set_layer_label(intptr_t index, String value);

	AutoEvent find_enclosing_event(const AutoEvent &e, intptr_t layer) const;

	bool content_modified() const override;

	AutoEvent find_event_starting_at(intptr_t layer_index, double time) const;

	AutoEvent find_event_ending_at(intptr_t layer_index, double time) const;

	AutoEvent find_previous_event(intptr_t layer_index, double time) const;

	AutoEvent find_next_event(intptr_t layer_index, double time) const;

	intptr_t get_event_index(intptr_t layer_index, double time) const;

	static Signal<const Handle<Annotation>&, const AutoEvent&, const String&> edit_event;

protected:

	void read_from_native();

	void preload();

	void load() override;

	void write() override;

	void save_metadata() override;

	bool uses_external_metadata() const override;

	void metadata_to_xml(xml_node meta_node) override;

	void metadata_from_xml(xml_node meta_node) override;

private:

	Annotation::Type guess_type();

	Handle<Sound> m_sound;

	AGraph m_graph;

	Type m_type = Undefined;

};


//----------------------------------------------------------------------------------------------------------------------

struct AnnotationLessComparator
{
	bool operator()(const Handle<Annotation> &lhs, const Handle<Annotation> &rhs) const
	{
		return lhs->path() < rhs->path();
	}
};


namespace traits {
template<> struct maybe_cyclic<Annotation> : std::false_type { };
}

} // namespace phonometrica

#endif // PHONOMETRICA_ANNOTATION_HPP
