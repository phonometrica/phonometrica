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
 * Created: 17/09/2019                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/application/protocol.hpp>

namespace phonometrica {

Protocol::Protocol(Runtime &rt, const String &path) :
	runtime(rt), m_path(path)
{
	parse();
}

String Protocol::get_field_name(intptr_t i) const
{
	return m_fields[i].name;
}

void Protocol::parse()
{
	auto result = runtime.do_file(m_path);
	if (!check_type<Table>(result)) {
		throw error("File % must contain a table", m_path);
	}
	auto json = std::move(raw_cast<Table>(result).data());
	Variant var;

	auto it = json.find("type");
	if (it == json.end()) {
		throw error("Protocol has no \"type\" key");
	}

	auto type = cast<String>(it->second);
	if (type != "coding_protocol") {
		throw error("Invalid type in protocol: \"%\"", type);
	}

	it = json.find("name");
	if (it == json.end()) {
		throw error("Protocol has no \"name\" key");
	}
	m_name = cast<String>(it->second);

	it = json.find("version");
	if (it != json.end()) {
		m_version = cast<String>(it->second);
	}

	it = json.find("field_separator");
	if (it != json.end()) {
		m_separator = cast<String>(it->second);
	}

	it = json.find("layer_index");
	if (it != json.end()) {
		m_layer_index = int(cast<intptr_t>(it->second));
	}

	it = json.find("layer_name");
	if (it != json.end()) {
		m_layer_pattern = cast<String>(it->second);
	}

	it = json.find("layer_field");
	if (it != json.end()) {
		m_layer_field = int(cast<intptr_t>(it->second));
	}

	// Don't use layer index if we have a valid name or if we use a layer field.
	if (!m_layer_pattern.empty() || m_layer_field != 0) m_layer_index = -1;
	// Sanity check.
	if (m_layer_pattern.empty() && layer_index() < 0) {
		throw error("Invalid negative layer index");
	}

	it = json.find("case_sensitive");
	if (it != json.end()) {
		m_case_sensitive = cast<bool>(it->second);
	}

	it = json.find("fields_per_row");
	if (it != json.end()) {
		m_fields_per_row = int(cast<intptr_t>(it->second));
	}

	it = json.find("fields");
	if (it == json.end()) {
		throw error("Protocol has no fields");
	}


	if (!check_type<List>(it->second)) {
		throw error("\"fields\" must be a list");
	}
	auto fields = std::move(raw_cast<List>(it->second).items());

	int f = 0; // for error reporting
	for (auto &field_var : fields)
	{
		f++;
		SearchField search_field;

		if (!check_type<Table>(field_var)) {
			throw error("Field % is not a table", f);
		}
		auto field = std::move(raw_cast<Table>(field_var).data());

		it = field.find("name"); // can be anonymous
		if (it != field.end()) {
			search_field.name = cast<String>(it->second);
		}

		it = field.find("match_all"); // can be empty
		if (it == field.end()) {
			throw error("Field % has no \"match_all\" key", f);
		}
		search_field.match_all = cast<String>(it->second);

		it = field.find("layer_pattern");
		if (it != field.end()) {
			if (f != m_layer_field) {
				throw error("Key \"layer_pattern\" can only be found in layer-selecting field");
			}
			search_field.layer_pattern = cast<String>(it->second);
		}

		it = field.find("values");
		if (it == field.end()) {
			throw error("Field % has no values", f);
		}

		if (!check_type<List>(it->second)) {
			throw error("\"values\" must be a list in field %", f);
		}
		auto values = std::move(raw_cast<List>(it->second).items());

		int g = 0; // for error reporting
		for (auto &value_var : values)
		{
			g++;
			SearchValue search_value;

			if (!check_type<Table>(value_var)) {
				throw error("Value % must be a ble in field %", g, f);
			}
			auto value = raw_cast<Table>(value_var).data();

			it = value.find("match");
			if (it == value.end()) {
				throw error("Value % has no \"match\" key in field %", g, f);
			}
			search_value.match = cast<String>(it->second);

			it = value.find("text");
			if (it == value.end()) {
				throw error("Value % has no \"text\" key in field %", g, f);
			}
			search_value.text = cast<String>(it->second);

			if (f == m_layer_field)
			{
				it = value.find("layer_name");
				if (it == value.end()) {
					throw error("Value % has no \"layer_name\" key in field %", g, f);
				}
				search_value.layer_name = cast<String>(it->second);
			}

			it = value.find("choices");
			if (it != value.end())
			{
				String choices = cast<String>(it->second);
				auto it2 = value.find("display");
				if (it2 == value.end()) {
					throw error("Value % in field % has choices but no \"display\" key", g, f);
				}
				String display = cast<String>(it->second);

				auto choice_items = choices.split("|");
				auto display_items = display.split("|");

				if (choice_items.size() != display_items.size()) {
					throw error("Inconsistent number of choice and display items in value % in field %", g, f);
				}

				for (intptr_t i = 1; i <= choice_items.size(); i++)
				{
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
} // namespace phonometrica
