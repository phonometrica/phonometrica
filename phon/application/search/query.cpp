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
 * Created: 07/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/application/search/query.hpp>

namespace phonometrica {

void Query::execute()
{
	filter_metadata();
	filter_data();
}

void Query::filter_metadata()
{
	emit debug("METADATA:");
	for (auto &node : metadata)
	{
		annotations = node->filter(annotations);
	}

	for (auto &annot : annotations)
	{
		emit debug(annot->path());
	}
}

void Query::filter_data()
{
	emit debug("CONSTRAINTS:");
	emit debug(search_tree->to_string());
	emit done();
}
} // namespace phonometrica