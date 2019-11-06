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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: raw text file.                                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DOCUMENT_HPP
#define PHONOMETRICA_DOCUMENT_HPP

#include <phon/application/vfs.hpp>

namespace phonometrica {

class Document final : public VFile
{
public:

	explicit Document(VFolder *parent, String path = String());

	const char *class_name() const override;

	bool is_document() const override;

private:

	void load() override;

	void write() override;

	std::vector<String> m_lines;

};


} // namespace phonometrica

#endif // PHONOMETRICA_DOCUMENT_HPP
