/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 06/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: JSON value (wrapper over a Variant).                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_JSON_HPP
#define PHONOMETRICA_JSON_HPP

#include <phon/runtime/variant.hpp>
#include <phon/runtime/list.hpp>
#include <phon/runtime/table.hpp>

namespace phonometrica {

class Json final
{
public:

	using Array = phonometrica::List::Storage;

	using Object = phonometrica::Table::Storage;

	class iterator final
	{
	public:

		using iterator_category = std::forward_iterator_tag;
		using value_type = std::pair<Json,Json>;
		using difference_type = ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		iterator(Object::iterator it) : m_iter(it) { }

		Json key() const { return m_iter->first; }

		Json value() const { return m_iter->second; }

		value_type operator*() const { return { Json(m_iter->first), Json(m_iter->second) }; }

		bool operator==(const iterator &other) const noexcept
		{
			return m_iter == other.m_iter;
		}

		bool operator!=(const iterator &other) const noexcept
		{
			return m_iter != other.m_iter;
		}

		iterator &operator++()
		{
			++m_iter;
			return *this;
		}

		iterator operator++(int)
		{
			auto tmp(*this);
			++m_iter;
			return tmp;
		}

		bool is_null() const { return m_iter->second.empty(); }

		String get_name() const { return cast<String>(m_iter->first); }

		bool get_boolean() const { return cast<bool>(m_iter->second); }

		intptr_t get_integer() const { return cast<intptr_t>(m_iter->second); }

		double get_float() const { return cast<double>(m_iter->second); }

		String get_string() const { return cast<String>(m_iter->second); }

		Json::Array &get_array() { return cast<List>(m_iter->second).items(); }
		const Json::Array &get_array() const { return cast<List>(m_iter->second).items(); }

		Json::Object &get_object() { return cast<Table>(m_iter->second).data(); }
		const Json::Object &get_object() const { return cast<Table>(m_iter->second).data(); }

	private:

		Object::iterator m_iter;
	};

	using const_iterator = const iterator;

	iterator begin() noexcept { return get_object().begin(); }

	const_iterator begin() const noexcept { return get_object().begin(); }

	const_iterator cbegin() const noexcept { return get_object().cbegin(); }

	iterator end() noexcept { return get_object().end(); }

	const_iterator end() const noexcept { return get_object().end(); }

	const_iterator cend() const noexcept { return get_object().cend(); }

	Json(Variant v) : m_value(std::move(v)) { }

	bool is_null() const { return m_value.empty(); }

	bool is_boolean() const { return check_type<bool>(m_value); }

	bool is_integer() const { return check_type<intptr_t>(m_value); }

	bool is_float() const { return check_type<double>(m_value); }

	bool is_number() const { return m_value.is_number(); }

	bool is_string() const { return check_type<String>(m_value); }

	bool is_array() const { return check_type<List>(m_value); }

	bool is_object() const { return check_type<Table>(m_value); }

	bool get_boolean() const { return cast<bool>(m_value); }

	intptr_t get_integer() const { return cast<intptr_t>(m_value); }

	double get_float() const { return cast<double>(m_value); }

	double get_number() const { return m_value.get_number(); }

	const String &get_string() const { return cast<String>(m_value); }

	const Json::Array &get_array() const { return cast<List>(m_value).items(); }

	const Json::Object &get_object() const { return cast<Table>(m_value).data(); }

	bool get_raw_boolean() const { return raw_cast<bool>(m_value); }

	intptr_t get_raw_integer() const { return raw_cast<intptr_t>(m_value); }

	double get_raw_float() const { return raw_cast<double>(m_value); }

	const String &get_raw_string() const { return raw_cast<String>(m_value); }

	const Json::Array &get_raw_array() const { return raw_cast<List>(m_value).items(); }

	const Json::Object &get_raw_object() const { return raw_cast<Table>(m_value).data(); }

	bool contains(const String &key) const { return get_object().contains(key); }

	iterator find(const String &key) { return get_raw_object().find(key); }

	const_iterator find(const String &key) const { return get_raw_object().find(key); }

private:

	Variant m_value;
};


} // namespace phonometrica

#endif // PHONOMETRICA_JSON_HPP
