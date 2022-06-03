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
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Table type (dynamic hash table).                                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TABLE_HPP
#define PHONOMETRICA_TABLE_HPP

#include <phon/hashmap.hpp>
#include <phon/runtime/variant.hpp>

namespace phonometrica {


class Table final
{
public:

	using Storage = Hashmap<Variant, Variant>;
	using iterator = Storage::iterator;
	using const_iterator = Storage::const_iterator;

	Table() = default;

	Table(Storage dat) : _map(std::move(dat)) { }

	Table(Table &&) = default;

	Table(const Table &other);

	bool operator==(const Table &other) const;

	Storage &data() { return _map; }

	const Storage &data() const { return _map; }

	intptr_t size() const { return intptr_t(_map.size()); }

	Variant &get(const Variant &key);

	Variant &operator[](const Variant &key) { return _map[key]; }

	String to_string() const;

	String to_json(int spacing) const;

	void traverse(const GCCallback &callback);

	Array<Variant> keys() const
	{
		Array<Variant> result;
		result.reserve(_map.size());

		for (auto &it : _map) {
			result.append(it.first);
		}

		return result;
	}

	Array<Variant> values() const
	{
		Array<Variant> result;
		result.reserve(_map.size());

		for (auto &it : _map) {
			result.append(it.second);
		}

		return result;
	}

	Storage &map() { return _map; }

	const Storage &map() const { return _map; }

private:

	Storage _map;
	mutable bool seen = false;
};


//---------------------------------------------------------------------------------------------------------------------

namespace meta {

static inline void traverse(Table &tab, const GCCallback &callback)
{
	tab.traverse(callback);
}


static inline String to_string(const Table &tab)
{
	return tab.to_string();
}
} // namespace phonometrica::meta
} // namespace phonometrica

#endif // PHONOMETRICA_TABLE_HPP
