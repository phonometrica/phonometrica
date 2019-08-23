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
#include <phon/runtime/environment.hpp>
#include <phon/runtime/object.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Object *Annotation::metaobject = nullptr;

Annotation::Annotation(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{
	guess_type();
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
		case Type::TextGrid:
			m_graph.write_textgrid(m_path);
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

		if (ext == ".dmf") {
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

EventList Annotation::get_layer_events(size_t i) const
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

void Annotation::initialize(Environment &env)
{
    metaobject = new Object(env, PHON_CUSERDATA, env.object_meta);
    env.permanent_objects.append(metaobject);

    auto new_annot = [](Environment &env) {
        env.push_null(); // TODO: Annotation ctor
    };

    auto annot_path = [](Environment &env) {
        auto annot = env.cast_user_data<std::shared_ptr<Annotation>>(0);
        env.push(annot->path());
    };

    auto add_property = [](Environment &env) {
    	auto annot = env.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto category = env.to_string(1);
    	std::any value;
    	if (env.is_boolean(2))
    		value = env.to_boolean(2);
    	else if (env.is_number(2))
    		value = env.to_number(2);
		else
			value = env.to_string(2);

		annot->add_property(Property(std::move(category), std::move(value)));
		env.push_null();
    };

    auto bind_to_sound = [](Environment &env) {
    	auto annot = env.cast_user_data<std::shared_ptr<Annotation>>(0);
    	auto path = env.to_string(1);
    	auto project = Project::instance();
    	project->import_file(path);
    	auto snd = downcast<Sound>(project->get(path));
    	if (snd) annot->set_sound(snd);
    	env.push_null();
    };

    env.push(metaobject);
    {
        env.add_accessor("path", annot_path);
        env.add_method("Annotation.meta.add_property", add_property, 2);
        env.add_method("Annotation.meta.bind_to_sound", bind_to_sound, 1);
    }
    env.new_native_constructor(new_annot, new_annot, "Annotation", 1);
    env.def_global("Annotation", PHON_DONTENUM);


}

bool Annotation::modified() const
{
    return VFile::modified() || m_graph.modified();
}

bool Annotation::content_modified() const
{
	return m_graph.modified() || VFile::content_modified();
}
} // namespace phonometrica