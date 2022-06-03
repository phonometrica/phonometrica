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
 * Created: 05/06/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/set.hpp>

namespace phonometrica {

Set::Set(const Set &other)
{
	for (auto &val : other._items) {
		_items.insert(val.resolve());
	}
}

String Set::to_string() const
{
	if (this->seen)
	{
		return "{...}";
	}

	bool flag = this->seen;
	String s("{");

	for (auto &val : _items)
	{
		s.append(val.to_string(true));
		s.append(", ");
	}
	s.remove_last(", ");
	s.append('}');
	this->seen = flag;

	return s;
}

void Set::traverse(const GCCallback &callback)
{
	for (auto &val : _items) {
		const_cast<Variant&>(val).traverse(callback);
	}
}

bool Set::operator==(const Set &other) const
{
	return this->_items == other._items;
}

} // namespace phonometrica