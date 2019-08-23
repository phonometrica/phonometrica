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
 * Purpose: Select times in a speeech view.                                                                           *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_TIME_SELECTION_DIALOG_HPP
#define PHONOMETRICA_TIME_SELECTION_DIALOG_HPP

#include <QDialog>
#include <QLineEdit>

namespace phonometrica {

class TimeSelectionDialog final : public QDialog
{
    Q_OBJECT

public:

    TimeSelectionDialog(QWidget *parent, double *from, double *to);

private slots:

    void setValues();

private:

    QLineEdit *from_edit, *to_edit;
    double *from, *to;
};


} // namespace phonometrica

#endif // PHONOMETRICA_TIME_SELECTION_DIALOG_HPP
