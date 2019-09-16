/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
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
#include <phon/utils/refcount.hpp>
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

	Handle<Data> impl;
};


} // namespace phonometrica

#endif // PHONOMETRICA_PROPERTY_HPP
