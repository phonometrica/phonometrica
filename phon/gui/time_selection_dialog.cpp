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
 * Created: 21/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <phon/gui/time_selection_dialog.hpp>
#include "time_selection_dialog.hpp"


namespace phonometrica {

TimeSelectionDialog::TimeSelectionDialog(QWidget *parent, double *from, double *to) : QDialog(parent)
{
    this->from = from;
    this->to = to;
    this->setWindowTitle("Set selection times...");
    this->setMinimumWidth(300);
    from_edit = new QLineEdit;
    to_edit = new QLineEdit;
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    auto layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("From:"));
    layout->addWidget(from_edit);
    layout->addWidget(new QLabel("To:"));
    layout->addWidget(to_edit);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &TimeSelectionDialog::setValues);
    connect(buttons, &QDialogButtonBox::rejected, this, &TimeSelectionDialog::reject);
    setLayout(layout);
}

void TimeSelectionDialog::setValues()
{
    bool ok1, ok2;
    double t1 = from_edit->text().toDouble(&ok1);
    double t2 = to_edit->text().toDouble(&ok2);

    if (!ok1 || !ok2 || t1 < 0 || t2 <= 0)
    {
        QMessageBox dlg(QMessageBox::Critical, "Error", "Invalid time selection");
        dlg.exec();
        reject();
    }
    *from = t1;
    *to = t2;
    accept();
}

} // namespace phonometrica

