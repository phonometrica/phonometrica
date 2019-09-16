/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 08/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/property_table.hpp>

#define CAT_COL 0
#define VAL_COL 1

namespace phonometrica {

PropertyTable::PropertyTable(const std::set<Property> &properties, QWidget *parent) :
    QTableWidget(0, 2, parent)
{
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->verticalHeader()->hide();
    this->setHorizontalHeaderItem(CAT_COL, new QTableWidgetItem(tr("Category")));
    this->setHorizontalHeaderItem(VAL_COL, new QTableWidgetItem(tr("Value")));

    for (auto &tag : properties)
        this->addProperty(tag);

    this->horizontalHeader()->setStretchLastSection(true);
}

void PropertyTable::resetProperties(const std::set<Property> &properties)
{
    while (rowCount() > 0)
        removeRow(0);

    for (auto &tag : properties)
        this->addProperty(tag);
}

void PropertyTable::addProperty(const Property &tag)
{
    int row = rowCount();

    QTableWidgetItem *cat = new QTableWidgetItem(tag.category());
    QTableWidgetItem *val = new QTableWidgetItem(tag.value());

    cat->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    val->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);

    this->insertRow(row);
    this->setItem(row, CAT_COL, cat);
    this->setItem(row, VAL_COL, val);

    verticalHeader()->resizeSection(row, 20);
}

QString PropertyTable::category(int row) const
{
    return item(row, CAT_COL)->text();
}

QString PropertyTable::value(int row) const
{
    return item(row, VAL_COL)->text();
}

} // namespace phonometrica
