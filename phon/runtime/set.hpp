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
 * Purpose: Set type (ordered set).                                                                                    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SET_HPP
#define PHONOMETRICA_SET_HPP

#include <set>
#include <phon/runtime/variant.hpp>

namespace phonometrica {

class Set final
{
public:

	using Storage = std::set<Variant>;
	using iterator = Storage::iterator;

	Set() = default;

	Set(Storage items) : _items(std::move(items)) { }

	Set(const Set &other);

	Set(Set &&other) = default;

	bool operator==(const Set &other) const;

	iterator begin() { return _items.begin(); }

	iterator end() { return _items.end(); }

	Storage &items() { return _items; }

	String to_string() const;

	void traverse(const GCCallback &callback);

	bool contains(const Variant &v) const { return _items.find(v) != _items.end(); }

	intptr_t size() const { return intptr_t(_items.size()); }

private:

	Storage _items;
	mutable bool seen = false;
};



//---------------------------------------------------------------------------------------------------------------------

namespace meta {

static inline void traverse(Set &set, const GCCallback &callback)
{
	set.traverse(callback);
}


static inline String to_string(const Set &set)
{
	return set.to_string();
}
} // namespace phonometrica::meta
} // namespace phonometrica

#endif // PHONOMETRICA_SET_HPP
