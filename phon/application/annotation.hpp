/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
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

class Annotation final : public VFile
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

	explicit Annotation(VFolder *parent, String path = String());

	const char *class_name() const override;

	bool is_annotation() const override;

	void set_path(String path, bool mutate) override;

	bool has_sound() const;

	std::shared_ptr<Sound> sound() const;

	void set_sound(const std::shared_ptr<Sound> &value, bool mutate = true);

	const EventList &get_layer_events(intptr_t i) const;

	bool is_textgrid() const { return m_type == TextGrid; }

	bool is_native() const { return m_type == Native; }

	static void initialize(Runtime &rt);

	static Object *meta() { return metaobject; }

	const LayerList &layers() const { return m_graph.layers(); }

	AGraph &graph() { return m_graph; }

	intptr_t size() const { return m_graph.layer_count(); }

	bool modified() const override;

	void set_event_text(AutoEvent &event, const String &new_text);

	static String left_context(const EventList &events, intptr_t i, String::const_iterator start, intptr_t length,
	                    const String &separator = String());

	static String right_context(const EventList &events, intptr_t i, String::const_iterator end, intptr_t length,
	                    const String &separator = String());

	void write_as_native(const String &path = String());

	void write_as_textgrid(const String &path = String());

	AutoEvent get_event(intptr_t layer, intptr_t event) const;

	intptr_t layer_count() const;

	void create_layer(intptr_t index, const String &name, bool has_instants);

	void remove_layer(intptr_t index);

	void clear_layer(intptr_t index);

	void discard_changes() override;

	void duplicate_layer(intptr_t index, intptr_t new_index);

	String get_layer_label(intptr_t index);

	void set_layer_label(intptr_t index, String value);

protected:

	bool content_modified() const override;

	void read_from_native();

	void load() override;

	void write() override;

	void save_metadata() override;

	bool uses_external_metadata() const override;

	void metadata_to_xml(xml_node meta_node) override;

	void metadata_from_xml(xml_node meta_node) override;

private:

	Annotation::Type guess_type();

	std::shared_ptr<Sound> m_sound;

	AGraph m_graph;

	Type m_type = Undefined;

	static Object *metaobject;

};


//----------------------------------------------------------------------------------------------------------------------

using AutoAnnotation = std::shared_ptr<Annotation>;

struct AnnotationLessComparator
{
	bool operator()(const AutoAnnotation &lhs, const AutoAnnotation &rhs) const
	{
		return lhs->path() < rhs->path();
	}
};

using AnnotationSet = std::set<AutoAnnotation, AnnotationLessComparator>;

} // namespace phonometrica

#endif // PHONOMETRICA_ANNOTATION_HPP
