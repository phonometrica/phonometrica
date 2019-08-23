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
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: abstract base for all view types displayed in the viewer.                                                 *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef VIEW_HPP
#define VIEW_HPP

#include <QWidget>
#include <phon/string.hpp>

namespace phonometrica {


class View : public QWidget
{
    Q_OBJECT

public:
    explicit View(QWidget *parent = nullptr);

    virtual void save() = 0;

    virtual void makeFocused() { }

    virtual void post_initialize() { }

signals:

    void modified();

    void saved();

public slots:
};

} // phonometrica

#endif // VIEW_HPP
