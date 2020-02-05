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
 * Created: 17/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/application/protocol.hpp>
#include <phon/third_party/json.hpp>

using namespace nlohmann;

namespace phonometrica {

Protocol::Protocol(Runtime &rt, const String &path) :
	runtime(rt), m_path(path)
{
	auto content = File::read_all(path);
	auto js = json::parse(content.data());
	json::iterator it;

	it = js.find("type");
	if (it == js.end()) {
		throw error("Protocol has no \"type\" key");
	}

	auto type = it->get<std::string>();
	if (type != "coding_protocol") {
		throw error("Invalid type in protocol: \"%\"", type);
	}

	it = js.find("name");
	if (it == js.end()) {
		throw error("Protocol has no \"name\" key");
	}
	m_name = it->get<std::string>();

	it = js.find("version");
	if (it != js.end()) {
		m_version = it->get<std::string>();
	}

	it = js.find("field_separator");
	if (it != js.end()) {
		m_separator = it->get<std::string>();
	}

	it = js.find("layer_index");
	if (it != js.end()) {
		m_layer_index = int(it->get<int64_t>());
	}

	it = js.find("layer_name");
	if (it != js.end()) {
		m_layer_pattern = it->get<std::string>();
	}

	it = js.find("layer_field");
	if (it != js.end()) {
		m_layer_field = int(it->get<int64_t>());
	}

	// Don't use layer index if we have a valid name or if we use a layer field.
	if (!m_layer_pattern.empty() || m_layer_field != 0) m_layer_index = -1;
	// Sanity check.
	if (m_layer_pattern.empty() && layer_index() < 0) {
		throw error("Invalid negative layer index");
	}

	it = js.find("case_sensitive");
	if (it != js.end()) {
		m_case_sensitive = it->get<bool>();
	}

	it = js.find("fields_per_row");
	if (it != js.end()) {
		m_fields_per_row = int(it->get<int64_t>());
	}

	it = js.find("fields");
	if (it == js.end()) {
		throw error("Protocol has no fields");
	}

	json fields = *it;
	if (!fields.is_array()) {
		throw error("\"fields\" must be an array");
	}
	int f = 0; // for error reporting
	for (auto field : fields)
	{
		f++;
		SearchField search_field;

		if (!field.is_object()) {
			throw error("Field % is not an object", f);
		}

		it = field.find("name"); // can be anonymous
		if (it != field.end()) {
			search_field.name = it->get<std::string>();
		}

		it = field.find("match_all"); // can be empty
		if (it == field.end()) {
			throw error("Field % has no \"match_all\" key", f);
		}
		search_field.match_all = it->get<std::string>();


		it = field.find("layer_pattern");
		if (it != field.end()) {
			if (f != m_layer_field) {
				throw error("Key \"layer_pattern\" can only be found in layer-selecting field");
			}
			search_field.layer_pattern = it->get<std::string>();
		}

		it = field.find("values");
		if (it == field.end()) {
			throw error("Field % has no values", f);
		}
		json values = *it;
		if (!values.is_array()) {
			throw error("\"values\" must be an array in field %", f);
		}

		for (auto value : values)
		{
			SearchValue search_value;

			it = value.find("match");
			if (it == value.end()) {
				throw error("Value has no \"match\" key in field %", f);
			}
			search_value.match = it->get<std::string>();

			it = value.find("text");
			if (it == value.end()) {
				throw error("Value has no \"text\" key in field %", f);
			}
			search_value.text = it->get<std::string>();

			if (f == m_layer_field)
			{
				it = value.find("layer_name");
				if (it == value.end()) {
					throw error("Value has no \"layer_name\" key in field %", f);
				}
				search_value.layer_name = it->get<std::string>();
			}

			it = value.find("choices");
			if (it != value.end())
			{
				String choices = it->get<std::string>();
				auto it2 = value.find("display");
				if (it2 == value.end()) {
					throw error("Field % has choices but no \"display\" key", f);
				}
				String display = it->get<std::string>();

				auto choice_items = choices.split("|");
				auto display_items = display.split("|");

				if (choice_items.size() != display_items.size()) {
					throw error("Inconsistent number of choice and display items in field %", f);
				}

				for (intptr_t i = 1; i <= choice_items.size(); i++) {
					String m = choice_items[i];
					String t = display_items[i];
					search_value.choices.append({m, t});
				}
			}

			search_field.values.append(std::move(search_value));
		}

		m_fields.append(std::move(search_field));
	}
}

String Protocol::get_field_name(intptr_t i) const
{
	return m_fields[i].name;
}
} // namespace phonometrica
