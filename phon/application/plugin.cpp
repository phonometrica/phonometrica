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
#include <phon/application/plugin.hpp>
#include <phon/utils/file_system.hpp>
#include <phon/third_party/json.hpp>

using namespace nlohmann;

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
	auto desc_path = filesystem::join(m_path, "description.json");

	if (!filesystem::exists(desc_path)) {
		throw error("File \"description.json\" does not exist in plugin %", label());
	}

	auto content = File::read_all(desc_path);
	auto js = json::parse(content);

	auto it = js.find("name");
	if (it == js.end()) {
		throw error("Plugin % has no \"name\" key in description.json", label());
	}
	m_label = it->get<std::string>();

	it = js.find("version");
	if (it != js.end()) {
		m_version = it->get<std::string>();
	}

	it = js.find("description");
	if (it != js.end()) {
		if (it->is_array())
		{
			for (auto line : *it)
			{
				m_description.append(line.get<std::string>());
			}
		}
		else
		{
			m_description = it->get<std::string>();
		}
	}

	it = js.find("actions");
	if (it != js.end())
	{
		// Fields "name" and "target" are compulsory, and represent the action's name and the target script, respectively.
		// Additionally, the user may specify a "shortcut".
		auto actions = *it;
		for (auto action : actions)
		{
			if (!action.is_object()) {
				throw error("Error in plugin %: actions in description.json must be objects", label());
			}

			it = action.find("name");
			if (it == action.end()) {
				throw error("Error in plugin %: action in description.json has no \"name\" key", label());
			}
			String name = it->get<std::string>();

			it = action.find("target");
			if (it == action.end()) {
				throw error("Error in plugin %: action in description.json has no \"target\" key", label());
			}
			String target = it->get<std::string>();
			auto script = get_script(target);
			if (!filesystem::is_file(script)) {
				throw error("Error in plugin %: cannot find script \"%\"", label(), script);
			}
			String shortcut;
			it = action.find("shortcut");
			if (it != action.end()) {
				shortcut = it->get<std::string>();
			}

			callback(name, script, shortcut);
			has_scripts = true;
		}
	}
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
					callback(String(), String(), String());
					has_separator = true;
				}
				callback(label, protocol, String());
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

} // namespace phonometrica
