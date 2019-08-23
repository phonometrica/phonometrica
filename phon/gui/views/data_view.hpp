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
 * Created: 14/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Display tabular data.                                                                                     *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_DATA_VIEW_HPP
#define PHONOMETRICA_DATA_VIEW_HPP

#include <QTableWidget>
#include <phon/gui/views/view.hpp>
#include <phon/application/dataset.hpp>

namespace phonometrica {


class DataView final : public View
{
    Q_OBJECT

public:

    DataView(QWidget *parent, std::shared_ptr<Dataset> data);



private:

    std::shared_ptr<Dataset> m_data;

    QTableWidget *m_table;

};

} // namespace phonometrica

#endif // PHONOMETRICA_DATA_VIEW_HPP
