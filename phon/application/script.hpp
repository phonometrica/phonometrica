/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Script file.                                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_SCRIPT_HPP
#define PHONOMETRICA_SCRIPT_HPP

#include <phon/application/vfs.hpp>

namespace phonometrica {

class Script final : public VFile
{
public:

	explicit Script(VFolder *parent, String path = String());

	bool is_script() const override;

	const char *class_name() const override;

	void from_xml(xml_node root, const String &project_dir);

	const String &content() const;

	void set_content(String value, bool mutate = true);

    // A script can only be modified in a script view. We don't update the script's content every time the text is
    // changed in the view. Instead, we inform the script that it has been modified with this method. When the view
    // is closed, the user will be asked whether they want to save the modifications or not. Modifications can also
    // be saved via the save button in the script view.
    void set_pending_modifications() { m_content_modified = true; }

private:

	void load() override;

	void write() override;

	String m_content;

};

using AutoScript = std::shared_ptr<Script>;

} // namespace phonometrica

#endif // PHONOMETRICA_SCRIPT_HPP
