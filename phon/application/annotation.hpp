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
