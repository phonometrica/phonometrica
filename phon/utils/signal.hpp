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
 * Created: 25/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Signal class similar to Qt's signal. We use this mainly to avoid multiple inheritance when we would need   *
 * to inherit from QObject to get access to Qt's signal/slot mechanism.                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SIGNAL_HPP
#define PHONOMETRICA_SIGNAL_HPP

#include <phon/third_party/sigslot/signal.hpp>

namespace phonometrica {

template <typename... T>
using Signal = sigslot::signal<T...>;

} // namespace phonometrica

#endif // PHONOMETRICA_SIGNAL_HPP
