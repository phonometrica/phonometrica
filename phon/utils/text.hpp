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
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: text manipulation routines.                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_HPP
#define PHONOMETRICA_TEXT_HPP

#include <phon/string.hpp>

namespace phonometrica { namespace utils {

Array<Array<String>> parse_csv(const String &path, std::string_view splitter = ",");

String matrix_to_csv(const Array<double> &matrix, std::string_view sep = ",");

}} // namespace phonometrica::utils

#endif // PHONOMETRICA_TEXT_HPP
