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

#include <phon/application/annotation.hpp>
#include <phon/application/macros.hpp>
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Signal<const std::shared_ptr<Annotation>&, const AutoEvent&, const String&> Annotation::edit_event;

Annotation::Annotation(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{
	m_type = guess_type();
	// Native files are loaded in 2 steps: first, we load the metadata when the file is created. Next,
	// we load the graph when open() is called.
	if (is_native() && has_path()) preload();
}

const char *Annotation::class_name() const
{
	return "Annotation";
}

bool Annotation::is_annotation() const
{
	return true;
}

void Annotation::preload()
{
	assert(!m_path.empty());
	static std::string_view project_tag("Phonometrica");
	static std::string_view class_tag = class_name();
	static std::string_view meta_tag = "Metadata";

	xml_document doc;
	auto root = read_xml(doc, m_path);

	if (root.name() != project_tag) {
		throw error("Invalid XML project root in %", m_path);
	}

	auto attr = root.attribute("class");

	if (!attr || attr.as_string() != class_tag) {
		throw error("[Input/Output] Expected an annotation file, got a % file instead", attr.as_string());
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == meta_tag)
		{
			metadata_from_xml(node);
		}
	}
}

void Annotation::load()
{
	// Newly created annotations don't have a path yet.
	if (m_path.empty() && is_native()) return;

	if (m_type == Undefined) {
		m_type = guess_type();
	}

	switch (m_type)
	{
		case Type::Native:
			read_from_native();
			break;
		case Type::TextGrid:
			m_graph.read_textgrid(m_path);
			break;
		default:
			throw error("Cannot load annotation: unsupported format");
	}
}

void Annotation::write()
{
	switch (m_type)
	{
		case Type::Native:
			write_as_native();
			break;
		case Type::TextGrid:
			write_as_textgrid(m_path);
			break;
		default:
			throw error("Cannot write annotation: unsupported format");
	}
}

bool Annotation::has_sound() const
{
	return bool(m_sound);
}

std::shared_ptr<Sound> Annotation::sound() const
{
	return m_sound;
}

void Annotation::set_sound(const std::shared_ptr<Sound> &value, bool mutate)
{
	m_sound = value;
	m_metadata_modified |= mutate;
}

Annotation::Type Annotation::guess_type()
{
	if (!m_path.empty())
	{
		auto ext = filesystem::ext(m_path, true);

		if (ext == PHON_EXT_ANNOTATION) {
			return Type::Native;
		}
		if (ext == ".textgrid") {
			return Type::TextGrid;
		}
		if (ext == ".lab") {
			return Type::WaveSurfer;
		}
	}

	return Type::Undefined;
}

const EventList & Annotation::get_layer_events(intptr_t i) const
{
	return m_graph.get_layer_events(i);
}

void Annotation::save_metadata()
{
	// Native files store their metadata directly, other files need to write them to the database.
	if (m_type != Native) {
		VFile::save_metadata();
	}
}

void Annotation::set_path(String path, bool mutate)
{
	VFile::set_path(std::move(path), mutate);
	m_type = guess_type();
}

bool Annotation::uses_external_metadata() const
{
	return m_type != Native;
}

void Annotation::initialize(Runtime &rt)
{
	auto cls = rt.add_standard_type<AutoAnnotation>("Annotation");

    auto annot_get_field = [](Runtime &, std::span<Variant> args) -> Variant  {
        auto &annot = cast<AutoAnnotation>(args[0]);
        auto &key = cast<String>(args[1]);

	    // Don't open the annotation yet if we just want its path
	    if (key == "path")
        {
        	return annot->path();
        }
	    annot->open();

        if (key == "sound")
        {
			if (annot->has_sound()) {
				return make_handle<AutoSound>(annot->sound());
			}

			return Variant();    	
        }
        else if (key == "nlayer") {
			return annot->layer_count();
        }
        throw error("[Index error] Annotation type has no member named \"%\"", key);
    };
    
    auto add_property = [](Runtime &, std::span<Variant> args) -> Variant  {
    	auto &annot = cast<AutoAnnotation>(args[0]);
    	auto &category = cast<String>(args[1]);
		annot->open();
    	std::any value;

    	if (check_type<String>(args[2])) {
			value = cast<String>(args[2]);
		}
    	else if (check_type<bool>(args[2])) {
    		value = cast<bool>(args[2]);
    	}
    	else if (args[2].resolve().is_number()) {
    		value = args[2].resolve().get_number();
    	}
    	else {
    		throw error("Invalid property type: %", args[2].class_name());
    	}
		annot->add_property(Property(category, std::move(value)));

		return Variant();
    };

    auto remove_property = [](Runtime &, std::span<Variant> args) -> Variant  {
    	auto &annot = cast<AutoAnnotation>(args[0]);
    	auto &category = cast<String>(args[1]);
		annot->open();
		annot->remove_property(category);
    	return Variant();
    };

    auto bind_to_sound = [](Runtime &, std::span<Variant> args) -> Variant  {
    	auto &annot = cast<AutoAnnotation>(args[0]);
    	auto &path = cast<String>(args[1]);
    	auto project = Project::get();
    	project->import_file(path);
    	auto snd = downcast<Sound>(project->get(path));
    	if (snd) annot->set_sound(snd);
    	return Variant();
    };

    auto get_property = [](Runtime &, std::span<Variant> args) -> Variant  {
    	auto &annot = cast<AutoAnnotation>(args[0]);
    	auto category = cast<String>(args[1]);
		annot->open();
    	auto prop = annot->get_property(category);

    	if (prop.valid())
	    {
    		if (prop.is_text())
    			return prop.value();
    		else if (prop.is_numeric())
    			return prop.numeric_value();
    		else if (prop.is_boolean())
    			return prop.boolean_value();
		    else
		    	throw error("[Internal error] Invalid property type");
	    }

    	return Variant();
    };

	auto get_event_count = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer_index = cast<intptr_t>(args[1]);
		annot->open();

		try
		{
			auto layer = annot->graph().get(layer_index);
			return layer->count();
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find layer %", layer_index);
		}
	};

	auto get_event_start = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		auto event = cast<intptr_t>(args[2]);
		annot->open();

		try
		{
			auto e = annot->get_event(layer, event);
			return e->start_time();
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find event % on layer %", event, layer);
		}
	};

	auto get_event_end = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		auto event = cast<intptr_t>(args[2]);
		annot->open();

		try
		{
			auto e = annot->get_event(layer, event);
			return e->end_time();
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find event % on layer %", event, layer);
		}
	};

    auto get_event_text = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		auto event = cast<intptr_t>(args[2]);
		annot->open();

	    try
	    {
		    auto e = annot->get_event(layer, event);
		   	return e->text();
	    }
	    catch (...)
	    {
		    throw error("[Index error] Couldn't find event % on layer %", event, layer);
	    }
    };

	auto set_event_text = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		auto event = cast<intptr_t>(args[2]);
		auto &text = cast<intptr_t>(args[3]);
		annot->open();

		try
		{
			auto e = annot->get_event(layer, event);
			e->set_text(text);
			return Variant();
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find event % on layer %", event, layer);
		}
	};

	auto get_layer_label = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		annot->open();
		try {
			return annot->get_layer_label(layer);
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find layer %", layer);
		}
		
	};

	auto set_layer_label = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &annot = cast<AutoAnnotation>(args[0]);
		auto layer = cast<intptr_t>(args[1]);
		auto &value = cast<String>(args[2]);
		annot->open();
		try {
			annot->set_layer_label(layer, value);
			return Variant();
		}
		catch (...)
		{
			throw error("[Index error] Couldn't find layer %", layer);
		}
	};
	
#define CLS(T) get_class<T>()
	cls->add_method(rt.get_field_string, annot_get_field, { CLS(AutoAnnotation), CLS(String) });
	rt.add_global("add_property", add_property, { CLS(AutoAnnotation), CLS(String), CLS(Object) });
	rt.add_global("remove_property", remove_property, { CLS(AutoAnnotation), CLS(String) });
	rt.add_global("get_property", get_property, { CLS(AutoAnnotation), CLS(String) });
	rt.add_global("bind_to_sound", bind_to_sound, { CLS(AutoAnnotation), CLS(String) });
	rt.add_global("get_event_start", get_event_start, { CLS(AutoAnnotation), CLS(intptr_t), CLS(intptr_t) });
	rt.add_global("get_event_end", get_event_end,  { CLS(AutoAnnotation), CLS(intptr_t), CLS(intptr_t) });
	rt.add_global("get_event_text", get_event_text,  { CLS(AutoAnnotation), CLS(intptr_t), CLS(intptr_t) });
	rt.add_global("set_event_text", set_event_text,  { CLS(AutoAnnotation), CLS(intptr_t), CLS(intptr_t), CLS(String) });
	rt.add_global("get_event_count", get_event_count,  { CLS(AutoAnnotation), CLS(intptr_t) });
	rt.add_global("get_layer_label", get_layer_label,  { CLS(AutoAnnotation), CLS(intptr_t) });
	rt.add_global("set_layer_label", set_layer_label,  { CLS(AutoAnnotation), CLS(intptr_t), CLS(String) });
#undef CLS
}

bool Annotation::modified() const
{
    return VFile::modified() || m_graph.modified();
}

bool Annotation::content_modified() const
{
	return m_graph.modified() || VFile::content_modified();
}

String Annotation::left_context(intptr_t layer, intptr_t event, intptr_t offset, intptr_t length, const String &separator) const
{
	try
	{
		String context(length);
		auto &events = get_layer_events(layer);
		auto it = events[event]->text().begin() + offset;
		context.append(events[event]->text().rmid(it, length));

		while (context.grapheme_count() != length && --event > 0)
		{
			auto &label = events[event]->text();
			auto prefix = label.right(length - context.grapheme_count() - separator.size());
			context.prepend(separator);
			context.prepend(prefix);
		}

		return context;
	}
	catch (std::exception &e)
	{
		throw error("Could not extract left context in annotation % in evnt % on layer %: %",
					path(), event, layer, e.what());
	}
}

String Annotation::right_context(intptr_t layer, intptr_t event, intptr_t offset, intptr_t length, const String &separator) const
{
	try
	{
		String context(length);
		auto &events = get_layer_events(layer);
		auto it = events[event]->text().begin() + offset;
		context.append(events[event]->text().mid(it, length));

		while (context.grapheme_count() != length && ++event <= events.size())
		{
			auto &label = events[event]->text();
			auto suffix = label.left(length - context.grapheme_count() - separator.size());
			context.append(separator);
			context.append(suffix);
		}

		return context;
	}
	catch (std::exception &e)
	{
		throw error("Could not extract right context in annotation % in event % on layer %: %",
					path(), event, layer, e.what());
	}
}

void Annotation::set_event_text(AutoEvent &event, const String &new_text)
{
	m_graph.set_event_text(event, new_text);
}

void Annotation::metadata_to_xml(xml_node meta_node)
{
	VFile::metadata_to_xml(meta_node);
	String snd = has_sound() ? sound()->path() : String();
	auto project = Project::get();
	Project::compress(snd, project->directory());
	add_data_node(meta_node, "Sound", snd);
}

void Annotation::write_as_native(const String &path)
{
	open();
	xml_document doc;

	auto root = doc.append_child("Phonometrica");
	auto attr = root.append_attribute("class");
    attr.set_value(class_name());
    auto meta_node = root.append_child("Metadata");
    metadata_to_xml(meta_node);
    auto graph_node = root.append_child("Graph");
    m_graph.to_xml(graph_node);

    auto &p = path.empty() ? m_path : path;
    write_xml(doc, p);
}

void Annotation::write_as_textgrid(const String &path)
{
	open();
	m_graph.write_textgrid(path);
}

void Annotation::read_from_native()
{
	assert(!m_path.empty());
	static std::string_view project_tag("Phonometrica");
	static std::string_view class_tag = class_name();
	static std::string_view graph_tag = "Graph";

	xml_document doc;
	auto root = read_xml(doc, m_path);

	if (root.name() != project_tag) {
		throw error("Invalid XML project root in %", m_path);
	}

    auto attr = root.attribute("class");

	if (!attr || attr.as_string() != class_tag) {
	    throw error("[Input/Output] Expected an annotation file, got a % file instead", attr.as_string());
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == graph_tag)
		{
			m_graph.from_xml(node);
		}
	}
}

void Annotation::metadata_from_xml(xml_node meta_node)
{
	static std::string_view sound_tag = "Sound";
	VFile::metadata_from_xml(meta_node);

	for (auto node = meta_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == sound_tag)
		{
			auto project = Project::get();
			auto path = project->import_file(node.text().get());
			auto sound = std::dynamic_pointer_cast<Sound>(project->get(path));
			set_sound(sound, false);

			return;
		}
	}
}

AutoEvent Annotation::get_event(intptr_t layer, intptr_t event) const
{
	return m_graph.get_event(layer, event);
}

intptr_t Annotation::layer_count() const
{
	return m_graph.layer_count();
}

void Annotation::create_layer(intptr_t index, const String &name, bool has_instants)
{
	if (index > this->layer_count()) {
		index = -1;
	}
	auto layer = m_graph.add_layer(index, name, has_instants);

	// Create one empty interval that spans the whole file.
	if (!has_instants)
	{
		m_graph.add_interval(layer->index, 0, m_sound->duration(), String());
	}
	m_graph.set_modified(true);
}

void Annotation::remove_layer(intptr_t index)
{
	m_graph.remove_layer(index);
	m_graph.set_modified(true);
}

void Annotation::clear_layer(intptr_t index)
{
	m_graph.clear_layer(index);
}

void Annotation::discard_changes()
{
	VNode::discard_changes();
	m_graph.set_modified(false);
}

void Annotation::duplicate_layer(intptr_t index, intptr_t new_index)
{
	m_graph.duplicate_layer(index, new_index);
}

String Annotation::get_layer_label(intptr_t index)
{
	return m_graph.get_layer_label(index);
}

void Annotation::set_layer_label(intptr_t index, String value)
{
	m_graph.set_layer_label(index, std::move(value));
}

AutoEvent Annotation::find_enclosing_event(const AutoEvent &e, intptr_t layer) const
{
	return m_graph.find_enclosing_event(e, layer);
}

std::span<AutoEvent> Annotation::get_slice(intptr_t layer_index, double start_time, double end_time) const
{
	return m_graph.get_slice(layer_index, start_time, end_time);
}

AutoEvent Annotation::find_event_starting_at(intptr_t layer_index, double time) const
{
	return m_graph.find_event_starting_at(layer_index, time);
}

AutoEvent Annotation::find_event_ending_at(intptr_t layer_index, double time) const
{
	return m_graph.find_event_ending_at(layer_index, time);
}

AutoEvent Annotation::find_previous_event(intptr_t layer_index, double time) const
{
	return m_graph.find_previous_event(layer_index, time);
}

AutoEvent Annotation::find_next_event(intptr_t layer_index, double time) const
{
	return m_graph.find_next_event(layer_index, time);
}

intptr_t Annotation::get_event_index(intptr_t layer_index, double time) const
{
	return m_graph.get_event_index(layer_index, time);
}

} // namespace phonometrica