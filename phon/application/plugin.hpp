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
	using Callback = std::function<void(String, MenuEntry, String)>;

	Plugin(Runtime &rt, String path, Callback menu_handle);

	~Plugin();

	String path() const { return m_path; }

	String label() const { return m_label; }

	String get_script_directory() const;

	String get_script(const String &name) const;

	String get_resource_directory() const;

	String get_resource(const String &name) const;

	String get_protocol_directory() const;

	String get_protocol(const String &name) const;

	bool has_entries() const;

	String description() const { return m_description; }

	String version() const { return m_version; }

private:

	void parse_description(Callback &callback);

	void parse_protocols(Callback &callback);

	Runtime &runtime;

	String m_path;

	String m_label;

	String m_version;

	String m_description;

	Array<AutoProtocol> m_protocols;

	bool has_scripts = false;
};

using AutoPlugin = std::shared_ptr<Plugin>;

} // namespace phonometrica

#endif // PHONOMETRICA_PLUGIN_HPP
