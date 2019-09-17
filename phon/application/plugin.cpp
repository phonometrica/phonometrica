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

	auto protocol_directory = get_protocol_directory();
	if (filesystem::exists(protocol_directory))
	{
		for (auto &name : filesystem::list_directory(protocol_directory))
		{
			auto path = filesystem::join(protocol_directory, name);
			m_protocols.append(std::make_shared<Protocol>(runtime, path));
		}
	}
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

	auto name_iter = js.find("name");
	if (name_iter == js.end()) {
		throw error("Plugin % has no \"name\" key in description.json", label());
	}
	String name = name_iter->get<std::string>();

	String version;
	auto version_iter = js.find("version");
	if (version_iter != js.end()) {
		version = version_iter->get<std::string>();
	}

	auto actions_iter = js.find("actions");
	if (actions_iter != js.end())
	{
		// Fields "name" and "target" are compulsory, and represent the action's name and the target script, respectively.
		// Additionally, the user may specify a "shortcut".
		for (auto action : *actions_iter)
		{
			if (!action.is_object()) {
				throw error("Error in plugin %: actions in description.json must be objects", label());
			}

			auto iter = action.find("name");
			if (iter == action.end()) {
				throw error("Error in plugin %: action in description.json has no \"name\" key", label());
			}
			String name = iter->get<std::string>();

			iter = action.find("target");
			if (iter == action.end()) {
				throw error("Error in plugin %: action in description.json has no \"target\" key", label());
			}
			String target = iter->get<std::string>();
			auto script = get_script(target);
			if (!filesystem::is_file(script)) {
				throw error("Error in plugin %: cannot find script \"%\"", label(), script);
			}
			String shortcut;
			iter = action.find("shortcut");
			if (iter != action.end()) {
				shortcut = iter->get<std::string>();
			}

			callback(name, script, shortcut);
		}
	}
}

String Plugin::label() const
{
	return filesystem::base_name(m_path);
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
	return filesystem::join(m_path, "Resources");
}

String Plugin::get_protocol(const String &name) const
{
	return filesystem::join(get_protocol_directory(), name);
}

} // namespace phonometrica
