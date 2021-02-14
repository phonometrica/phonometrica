/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/conc/match.hpp>

namespace phonometrica {

Match::Target::Target(const AutoEvent &e, String value, intptr_t layer, intptr_t offset, bool is_ref) :
		event(e), value(std::move(value)), layer((int)layer), offset((int)offset), is_reference(is_ref)
{

}

Match::Match(const AutoAnnotation &annot, std::unique_ptr<Target> t) :
	m_annot(annot), m_target(std::move(t))
{

}

Match::Target *Match::get(intptr_t i) const
{
	intptr_t n = 0;
	auto t = m_target.get();

	while (++n < i)
	{
		assert(t->next);
		t = t->next.get();
	}

	return t;
}

const AutoEvent &Match::get_event(intptr_t i) const
{
	return get(i)->event;
}

intptr_t Match::get_layer(intptr_t i) const
{
	return get(i)->layer;
}

intptr_t Match::get_offset(intptr_t i) const
{
	return get(i)->offset;
}

String Match::get_value(intptr_t i) const
{
	return get(i)->value;
}

const AutoAnnotation &Match::annotation() const
{
	return m_annot;
}

Match::Target &Match::last_target()
{
	auto t = m_target.get();

	while (t->next)
	{
		t = t->next.get();
	}

	return *t;
}

Match::Target *Match::reference_target() const
{
	auto t = m_target.get();

	do {
		if (t->is_reference) {
			return t;
		}
		t = t->next.get();
	}
	while (t);

	return nullptr;
}

void Match::to_xml(xml_node root) const
{
	auto node = root.append_child("Match");
	add_data_node(node, "Annotation", m_annot->path().data());
	auto targets_node = node.append_child("Targets");
	auto target = m_target.get();

	while (target)
	{
		auto index = m_annot->get_event_index(target->layer, target->start_time());
		assert(index != 0);
		auto subnode = targets_node.append_child("Target");
		subnode.append_attribute("layer").set_value(target->layer);
		subnode.append_attribute("event").set_value(index);
		subnode.append_attribute("offset").set_value(target->offset);
		subnode.append_attribute("ref").set_value(target->is_reference ? "true" : "false");
		subnode.append_child(node_pcdata).set_value(target->value.data());
		target = target->next.get();
	}
}

bool Match::valid()
{
	return m_annot != nullptr && m_target != nullptr;
}

void Match::append(std::unique_ptr<Target> next)
{
	auto t = m_target.get();

	while (t) {
		t = t->next.get();
	}
}

double Match::get_start_time(intptr_t i) const
{
	return get_event(i)->start_time();
}

double Match::get_end_time(intptr_t i) const
{
	return get_event(i)->end_time();
}

} // namespace phonometrica
