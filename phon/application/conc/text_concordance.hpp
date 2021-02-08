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
 * Purpose: Represents the result of a text query.                                                                     *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_CONCORDANCE_HPP
#define PHONOMETRICA_TEXT_CONCORDANCE_HPP

#include <phon/application/conc/concordance.hpp>
#include <phon/application/conc/match.hpp>

namespace phonometrica {

class TextConcordance : public Concordance
{
public:

	TextConcordance(VFolder *parent, const String &path = String());

protected:

	Array<std::unique_ptr<Match>> m_matches;
};

} // namespace phonometrica



#endif // PHONOMETRICA_TEXT_CONCORDANCE_HPP
