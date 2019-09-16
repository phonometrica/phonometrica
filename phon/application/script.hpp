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
 * Purpose: Script file.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
