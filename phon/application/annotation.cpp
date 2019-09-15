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

#include "annotation.hpp"
#include <phon/runtime/runtime.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Object *Annotation::metaobject = nullptr;

Annotation::Annotation(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{
	m_type = guess_type();
}

const char *Annotation::class_name() const
{
	return "Annotation";
}

bool Annotation::is_annotation() const
{
	return true;
}

void Annotation::load()
{
	if (m_type == Undefined) {
		m_type = guess_type();
	}

	switch (m_type)
	{
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
		auto v = ext.view();

		if (ext == ".phon-annot") {
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

EventList Annotation::get_layer_events(intptr_t i) const
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
	assert(!loaded());
	VFile::set_path(std::move(path), mutate);
	m_type = guess_type();
}

bool Annotation::uses_external_metadata() const
{
	return m_type != Native;
}

void Annotation::initialize(Runtime &rt)
{
    metaobject = new Object(rt, PHON_CUSERDATA, rt.object_meta);
    rt.permanent_objects.append(metaobject);

    auto new_annot = [](Runtime &rt) {
        rt.push_null(); // TODO: Annotation ctor
    };

    auto annot_path = [](Runtime &rt) {
        auto annot = rt.cast_user_data<std::shared_ptr<Annotation>>(0);
        rt.push(annot->path());
    };

    auto add_property = [](Runtime &rt) {
    	auto annot = rt.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto category = rt.to_string(1);
    	std::any value;
    	if (rt.is_boolean(2))
    		value = rt.to_boolean(2);
    	else if (rt.is_number(2))
    		value = rt.to_number(2);
		else
			value = rt.to_string(2);

		annot->add_property(Property(std::move(category), std::move(value)));
		rt.push_null();
    };

    auto remove_property = [](Runtime &rt) {
    	auto annot = rt.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto category = rt.to_string(1);
    	annot->remove_property(category);
    	rt.push_null();
    };

    auto bind_to_sound = [](Runtime &rt) {
    	auto annot = rt.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto path = rt.to_string(1);
    	auto project = Project::instance();
    	project->import_file(path);
    	auto snd = downcast<Sound>(project->get(path));
    	if (snd) annot->set_sound(snd);
    	rt.push_null();
    };

    auto get_property = [](Runtime &rt) {
    	auto annot = rt.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto category = rt.to_string(1);
    	auto prop = annot->get_property(category);

    	if (prop.valid())
	    {
    		if (prop.is_text())
    			rt.push(prop.value());
    		else if (prop.is_numeric())
    			rt.push(prop.numeric_value());
    		else if (prop.is_boolean())
    			rt.push_boolean(prop.boolean_value());
		    else
		    {
		    	throw error("[Internal error] Invalid property type");
		    }
	    }
	    else
	    {
	    	rt.push_null();
	    }
    };

    rt.push(metaobject);
    {
        rt.add_accessor("path", annot_path);
        rt.add_method("Annotation.meta.add_property", add_property, 2);
	    rt.add_method("Annotation.meta.remove_property", remove_property, 2);
	    rt.add_method("Annotation.meta.get_property", get_property, 2);
        rt.add_method("Annotation.meta.bind_to_sound", bind_to_sound, 1);
    }
    rt.new_native_constructor(new_annot, new_annot, "Annotation", 1);
    rt.def_global("Annotation", PHON_DONTENUM);


}

bool Annotation::modified() const
{
    return VFile::modified() || m_graph.modified();
}

bool Annotation::content_modified() const
{
	return m_graph.modified() || VFile::content_modified();
}

String Annotation::left_context(const EventList &events, intptr_t i, String::const_iterator start, intptr_t length,
                                const String &separator)
{
	String context(length);
	context.append(events[i]->text().rmid(start, length));

	while (context.grapheme_count() != length && --i > 0)
	{
		auto &label = events[i]->text();
		auto prefix = label.right(length - context.grapheme_count() - separator.size());
		context.prepend(separator);
		context.prepend(prefix);
	}

	return context;
}

String Annotation::right_context(const EventList &events, intptr_t i, String::const_iterator end, intptr_t length,
                                 const String &separator)
{
	String context(length);
	context.append(events[i]->text().mid(end, length));

	while (context.grapheme_count() != length && ++i <= events.size())
	{
		auto &label = events[i]->text();
		auto suffix = label.left(length - context.grapheme_count() - separator.size());
		context.append(separator);
		context.append(suffix);
	}

	return context;
}

void Annotation::set_event_text(AutoEvent &event, const String &new_text)
{
	m_graph.set_event_text(event, new_text);
}

void Annotation::metadata_to_xml(xml_node meta_node)
{
	VFile::metadata_to_xml(meta_node);
	String snd = has_sound() ? sound()->path() : String();
	auto project = Project::instance();
	filesystem::compress(snd, project->directory());
	add_data_node(meta_node, "Sound", snd);
}

void Annotation::write_as_native(const String &path)
{
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
	m_graph.write_textgrid(path);
}
} // namespace phonometrica