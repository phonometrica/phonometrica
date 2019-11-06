/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
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
 * Purpose: hash table built on top of std::unordered_map.                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_HASHMAP_HPP
#define PHONOMETRICA_HASHMAP_HPP

#include <unordered_map>
#include <phon/definitions.hpp>
#include <phon/array.hpp>

namespace phonometrica {

template<typename Key, typename Val>
class Hashmap
{
    using storage_type = std::unordered_map<Key,Val>;
	
public:

	using iterator = typename storage_type::iterator;
	using const_iterator = typename storage_type::const_iterator;
	using value_type = typename storage_type::value_type;
	
	Hashmap() = default;
	
	Hashmap(const Hashmap &other) = default;
	
	Hashmap(Hashmap &&other) noexcept = default;
	
	~Hashmap() = default;

	Hashmap &operator=(const Hashmap &other)
	{
		if (this != &other)
		{
			this->m_map = other.m_map;
		}

		return *this;
	}

	Hashmap &operator=(Hashmap &&other) noexcept
	{
		this->m_map = std::move(other.m_map);
		return *this;
	}

	bool operator==(const Hashmap &other) const
	{
		return this->m_map == other.m_map;
	}

	std::pair<iterator,bool> insert(const Key &key, const Val &value)
	{
		return m_map.insert({key, value});
	}

	std::pair<iterator,bool> insert(const Key &key, Val &&value)
	{
		return m_map.insert({key, std::move(value)});
	}

	const Val &operator[](const Key &key) const
	{
		return m_map[key];
	}

	Val &operator[](const Key &key)
	{
		return m_map[key];
	}

	bool contains(const Key &key) const
	{
		return m_map.find(key) != m_map.end();
	}

	iterator begin()
	{
		return m_map.begin();
	}

	const_iterator begin() const
	{
		return m_map.begin();
	}

	const_iterator cbegin() const
	{
		return m_map.cbegin();
	}

	iterator end()
	{
		return m_map.end();
	}

	const_iterator end() const
	{
		return m_map.end();
	}

	const_iterator cend() const
	{
		return m_map.cend();
	}

	const_iterator find(const Key &key) const
	{
		return m_map.find(key);
	}

	iterator find(const Key &key)
	{
		return m_map.find(key);
	}

	void remove(const Key &key)
	{
		m_map.erase(key);
	}

	intptr_t size() const
	{
		return intptr_t(m_map.size());
	}

	bool empty() const
	{
		return m_map.empty();
	}

	Array<Key> keys() const
	{
		Array<Key> result;
		result.reserve(m_map.size());

		for (auto &it : m_map) {
			result.append(it.first);
		}

		return result;
	}

	Array<Key> values() const
	{
		Array<Key> result;
		result.reserve(m_map.size());

		for (auto &it : m_map) {
			result.append(it.second);
		}

		return result;
	}

	void clear()
    {
	    m_map.clear();
    }

private:
	
	storage_type m_map;
};

} // namespace phonometrica

#endif // PHONOMETRICA_HASHMAP_HPP
