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
 * Created: 21/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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

