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
 * Created: 08/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

Concordance::Concordance(intptr_t target_count, VFolder *parent, const String &path) :
	Dataset(parent, path)
{
	m_target_count = target_count;
}

const char *Concordance::class_name() const
{
	return "Concordance";
}

bool Concordance::empty() const
{
	return m_matches.empty();
}

String Concordance::get_header(intptr_t j) const
{
	return String();
}

String Concordance::get_cell(intptr_t i, intptr_t j) const
{
	return String();
}

intptr_t Concordance::row_count() const
{
	return m_matches.size();
}

intptr_t Concordance::column_count() const
{
	return 0;
}
} // namespace phonometrica
