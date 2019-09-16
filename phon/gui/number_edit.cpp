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
 * Created: 05/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <phon/gui/number_edit.hpp>

namespace phonometrica {

NumberEdit::NumberEdit(String desc) : QWidget()
{
	auto layout = new QVBoxLayout;
	this->desc = std::move(desc);
	op_box = new QComboBox;
	num_edit = new LineEdit(tr("numeric value"));

	QStringList ops;
	ops << "equal" << "not equal" << "less than" << "less than or equal" << "greater than" << "greater than or equal";

	for (auto &op: ops) {
		op_box->addItem(op);
	}

	layout->addWidget(new QLabel("<b>" + QString(this->desc) + "</b>"));
	layout->addWidget(op_box);
	layout->addWidget(num_edit);
	layout->addStretch(1);
	setLayout(layout);
	op_box->setFocusProxy(num_edit);
}

String NumberEdit::description() const
{
	return desc;
}

std::function<bool(double)> NumberEdit::functor() const
{
	double val = this->value();

	switch (op_box->currentIndex())
	{
		case 0:
			return [=](double x) { return x == val; };
		case 1:
			return [=](double x) { return x != val; };
		case 2:
			return [=](double x) { return x < val; };
		case 3:
			return [=](double x) { return x <= val; };
		case 4:
			return [=](double x) { return x > val; };
		default:
			return [=](double x) { return x >= val; };
	}
}

double NumberEdit::value() const
{
	return num_edit->text().toDouble();
}

bool NumberEdit::hasValue() const
{
	return !num_edit->text().isEmpty();
}

} // namespace phonometrica