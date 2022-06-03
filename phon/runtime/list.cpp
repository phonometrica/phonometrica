/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Created: 01/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/list.hpp>

namespace phonometrica {

List::List(const List &other) : _items(other._items)
{
	// When we clone a list, we need to make sure that aliases are resolved, otherwise both lists may get mutated if
	// we mutate a reference in one of them.
	for (auto &item : _items) {
		item.unalias();
	}
}

void List::traverse(const GCCallback &callback)
{
	for (auto &item : _items) {
		item.traverse(callback);
	}
}

String List::to_string() const
{
	if (this->seen)
	{
		return "[...]";
	}

	bool flag = this->seen;
	String s("[");
	for (intptr_t i = 1; i <= _items.size(); i++)
	{
		s.append(_items[i].to_string(true));
		if (i < _items.size()) {
			s.append(", ");
		}
	}
	s.append(']');
	this->seen = flag;

	return s;
}

bool List::operator==(const List &other) const
{
	return this->items() == other.items();
}

String List::to_json(int spacing) const
{
	if (this->seen) {
		throw error("[JSON error] Cannot convert recursive list to JSON");
	}

	bool flag = this->seen;
	String s("[\n");
	for (intptr_t i = 1; i <= _items.size(); i++)
	{
		for (int c = 0; c < spacing; c++) {
			s.append('\t');
		}
		s.append(_items[i].to_json(spacing+1));
		if (i < _items.size()) {
			s.append(',');
		}
		s.append('\n');
	}
	for (int c = 0; c < spacing-1; c++) {
		s.append('\t');
	}
	s.append(']');
	this->seen = flag;

	return s;
}

List &List::operator=(List other)
{
	swap(other);
	return *this;
}

void List::swap(List &other) noexcept
{
	_items.swap(other._items);
	std::swap(this->seen, other.seen);
}

} // namespace phonometrica