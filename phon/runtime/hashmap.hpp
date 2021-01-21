/***********************************************************************************************************************
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
 * Created: 01/12/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: hash table (aka associative array) mapping keys to values. This implementation uses open addressing with   *
 * Robin Hood hashing. Furthermore, iterators are guaranteed to point to an element inside the hashmap's array, even   *
 * if they have been invalidated. The interface is similar (but not identical) to that of std::unordered_map.          *
 * This implementation is partly based on:                                                                             *
 * https://www.sebastiansylvan.com/post/robin-hood-hashing-should-be-your-default-hash-table-implementation/           *
 * See also the following link for an overview of Robin Hood Hashing:                                                  *
 * https://programming.guide/robin-hood-hashing.html                                                                   *
 *                                                                                                                     *
 * Note: If the macro PHON_STD_UNORDERED_MAP is defined, Hashmap will simply be an alias for std::unordered_map.       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_HASHMAP_HPP
#define PHONOMETRICA_HASHMAP_HPP

#include <functional>
#ifdef PHON_STD_UNORDERED_MAP

#include <unordered_map>

namespace phonometrica {

template <class Key,
		class T,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>>
using Hashmap = std::unordered_map<Key, T, Hash, KeyEqual>;

} // namespace phonometrica

#else // PHON_STD_UNORDERED_MAP

#include <exception>
#include <functional>
#include <iterator>
#include <type_traits>
#include <phon/runtime/array.hpp>


namespace phonometrica {

template <class Key,
		class T,
		class Hash = std::hash<Key>,
		class KeyEqual = std::equal_to<Key>,
		class Allocator = std::allocator<std::pair<const Key, T>>,
		int LoadFactor = 67> // Maximum load factor (percentage)
class Hashmap final
{
public:

	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<Key, T>;
	using size_type = std::intptr_t;
	using difference_type = std::ptrdiff_t;
	using hasher = Hash;
	using key_equal = KeyEqual;
	using allocator_type = Allocator;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

	class iterator
	{
	public:

		using iterator_category = std::forward_iterator_tag;
	    using value_type = Hashmap::value_type;
	    using difference_type = ptrdiff_t;
	    using pointer = value_type*;
	    using reference = value_type&;

		iterator(Hashmap *map, size_type pos)
		{
			if (map->empty()) {
				pos = map->capacity();
			}
			while (pos < map->capacity() && !map->node(pos)->used()) {
				++pos;
			}
			this->pos = pos;
			this->map = map;
		}

		iterator(const iterator &other) noexcept = default;

		bool operator==(const iterator &other) const noexcept
		{
			return this->pos == other.pos && this->map == other.map;
		}

		bool operator!=(const iterator &other) const noexcept
		{
			return this->pos != other.pos || this->map != other.map;
		}

		iterator &operator++()
		{
			do { ++pos; } while (pos < map->capacity() && !map->node(pos)->used());
			return (*this);
		}

		iterator operator++(int)
		{
			auto tmp(*this);
			do { ++pos; } while (pos < map->capacity() && !map->node(pos)->used());
			return tmp;
		}

		value_type & operator*() {
			return *map->get_value(pos);
		}

		const value_type & operator*() const {
			return *map->get_value(pos);
		}

		value_type *operator->() {
			return map->get_value(pos);
		}

		const value_type *operator->() const {
			return map->get_value(pos);
		}

		const key_type &key() const {
			return map->get_value(pos)->first;
		}

		mapped_type &value() {
			return map->get_value(pos)->second;
		}

		const mapped_type &value() const {
			return map->get_value(pos)->second;
		}

		bool valid() const {
			return map->node(pos)->used();
		}

		size_type index() const {
			return pos;
		}

	private:

		Hashmap *map;
		size_type pos;
	};

	using const_iterator = const iterator;

private:

	friend iterator;

	// 0 is used to indicate that the slot is unused
	static constexpr size_t Empty = 0;

	struct Node
	{
		bool used() const noexcept {
			return hash != Empty;
		}

		~Node() { clear(); }

		void clear() {
			hash = Empty;
		}

		value_type value;
		size_t hash;
	};

	using storage_type = typename std::aligned_storage<sizeof(Node), alignof(Node)>::type;

public:

	iterator begin() noexcept
	{
		return iterator(this, 0);
	}

	const_iterator begin() const noexcept
	{
		return iterator(const_cast<Hashmap*>(this), 0);
	}

	const_iterator cbegin() const noexcept
	{
		return iterator(const_cast<Hashmap*>(this), 0);
	}

	iterator end() noexcept
	{
		return iterator(this, this->capacity());
	}

	const_iterator end() const noexcept
	{
		return iterator(const_cast<Hashmap*>(this), this->capacity());
	}

	const_iterator cend() const noexcept
	{
		return iterator(const_cast<Hashmap*>(this), this->capacity());
	}


	Hashmap() = default;

	explicit Hashmap(size_type capacity)
	{
		capacity = next_power2(capacity);
		m_size = 0;
		set_capacity(capacity);
		m_data = allocate(capacity);
	}

	Hashmap(std::initializer_list<value_type> init) :
		Hashmap(init.size() * 2)
	{
		for (auto &value : init)
		{
			this->insert(value);
		}
	}

	Hashmap(const Hashmap &other) :
			Hashmap((other.size() + 1) * LoadFactor / 100)
	{
		for (auto &value : other)
		{
			insert(value);
		}
	}

	Hashmap(Hashmap &&other) noexcept
	{
		m_size = other.size();
		m_data = other.m_data;
		set_capacity(other.capacity());
		other.m_data = nullptr;
	}

	~Hashmap()
	{
		if (m_data != nullptr)
		{
			deallocate();
		}
	}

	Hashmap &operator=(const Hashmap &other)
	{
		if (this != &other)
		{
			Hashmap tmp(other);
			this->swap(tmp);
		}

		return *this;
	}

	Hashmap &operator=(Hashmap &&other) noexcept
	{
		swap(other);
		return *this;
	}

	bool operator==(const Hashmap &other) const
	{
		if (this->size() != other.size()) {
			return false;
		}
		// We can't assume that items are in the same order,
		// so we check that each item in this is also in other.
		for (auto &pair : *this)
		{
			auto it = other.find(pair.first);
			if (it == other.end() || pair.second != it.value()) {
				return false;
			}
		}

		return true;
	}

	size_type size() const noexcept
	{
		return m_size;
	}

	size_type capacity() const noexcept
	{
		return m_mask + 1;
	}

	size_type bucket_count() const noexcept
	{
		return this->capacity();
	}

	float load_factor() const
	{
		return float(double(size()) / bucket_count());
	}

	bool empty() const noexcept
	{
		return m_size == 0;
	}

	void swap(Hashmap &other) noexcept
	{
		std::swap(m_size, other.m_size);
		std::swap(m_mask, other.m_mask);
		std::swap(m_data, other.m_data);
	}

	std::pair<iterator, bool> insert(const value_type &value)
	{
		bool inserted;
		auto pos = insert_entry(hash_entry(value.first), value, inserted);
		if (inserted) ++m_size;

		return { iterator(this, pos), inserted };
	}

	std::pair<iterator, bool> insert(value_type &&value)
	{
		bool inserted;
		auto hash = hash_entry(value.first);
		auto pos = insert_entry(hash, std::forward<value_type>(value), inserted);
		if (inserted) ++m_size;

		return { iterator(this, pos), inserted };
	}

	mapped_type &operator[](const key_type &key)
	{
		auto it = find(key);

		if (it != this->end())
		{
			return (*it).second;
		}
		auto result = insert({ key, mapped_type() });

		return result.first->second;
	}

	mapped_type &at(const key_type &key)
	{
		auto it = find(key);

		if (it != this->end()) {
			return (*it).second;
		}

		throw std::out_of_range("");
	}

	const mapped_type &at(const key_type &key) const
	{
		return const_cast<Hashmap*>(this)->at(key);
	}

	iterator find(const key_type &key)
	{
		return this->empty() ? this->end() : lookup(key);
	}

	const_iterator find(const key_type &key) const
	{
		return this->empty() ? this->cend() : const_cast<Hashmap*>(this)->lookup(key);
	}

	bool contains(const key_type &key) const
	{
		return find(key) != this->end();
	}

	void clear()
	{
		auto dat = reinterpret_cast<Node*>(m_data);
		auto capacity = this->capacity();

		for (size_type i = 0; i < capacity; ++i)
		{
			auto n = &dat[i];
			if (n->used()) n->~Node();
		}

		m_size = 0;
	}

	void reserve(size_type requested)
	{
		requested = next_power2(requested);
		if (requested > this->capacity())
		{
			rehash(requested);
		}
	}

	void erase(const key_type &key)
	{
		erase(find(key));
	}

	void erase(const_iterator it)
	{
		if (it == this->cend()) {
			return;
		}

		auto pos = it.index();
		auto previous = node(pos);
		previous->~Node();
		pos = (pos + 1) & m_mask;
		auto current = node(pos);

		// Backwards shift deletion
		while (current->used() && probe_distance(current->hash, pos) != 0)
		{
			previous->hash = current->hash;
			new (&previous->value) value_type(std::move(current->value));
			current->hash = Empty;
			pos = (pos + 1) & m_mask;
			previous = current;
			current = node(pos);
		}

		m_size--;
	}

	float max_load_factor() const
	{
		return float(LoadFactor) / 100;
	}

	Array<Key> keys() const
	{
		Array<Key> result;
		result.reserve(size());

		for (auto &it : *this) {
			result.append(it.first);
		}

		return result;
	}

	Array<Key> values() const
	{
		Array<Key> result;
		result.reserve(size());

		for (auto &it : *this) {
			result.append(it.second);
		}

		return result;
	}

private:

	storage_type *allocate(size_type count)
	{
		auto dat = new storage_type[count];

		for (size_type i = 0; i < count; ++i)
		{
			reinterpret_cast<Node*>(&dat[i])->clear();
		}

		return dat;
	}

	void deallocate()
	{
		clear();
		delete[] m_data;
	}

	void ensure_capacity()
	{
		if (this->size() + 1 > this->capacity() * LoadFactor / 100)
		{
			size_type new_capacity = this->empty() ? 8 : this->capacity() * 2;
			rehash(new_capacity);
		}
	}

	void rehash(size_type new_capacity)
	{
		auto old_data = reinterpret_cast<Node*>(m_data);
		auto old_capacity = this->capacity();
		set_capacity(new_capacity);
		m_data = allocate(new_capacity);

		for (size_type i = 0; i < old_capacity; ++i)
		{
			auto node = &old_data[i];
			if (node->used())
			{
				bool inserted;
				insert_entry(node->hash, std::move(node->value), inserted);
			}
		}

		// Values have already been moved, so we just need to delete the storage.
		delete[] reinterpret_cast<storage_type*>(old_data);
	}

	size_t &get_hash(size_type pos)
	{
		return reinterpret_cast<Node*>(m_data + pos)->hash;
	}

	Node *node(size_type pos)
	{
		return reinterpret_cast<Node*>(m_data + pos);
	}

	const Node *node(size_type pos) const
	{
		return reinterpret_cast<const Node *>(m_data + pos);
	}

	value_type *get_value(size_type pos)
	{
		return &node(pos)->value;
	}

	const value_type *get_value(size_type pos) const
	{
		return &node(pos)->value;
	}

	size_t hash_entry(const Key &key)
	{
		auto h = hasher()(key);
		// Ensure that the key is never 0.
		return h | (h == 0);
	}

	size_type get_position(size_t hash) const
	{
		return size_type(hash & m_mask);
	}

	void construct(size_type pos, size_t hash, value_type &&value)
	{
		auto n = node(pos);
		new (&n->value) value_type(std::move(value));
		n->hash = hash;
	}

	size_type probe_distance(size_t hash, size_type pos) const
	{
		return (pos + this->capacity() - get_position(hash)) & m_mask;
	}

	iterator lookup(const key_type &key)
	{
		auto hash = hash_entry(key);
		auto pos = get_position(hash);
		size_type dist = 0;

		while (true)
		{
			Node *current_node = node(pos);

			if (!current_node->used() || dist > probe_distance(current_node->hash, pos))
			{
				return this->end(); // not found
			}
			else if (hash == current_node->hash && key_equal()(current_node->value.first, key))
			{
				 return { this,  pos }; // found
			}

			pos = (pos + 1) & m_mask;
		}
	}

	size_type insert_entry(size_t hash, value_type value, bool &inserted)
	{
		ensure_capacity();
		bool found = false;
		auto cached_pos = this->capacity();

		auto pos = get_position(hash);
		size_type dist = 0;

		while (true)
		{
			auto current_node = node(pos);

			// If the slot is free, insert the element in its preferred position.
			if (!current_node->used())
			{
				construct(pos, hash, std::move(value));
				// If the original element has been inserted in a non-optimal position, return that position
				if (found) pos = cached_pos;
				inserted = true;

				return pos;
			}

			// Check if an entry with the same key already exists.
			if (current_node->hash == hash && key_equal()(current_node->value.first, value.first))
			{
				// Update value
				current_node->value.second = std::move(value.second);
				inserted = found;
				return found ? cached_pos : pos;
			}

			// Find the next best position: perform linear probing, permuting elements if an existing element has a
			// smaller probe distance than the element we are trying to insert.
			auto candidate_dist = probe_distance(current_node->hash, pos);

			// Steal from the rich and give to the poor...
			if (candidate_dist < dist)
			{
				std::swap(hash, current_node->hash);
				std::swap(value, current_node->value);
				dist = candidate_dist;

				// Cache the position the first time the value is found, since this means it has been inserted
				// in its preferred position.
				if (!found)
				{
					cached_pos = pos;
					found = true;
				}
			}

			pos = (pos + 1) & m_mask;
			++dist;
		}
	}

	// Find the first power of 2 that is not smaller than x
	size_type next_power2(size_type x)
	{
		if (x <= 1) return 1;
		size_type next = 2;
		x--;
		while (x >>= 1) next <<= 1;

		return next;
	}

	void set_capacity(size_type capacity)
	{
		m_mask = capacity - 1;
	}

	size_type m_size = 0;
	size_t m_mask = -1; // capacity - 1
	storage_type *m_data = nullptr;
};

} // namespace phonometrica

#endif // PHON_STD_UNORDERED_MAP
#endif // PHONOMETRICA_HASHMAP_HPP
