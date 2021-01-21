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

#include <phon/application/plugin.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Plugin::Plugin(Runtime &rt, String path, Callback menu_handle) :
	runtime(rt), m_path(std::move(path))
{
	parse_description(menu_handle);

	// If there's an initialization script, run it.
	auto script = filesystem::join(m_path, "initialize.phon");
	if (filesystem::exists(script))
	{
		runtime.do_file(script);
	}

	parse_protocols(menu_handle);
}

Plugin::~Plugin()
{
	// Give the user a chance to do some cleanup.
	auto script = filesystem::join(m_path, "finalize.phon");
	if (filesystem::exists(script))
	{
		runtime.do_file(script);
	}
}

void Plugin::parse_description(Callback &callback)
{
	auto desc_path = filesystem::join(m_path, "description.phon");
	auto old_desc_path = filesystem::join(m_path, "description.json");

	if (!filesystem::exists(desc_path) && filesystem::exists(old_desc_path)) {
		auto msg = utils::format("Plugin \"%\" is not compatible with this version of Phonometrica."
						   "\"description.json\" should be renamed to \"description.phon\"", label());
		throw error(msg);
	}


	if (!filesystem::exists(desc_path)) {
		throw error("File \"description.phon\" does not exist in plugin %", label());
	}
	
	auto result = runtime.do_file(desc_path);
	if (!check_type<Table>(result)) {
		throw error("File \"description.phon\" must contain a table");
	}
	auto table = std::move(raw_cast<Table>(result).data());

	auto it = table.find("name");
	if (it == table.end()) {
		throw error("Plugin % has no \"name\" key in description.phon", label());
	}
	m_label = cast<String>(it->second);

	it = table.find("version");
	if (it != table.end()) {
		m_version = cast<String>(it->second);
	}

	it = table.find("description");
	if (it != table.end())
	{
		if (check_type<List>(it->second))
		{
			auto lines = std::move(raw_cast<List>(it->second).items());
			for (auto &line : lines)
			{
				m_description.append(cast<String>(line));
			}
		}
		else
		{
			m_description = cast<String>(it->second);
		}
	}

	it = table.find("actions");
	if (it != table.end())
	{
		// Fields "name" and "target" are compulsory, and represent the action's name and the target script, respectively.
		// Additionally, the user may specify a "shortcut".
		if (!check_type<List>(it->second)) {
			throw error("Error in plugin %: actions must be a list", label());
		}
		auto actions = std::move(raw_cast<List>(it->second).items());
		for (auto &action_var : actions)
		{
			if (!check_type<Table>(action_var)) {
				throw error("Error in plugin %: actions in description.phon must be tables", label());
			}
			auto action = std::move(raw_cast<Table>(action_var).data());

			it = action.find("name");
			if (it == action.end()) {
				throw error("Error in plugin %: action in description.phon has no \"name\" key", label());
			}
			String name = cast<String>(it->second);

			it = action.find("target");
			if (it == action.end()) {
				throw error("Error in plugin %: action in description.phon has no \"target\" key", label());
			}
			String target = cast<String>(it->second);
			auto file = target.ends_with(".html") ? get_documentation_page(target) : get_script(target);
			if (!filesystem::is_file(file)) {
				throw error("Error in plugin %: cannot find file \"%\"", label(), file);
			}

			callback(name, file);
			has_scripts = true;
		}
	}
}

String Plugin::get_documentation_directory() const
{
	return filesystem::join(m_path, "Documentation");
}

String Plugin::get_script_directory() const
{
	return filesystem::join(m_path, "Scripts");
}

String Plugin::get_script(const String &name) const
{
	return filesystem::join(get_script_directory(), name);
}

String Plugin::get_resource_directory() const
{
	return filesystem::join(m_path, "Resources");
}

String Plugin::get_resource(const String &name) const
{
	return filesystem::join(get_resource_directory(), name);
}

String Plugin::get_protocol_directory() const
{
	return filesystem::join(m_path, "Protocols");
}

String Plugin::get_protocol(const String &name) const
{
	return filesystem::join(get_protocol_directory(), name);
}

void Plugin::parse_protocols(Plugin::Callback &callback)
{
	auto protocol_directory = get_protocol_directory();
	bool has_separator = false;

	if (filesystem::exists(protocol_directory))
	{
		for (auto &name : filesystem::list_directory(protocol_directory))
		{
			auto path = filesystem::join(protocol_directory, name);

			try
			{
				auto protocol = std::make_shared<Protocol>(runtime, path);
				String label = protocol->name();

				if (!has_separator)
				{
					callback(String(), String());
					has_separator = true;
				}
				callback(label, protocol);
				m_protocols.append(std::move(protocol));
			}
			catch (std::exception &e)
			{
				auto msg = utils::format("Error in protocol %: %", path, e.what());
				throw error(msg);
			}
		}
	}
}

bool Plugin::has_entries() const
{
	return has_scripts || !m_protocols.empty();
}

String Plugin::get_documentation_page(const String &name) const
{
	return filesystem::join(get_documentation_directory(), name);
}

String Plugin::label() const
{
	return m_label.empty() ? filesystem::base_name(m_path) : m_label;
}

} // namespace phonometrica
