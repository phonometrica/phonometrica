/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: a property is a key/value pair which can be used to add metadata to VFile subclasses. The value can be a   *
 * Boolean, an number or a string.                                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROPERTY_HPP
#define PHONOMETRICA_PROPERTY_HPP

#include <string>
#include <set>
#include <unordered_map>
#include <memory>
#include <phon/string.hpp>
#include <phon/utils/ref_count.hpp>
#include <phon/utils/any.hpp>

namespace phonometrica {

class Property final
{
public:

	// Invalid property
	Property() = default;

	Property(String category, std::any value);

	Property(const Property &) = default;

	Property(Property &&) noexcept = default;

	~Property() = default;

	static Property from_string(const String &category, const String &value);

	void swap(Property &other) noexcept;

	Property &operator=(const Property &other) noexcept;
	Property &operator=(Property &&other) noexcept;

	bool operator==(const Property &other) const;
	bool operator!=(const Property &other) const;
	bool operator<(const Property &other) const;

	String category() const;

	String value() const;

	bool boolean_value() const;

	double numeric_value() const;

	const String &text_value() const;

	static const std::set<String> & get_categories();

	static std::set<String> get_values(const String &category);

	static const std::type_info &get_type(const String &category);

	static bool is_boolean(const String &category);

	static bool is_numeric(const String &category);

	static bool is_text(const String &category);

	const std::type_info &type() const;

	const char *type_name() const;

	static const std::type_info &parse_type_name(std::string_view name);

	static void remove(const Property &p);

	String to_string() const;

	bool is_boolean() const;

	bool is_numeric() const;

	bool is_text() const;

	static String false_string();

	static String true_string();

	static String undefined_string();

	bool valid() const { return bool(impl); }

private:

	static std::set<Property> the_known_properties;
	static std::set<String> the_known_categories;
	static std::unordered_map<String, const std::type_info*> the_property_types;

	struct Data : public Countable<Data>
	{
		Data(String category, std::any value) :
				category(std::move(category)), value(std::move(value))
		{ }

		~Data() = default;

		String category;
		std::any value;
	};

	IntrusivePtr<Data> impl;
};


} // namespace phonometrica

#endif // PHONOMETRICA_PROPERTY_HPP
