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
 * Purpose: monospace font.                                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FONT_HPP
#define PHONOMETRICA_FONT_HPP

#include <QFont>

namespace phonometrica {

static inline
QFont get_monospace_font()
{
    QFont font;
    font.setFixedPitch(true);
#if PHON_WINDOWS
    font.setPointSize(10);
    font.setFamily("Noto Sans Mono");
#elif PHON_MACOS
    font.setPointSize(14);
    font.setFamily("Consolas");
#else
    font.setPointSize(11);
    font.setFamily("Noto Sans Mono");
#endif

    return font;
}

} // phonometrica

#endif // PHONOMETRICA_FONT_HPP
