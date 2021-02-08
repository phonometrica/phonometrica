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


Match::Match(const AutoEvent &e, String target, int layer, int offset)
{
	m_target = std::make_unique<Target>();
	m_target->event = e;
	m_target->value = std::move(target);
	m_target->layer = layer,
	m_target->offset = offset;
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

int Match::get_layer(intptr_t i) const
{
	return get(i)->layer;
}

int Match::get_offset(intptr_t i) const
{
	return get(i)->offset;
}

String Match::get_target(intptr_t i) const
{
	return get(i)->value;
}

void Match::append(std::unique_ptr<Target> new_target)
{
	auto t = m_target.get();

	while (t->next)
	{
		t = t->next.get();
	}

	t->next = std::move(new_target);
}

const AutoAnnotation &Match::annotation() const
{
	return m_annot;
}

} // namespace phonometrica
