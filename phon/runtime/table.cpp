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
 * Created: 02/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/runtime/table.hpp>

namespace phonometrica {

Table::Table(const Table &other)
{
	_map.reserve(other.size());

	for (auto &pair : other._map) {
		_map.insert({ pair.first.resolve(), pair.second.resolve() });
	}
}

Variant &Table::get(const Variant &key)
{
	auto it = _map.find(key);

	if (it == _map.end()) {
		throw error("[Index error] Missing key in table: %", key.to_string(true));
	}

	return it->second;
}

String Table::to_string() const
{
	if (this->seen)
	{
		return "{...}";
	}

	bool flag = this->seen;
	this->seen = true;
	String s("{");

	for (auto &pair : _map)
	{
		s.append(pair.first.to_string(true));
		s.append(": ");
		s.append(pair.second.to_string(true));
		s.append(", ");
	}
	s.remove_last(", ");
	s.append('}');
	this->seen = flag;

	return s;
}

void Table::traverse(const GCCallback &callback)
{
	for (auto &pair : _map)
	{
#ifdef PHON_STD_UNORDERED_MAP
		const_cast<Variant&>(pair.first).traverse(callback);
#else
		pair.first.traverse(callback);
#endif
		pair.second.traverse(callback);
	}
}

String Table::to_json(int spacing) const
{
	if (this->seen) {
		throw error("[JSON error] Cannot convert recursive table to JSON");
	}

	bool flag = this->seen;
	String s("{\n");
	auto keys = this->keys();
	// Sort the keys so that we have a predictable order.
	std::sort(keys.begin(), keys.end());

	for (intptr_t i = 1; i <= keys.size(); i++)
	{
		for (int c = 0; c < spacing; c++) {
			s.append('\t');
		}
		auto &key = keys[i];
		s.append(key.to_json(spacing + 1));
		s.append(": ");
		auto it = _map.find(key);
		s.append(it->second.to_json(spacing + 1));
		if (i != keys.size()) {
			s.append(',');
		}
		s.append('\n');
	}
	for (int c = 0; c < spacing-1; c++) {
		s.append('\t');
	}
	s.append('}');
	this->seen = flag;

	return s;
}

bool Table::operator==(const Table &other) const
{
	return this->map() == other.map();
}


} // namespace phonometrica