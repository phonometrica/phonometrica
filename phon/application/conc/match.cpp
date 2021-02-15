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

bool Match::Target::operator==(const Match::Target &other) const
{
	return (this->event == other.event && this->offset == other.offset && this->value == other.value);
}

bool Match::Target::operator!=(const Match::Target &other) const
{
	return !(*this == other);
}

bool Match::Target::operator<(const Match::Target &other) const
{
	// Same layer?
	if (this->layer < other.layer) {
		return true;
	}
	if (this->layer > other.layer) {
		return false;
	}

	// Same event?
	if (this->event->start_time() < other.event->start_time()) {
		return true;
	}
	if (this->event->start_time() > other.event->start_time()) {
		return false;
	}

	// Compare offsets: we don't need too look at the value since values at a given offset will
	// always be identical.
	return this->offset < other.offset;
}

Match::Match(const AutoAnnotation &annot, std::unique_ptr<Target> t) :
	m_annot(annot), m_target(std::move(t))
{

}

Match::Match(const Match &other) : m_annot(other.annotation())
{
	auto target = other.m_target.get();
	m_target = std::make_unique<Target>(target->event, target->value, target->layer, target->offset, target->is_reference);
	target = target->next.get();
	auto new_target = m_target.get();

	while (target)
	{
		new_target->next = std::make_unique<Target>(target->event, target->value, target->layer, target->offset, target->is_reference);
		target = target->next.get();
		new_target = new_target->next.get();
	}
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

bool Match::operator==(const Match &other) const
{
	if (m_annot->path() != other.annotation()->path()) {
		return false;
	}

	Target *t1 = m_target.get();
	Target *t2 = other.m_target.get();

	while (t1)
	{
		if (!t2) {
			return false;
		}
		if (*t1 != *t2) {
			return false;
		}

		t1 = t1->next.get();
		t2 = t2->next.get();
	}

	return true;
}

bool Match::operator!=(const Match &other) const
{
	return !(*this == other);
}

bool Match::operator<(const Match &other) const
{
	if (this->annotation()->path() < other.annotation()->path()) {
		return true;
	}
	if (this->annotation()->path() > other.annotation()->path()) {
		return false;
	}

	auto t1 = m_target.get();
	auto t2 = other.m_target.get();

	while (t1)
	{
		// A match with fewer targets is ranked lower.
		if (!t2) {
			return true;
		}
		if (*t1 < *t2) {
			return true;
		}
		if (*t1 != *t2) {
			return false;
		}
		t1 = t1->next.get();
		t2 = t2->next.get();
	}

	// Matches are equal
	return false;
}

bool Match::update(bool &modified)
{
	auto target = m_target.get();
	modified = false;

	while (target)
	{
		auto &text = target->event->text();

		if (target->offset + target->value.size() > text.size()) {
			return false;
		}

		auto start = text.begin() + target->offset;
		String new_value(start, target->value.size());
		modified = (new_value != target->value);

		// TODO: there must be an undo operation synchronized with the match
		if (modified) {
			target->value = new_value;
		}

		target = target->next.get();
	}

	return true;
}

} // namespace phonometrica
