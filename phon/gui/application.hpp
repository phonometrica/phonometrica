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
 * Created: 03/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: main application.                                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_APPLICATION_HPP
#define PHONOMETRICA_APPLICATION_HPP

#include <QApplication>

namespace phonometrica {

class Application final : public QApplication
{
    Q_OBJECT

public:

    Application(int &argc, char **argv) : QApplication(argc, argv) {}

    bool notify(QObject* receiver, QEvent* event) override;
};

} // namespace phonometrica

#endif // PHONOMETRICA_APPLICATION_HPP
