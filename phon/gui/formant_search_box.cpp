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
 * Created: 31/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <phon/application/settings.hpp>
#include <phon/gui/formant_search_box.hpp>

namespace phonometrica {

FormantQuerySettings::FormantQuerySettings(double win_size, int nformant, double max_freq, int lpc_order, bool erb,
                                           bool bark) : Query::Settings(Query::Type::Formants)
{
	this->win_size = win_size;
	this->nformant = nformant;
	this->max_freq = max_freq;
	this->lpc_order = lpc_order;
	this->parametric = false;
	this->erb = erb;
	this->bark = bark;
}

FormantQuerySettings::FormantQuerySettings(double win_size, int nformant, double max_freq1, double max_freq2, double step,
                                           int lpc_order1, int lpc_order2, bool erb, bool bark) : Query::Settings(Query::Type::Formants)
{
	this->win_size = win_size;
	this->nformant = nformant;
	this->max_freq1 = max_freq1;
	this->max_freq2 = max_freq2;
	this->step = step;
	this->lpc_order1 = lpc_order1;
	this->lpc_order2 = lpc_order2;
	this->parametric = true;
	this->erb = erb;
	this->bark = bark;
}

FormantSearchBox::FormantSearchBox(QWidget *parent) :
	DefaultSearchBox(parent, 0)
{
	this->setTitle(tr("Formant analysis"));
}

AutoSearchNode FormantSearchBox::buildSearchTree()
{
	return DefaultSearchBox::buildSearchTree();
}

void FormantSearchBox::setupUi(Runtime &rt)
{
	String category("formants");
	DefaultSearchBox::setupUi(rt);
	add_button->hide();
	remove_button->hide();
	main_layout->addSpacing(10);

	auto measurement_box = new QGroupBox;
	auto box_layout = new QVBoxLayout;
	auto manual_button = new QRadioButton(tr("Manual"));
	parametric_button = new QRadioButton(tr("Parametric formant selection"));
	box_layout->addWidget(new QLabel(tr("Measurement method:")));
	box_layout->addWidget(parametric_button);
	box_layout->addWidget(manual_button);
	parametric_button->setChecked(true);

	// The stacked layout manages settings for the manual method and for the parametric method.
	stack = new QStackedLayout;
	auto parametric_widget = new QWidget;
	auto parametric_layout = new QVBoxLayout;
	param_min_freq_edit = new QLineEdit;
	param_max_freq_edit = new QLineEdit;
	param_step_freq_edit = new QLineEdit;
	param_min_freq_edit->setText("4000");
	param_max_freq_edit->setText("6000");
	param_step_freq_edit->setText("500");
	parametric_layout->addWidget(new QLabel(tr("Maximum frequency (Hz):")));
	auto freq_layout = new QHBoxLayout;
	freq_layout->addWidget(new QLabel(tr("from:")));
	freq_layout->addWidget(param_min_freq_edit);
	freq_layout->addWidget(new QLabel(tr("to:")));
	freq_layout->addWidget(param_max_freq_edit);
	freq_layout->addWidget(new QLabel(tr("step:")));
	freq_layout->addWidget(param_step_freq_edit);
	parametric_layout->addLayout(freq_layout);
	parametric_layout->addWidget(new QLabel(tr("LPC order:")));
	auto lpc_layout = new QHBoxLayout;
	param_lpc_min_spinbox = new QSpinBox;
	param_lpc_min_spinbox->setMinimum(4);
	param_lpc_min_spinbox->setMaximum(20);
	param_lpc_min_spinbox->setValue(8);
	param_lpc_max_spinbox = new QSpinBox;
	param_lpc_max_spinbox->setMinimum(4);
	param_lpc_max_spinbox->setMaximum(20);
	param_lpc_max_spinbox->setValue(12);
	lpc_layout->addWidget(new QLabel(tr("from:")));
	lpc_layout->addWidget(param_lpc_min_spinbox);
	lpc_layout->addWidget(new QLabel(tr("to:")));
	lpc_layout->addWidget(param_lpc_max_spinbox);
	lpc_layout->addStretch(1);
	parametric_layout->addLayout(lpc_layout);
	parametric_widget->setLayout(parametric_layout);

	auto manual_widget = new QWidget;
	auto manual_layout = new QVBoxLayout;
	auto manual_inner_layout = new QHBoxLayout;
	manual_inner_layout->addWidget(new QLabel(tr("Maximum frequency (Hz):")));
	int m = (int) Settings::get_number(rt, category, "max_frequency");
	max_freq_edit = new QLineEdit();
	max_freq_edit->setText(QString::number(m));
	manual_inner_layout->addWidget(max_freq_edit);
	lpc_spinbox = new QSpinBox;
	lpc_spinbox->setMinimum(4);
	lpc_spinbox->setMaximum(20);
	lpc_spinbox->setSingleStep(1);
	int order = (int) Settings::get_number(rt, category, "lpc_order");
	lpc_spinbox->setValue(order);
	manual_inner_layout->addWidget(new QLabel(tr("LPC order:")));
	manual_inner_layout->addWidget(lpc_spinbox);
	manual_inner_layout->addStretch(1);
	manual_layout->addLayout(manual_inner_layout);
	manual_layout->addStretch(1);
	manual_widget->setLayout(manual_layout);

	stack->addWidget(parametric_widget);
	stack->addWidget(manual_widget);
	stack->setCurrentIndex(0);
	box_layout->addLayout(stack);

	auto common_layout = new QHBoxLayout;
	formant_spinbox = new QSpinBox;
	formant_spinbox->setMinimum(2);
	formant_spinbox->setMaximum(7);
	formant_spinbox->setSingleStep(1);
	formant_spinbox->setValue(3);
	common_layout->addWidget(new QLabel(tr("Number of formants:")));
	common_layout->addWidget(formant_spinbox);
	common_layout->addWidget(new QLabel(tr("Window size (s):")));
	win_edit = new QLineEdit;
	double l = Settings::get_number(rt, category, "window_size");
	win_edit->setText(QString::number(l));
	common_layout->addWidget(win_edit);
	bark_checkbox = new QCheckBox(tr("Add measurements in bark"));
	erb_checkbox = new QCheckBox(tr("Add measurements in ERB units"));
	common_layout->addWidget(erb_checkbox);
	common_layout->addWidget(bark_checkbox);

	measurement_box->setLayout(box_layout);
	main_layout->addWidget(measurement_box);
	main_layout->addSpacing(10);
	main_layout->addLayout(common_layout);
	main_layout->addSpacing(10);

	connect(parametric_button, &QRadioButton::clicked, [this](bool) { changeMethod(0); });
	connect(manual_button, &QRadioButton::clicked, [this](bool) { changeMethod(1); });
}

void FormantSearchBox::changeMethod(int index)
{
	stack->setCurrentIndex(index);
}

AutoQuerySettings FormantSearchBox::getSettings() const
{
	bool ok;
	int nformant = formant_spinbox->value();
	bool bark = bark_checkbox->isChecked();
	bool erb = erb_checkbox->isChecked();
	double win_size = win_edit->text().toDouble(&ok);

	if (!ok) {
		throw error("Invalid window size");
	}

	if (parametric_button->isChecked())
	{
		double max_freq1 = param_min_freq_edit->text().toDouble(&ok);
		if (!ok) throw error("Invalid maximum frequency (from)");
		double max_freq2 = param_max_freq_edit->text().toDouble(&ok);
		if (!ok) throw error("Invalid maximum frequency (to)");
		double step = param_step_freq_edit->text().toDouble(&ok);
		if (!ok) throw error("Invalid frequency step");

		int lpc_order1 = param_lpc_min_spinbox->value();
		int lpc_order2 = param_lpc_max_spinbox->value();

		return std::make_unique<FormantQuerySettings>(win_size, nformant, max_freq1, max_freq2, step, lpc_order1,
				lpc_order2, erb, bark);
	}
	else
	{
		double max_freq = max_freq_edit->text().toDouble(&ok);
		if (!ok) throw error("Invalid maximum frequency");
		int lpc_order = lpc_spinbox->value();

		return std::make_unique<FormantQuerySettings>(win_size, nformant, max_freq, lpc_order, erb, bark);
	}
}


} // namespace phonometrica
