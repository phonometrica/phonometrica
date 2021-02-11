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

} // namespace phonometrica
