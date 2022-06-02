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
 * Purpose: a plugin is a dynamically loaded add-on that extends Phonometrica. Plugins are loaded in the Tools menu.   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PLUGIN_HPP
#define PHONOMETRICA_PLUGIN_HPP

#include <functional>
#include <phon/utils/any.hpp>
#include <phon/application/protocol.hpp>

namespace phonometrica {

class Plugin final
{
public:

	// Helper structure to add a script or a protocol to a the plugin's menu.
	// FIXME: this should be std::variant<String,AutoProtocol> but Apple has broken support for C++17 on macOS 10.13
	//  and I can't seem to get any variant alternative to work properly.
	using MenuEntry = std::any;

	// Callback to add entries to the plugin's menu.
	using Callback = std::function<void(String, MenuEntry)>;

	Plugin(Runtime &rt, String path, Callback menu_handle);

	~Plugin();

	String path() const { return m_path; }

	String label() const;

	String get_script_directory() const;

	String get_script(const String &name) const;

	String get_resource_directory() const;

	String get_documentation_directory() const;

	String get_documentation_page(const String &name) const;

	String get_resource(const String &name) const;

	String get_protocol_directory() const;

	String get_protocol(const String &name) const;

	bool has_entries() const;

	String description() const { return m_description; }

	String version() const { return m_version; }

	int menu_id() const { return m_menu_id; }

	void set_menu_id(int value) { m_menu_id = value; }

private:

	void parse_description(Callback &callback);

	void parse_protocols(Callback &callback);

	Runtime &runtime;

	String m_path;

	String m_label;

	String m_version;

	String m_description;

	Array<AutoProtocol> m_protocols;

	int m_menu_id = -1;

	bool has_scripts = false;
};

using AutoPlugin = std::shared_ptr<Plugin>;

} // namespace phonometrica

#endif // PHONOMETRICA_PLUGIN_HPP
