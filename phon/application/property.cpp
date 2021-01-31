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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/property.hpp>
#include <phon/error.hpp>
#include "property.hpp"


namespace phonometrica {

std::set<Property> Property::known_properties;
std::set<String> Property::known_categories;
std::unordered_map<String, const std::type_info*> Property::the_property_types;

Property::Property(String category, std::any value) :
		impl(std::move(category), std::move(value))
{
	known_properties.insert(*this);
	auto result = known_categories.insert(this->category());

	// If insertion was successful, we have a new category, so we register its type
	if (result.second)
	{
		the_property_types[this->category()] = &this->type();
	}
	// We don't need to check for inconsistent types since operator<() will have thrown when trying to insert
	// the property into known_properties.
}

void Property::swap(Property &other) noexcept
{
	impl.swap(other.impl);
}

bool Property::operator<(const Property &other) const
{
	// Order by category, then by value
	auto cat1 = category();
	auto cat2 = other.category();
	int compare_categories = cat1.compare(cat2);

	if (compare_categories < 0) {
		return true;
	}
	else if (compare_categories > 0) {
		return false;
	}

	auto &type = impl->value.type();

	if (type != other.impl->value.type()) {
		throw error("Inconsistent type for property \"%\": "
			  "you probably created a property whose type differs from previous properties in the same category", cat1);
	}

	if (type == typeid(String))
	{
		auto &s1 = this->text_value();
		auto &s2 = other.text_value();

		return (s1 < s2);
	}
	else if (type == typeid(double))
	{
		auto n1 = std::any_cast<double>(impl->value);
		auto n2 = std::any_cast<double>(other.impl->value);

		return (n1 < n2);
	}
	else if (type == typeid(bool))
	{
		auto b1 = std::any_cast<bool>(impl->value);
		auto b2 = std::any_cast<bool>(other.impl->value);

		return (b1 < b2);
	}
	else
	{
		throw error("Internal error: invalid property value type");
	}
}

bool Property::operator==(const Property &other) const
{
	if (impl == other.impl) {
		return true;
	}

	// We often compare values of the same category, and it is rather unlikely that values across categories are
	// identical, so we compare values first, then categories.
	auto &type = impl->value.type();
	bool equal;

	if (type != other.impl->value.type()) {
		return false;
	}

	if (type == typeid(String))
	{
		auto &s1 = this->text_value();
		auto &s2 = other.text_value();

		equal = (s1 == s2);
	}
	else if (type == typeid(double))
	{
		auto n1 = this->numeric_value();
		auto n2 = other.numeric_value();

		equal = (n1 == n2);
	}
	else if (type == typeid(bool))
	{
		auto b1 = this->boolean_value();
		auto b2 = other.boolean_value();

		equal = (b1 == b2);
	}
	else
	{
		throw error("Internal error: invalid property value type");
	}

	return equal && (this->category() == other.category());
}

bool Property::operator!=(const Property &other) const
{
	return !(*this == other);
}

String Property::category() const
{
	return impl->category;
}

const std::set<String> & Property::get_categories()
{
	return known_categories;
}

std::set<String> Property::get_categories_by_type(const std::type_info &type)
{
	std::set<String> categories;

	for (auto &p : known_properties)
	{
		if (p.type() == type) {
			categories.insert(p.category());
		}
	}

	return categories;
}

const std::type_info &Property::type() const
{
	return impl->value.type();
}

Property &Property::operator=(const Property &other) noexcept
{
	Property tmp(other);
	this->swap(tmp);

	return *this;
}

Property &Property::operator=(Property &&other) noexcept
{
	this->swap(other);
	return *this;
}

void Property::remove(const Property &p)
{
	known_properties.erase(p);
}

String Property::to_string() const
{
	String result(impl->category);
	result.append(" : ");
	result.append(this->value());

	return result;
}

String Property::value() const
{
	auto &type = impl->value.type();

	if (type == typeid(String))
	{
		return text_value();
	}
	else if (type == typeid(double))
	{
		double n = numeric_value();
		auto i = intptr_t(n);

		if (double(i) == n)
			return String::convert(i);
		else
			return String::convert(n);
	}
	else if (type == typeid(bool))
	{
		return boolean_value() ? true_string() : false_string();
	}
	else
	{
		throw error("Internal error: invalid property value type");
	}
}

std::set<String> Property::get_values(const String &category)
{
	std::set<String> result;

	// The category may not exist if we are creating a new category in the information panel.
	if (!has_category(category)) {
		return result;
	}
	else if (is_boolean(category))
	{
		result.insert(true_string());
		result.insert(false_string());
	}
	else
	{
		for (auto &p : known_properties)
		{
			if (p.category() == category)
			{
				result.insert(p.value());
			}
		}
	}

	return result;
}

bool Property::boolean_value() const
{
	return std::any_cast<bool>(impl->value);
}

double Property::numeric_value() const
{
	return std::any_cast<double>(impl->value);
}

const String &Property::text_value() const
{
	return std::any_cast<const String &>(impl->value);
}

bool Property::is_boolean() const
{
	return this->type() == typeid(bool);
}

bool Property::is_numeric() const
{
	return this->type() == typeid(double);
}

bool Property::is_text() const
{
	return this->type() == typeid(String);
}

const std::type_info &Property::get_type(const String &category)
{
	auto it = the_property_types.find(category);
	assert(it != the_property_types.end());

	return *it->second;
}

String Property::false_string()
{
    static String f("false");
    return f;
}

String Property::true_string()
{
    static String t("true");
    return t;
}

String Property::undefined_string()
{
	static String u("undefined");
	return u;
}

Property Property::from_string(const String &category, const String &value)
{
    if (value == true_string())
    	return Property(category, true);
    if (value == false_string())
    	return Property(category, false);

    bool ok;
    auto num = value.to_float(&ok);
    if (ok) return Property(category, num);

    return Property(category, value);
}

bool Property::is_boolean(const String &category)
{
	return get_type(category) == typeid(bool);
}

bool Property::is_numeric(const String &category)
{
	return get_type(category) == typeid(double);
}

bool Property::is_text(const String &category)
{
	return get_type(category) == typeid(String);
}

const char *Property::xml_type_name() const
{
	if (type() == typeid(bool))
		return "boolean";
	if (type() == typeid(double))
		return "numeric";

	return "text";
}

const std::type_info &Property::parse_xml_type_name(std::string_view name)
{
	if (name == "text")
		return typeid(String);
	if (name == "numeric")
		return typeid(double);
	if (name == "boolean")
		return typeid(bool);

	throw error("Invalid property type name \"%\"", name);
}

Property Property::parse(const String &type, const String &category, const String &value)
{
	if (type == "Text")
	{
		return Property(category, value);
	}
	else if (type == "Number")
	{
		bool ok;
		double num = value.to_float(&ok);

		if (!ok)
		{
			throw error("Invalid numeric value for property: %", value);
		}
		return Property(category, num);
	}
	else if (type == "Boolean")
	{
		if (value == "1" || value == "true") {
			return Property(category, true);
		}
		else if (value == "" || value == "0" || value == "false") {
			return Property(category, false);
		}

		throw error("Invalid Boolean value for property: %", value);
	}

	throw error("Invalid property type: %", type);
}

String Property::type_name() const
{
	if (type() == typeid(bool))
		return "Boolean";
	if (type() == typeid(double))
		return "Number";

	return "Text";
}

bool Property::has_category(const String &category)
{
	return known_categories.find(category) != known_categories.end();
}

} // namespace phonometrica