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
 * Created: 23/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: List type (dynamic array of variants).                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_LIST_HPP
#define PHONOMETRICA_LIST_HPP

#include <phon/array.hpp>
#include <phon/runtime/variant.hpp>

namespace phonometrica {

class List final
{
public:

	using Storage = Array<Variant>;
	using iterator = Storage::iterator;
	using const_iterator = Storage::const_iterator;

	List() = default;

	List(std::initializer_list<Variant> lst) : _items(lst) { }

	explicit List(intptr_t size) : _items(size, Variant()) { }

	List(const List &other);

	List(List &&other) noexcept = default;

	List(Storage items) : _items(std::move(items)) { }

	List &operator=(List other);

	bool operator==(const List &other) const;

	intptr_t size() const { return _items.size(); }

	Variant *data() { return _items.data(); }

	const Variant *data() const { return _items.data(); }

	Variant &operator[](intptr_t i) { return _items[i]; }

	Variant &at(intptr_t i) { return _items.at(i); }

	iterator begin() { return _items.begin(); }
	const_iterator cbegin() { return _items.begin(); }

	iterator end() { return _items.end(); }
	const_iterator cend() { return _items.end(); }

	void traverse(const GCCallback &callback);

	Storage &items() { return _items; }
	const Storage &items() const { return _items; }

	String to_string() const;

	String to_json(int spacing) const;

	void swap(List &other) noexcept;

private:

	Storage _items;
	mutable bool seen = false; // for printing
};


//---------------------------------------------------------------------------------------------------------------------

namespace meta {

static inline void traverse(List &lst, const GCCallback &callback)
{
	lst.traverse(callback);
}


static inline String to_string(const List &lst)
{
	return lst.to_string();
}

} // namespace phonometrica::meta

} // namespace phonometrica

#endif // PHONOMETRICA_LIST_HPP
