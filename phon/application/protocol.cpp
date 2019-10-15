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
			if (f != m_layer_selecting_field) {
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

			if (f == m_layer_selecting_field)
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
