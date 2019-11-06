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
 * Created: 08/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: display a list of properties in the property editor.                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROPERTY_TABLE_HPP
#define PHONOMETRICA_PROPERTY_TABLE_HPP

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <phon/application/property.hpp>

namespace phonometrica {


class PropertyTable : public QTableWidget
{
    Q_OBJECT

public:

    explicit PropertyTable(const std::set<Property> &properties, QWidget *parent = NULL);

    QString category(int row) const;

    QString value(int row) const;

    void resetProperties(const std::set<Property> &properties);

signals:

public slots:

private:

    void addProperty(const Property &tag);
};

} // namespace phonometrica

#endif // PHONOMETRICA_PROPERTY_TABLE_HPP
