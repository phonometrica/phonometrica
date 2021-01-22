/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/document.hpp>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

Document::Document(VFolder *parent, String path) :
		VFile(parent, std::move(path))
{

}

const char *Document::class_name() const
{
	return "Document";
}

bool Document::is_document() const
{
	return true;
}

void Document::load()
{

}

void Document::write()
{

}

void Document::initialize(Runtime &rt)
{
	auto cls = rt.create_type<AutoDocument>("Document", rt.get_object_class());
	rt.add_global("Document", std::move(cls));
}
} // namespace phonometrica