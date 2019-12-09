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
#include <phon/include/reset_formants_settings_phon.hpp>

namespace phonometrica {

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
	main_layout->addSpacing(10);

	auto param_layout = new QHBoxLayout;
	formant_spinbox = new QSpinBox;
	formant_spinbox->setMinimum(2);
	formant_spinbox->setMaximum(7);
	formant_spinbox->setSingleStep(1);
	formant_spinbox->setValue(3);
	param_layout->addWidget(new QLabel(tr("Number of formants:")));
	param_layout->addWidget(formant_spinbox);
	param_layout->addWidget(new QLabel(tr("Maximum bandwidth (Hz):")));
	max_bw_edit = new QLineEdit;

	// If the user has an older version, "formants" might not be there.
	int bw;
	try
	{
		bw = (int) Settings::get_number(rt, category, "max_bandwidth");
	}
	catch (std::exception&)
	{
		run_script(rt, reset_formants_settings);
		bw = (int) Settings::get_number(rt, category, "max_bandwidth");
	}

	max_bw_edit->setText(QString::number(bw));
	param_layout->addWidget(max_bw_edit);
	param_layout->addWidget(new QLabel(tr("Window size (s):")));
	win_edit = new QLineEdit;
	double l = Settings::get_number(rt, category, "window_size");
	win_edit->setText(QString::number(l));
	param_layout->addWidget(win_edit);
	param_layout->addStretch(1);

	auto method_box = new QGroupBox;
	auto method_layout = new QVBoxLayout;
	auto manual_button = new QRadioButton(tr("Manual"));
	parametric_button = new QRadioButton(tr("Automatic formant selection (experimental)"));
	method_layout->addWidget(new QLabel(tr("Measurement method:")));
	method_layout->addWidget(manual_button);
	method_layout->addWidget(parametric_button);
	manual_button->setChecked(true);

	// The stacked layout manages settings for the manual method and for the automatic method.
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
	param_lpc_min_spinbox->setValue(10);
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

	stack->addWidget(manual_widget);
	stack->addWidget(parametric_widget);
	stack->setCurrentIndex(0);
	method_layout->addLayout(stack);
	method_box->setLayout(method_layout);

	auto location_box = new QGroupBox;
	auto location_layout = new QVBoxLayout;
	location_layout->addWidget(new QLabel(tr("Measurement location:")));
	mid_radio = new QRadioButton(tr("Mid point"));
	npoint_radio = new QRadioButton(tr("N-point average (%)"));
	mid_radio->setChecked(true);
	npoint_edit = new QLineEdit;
	npoint_edit->setEnabled(false);
	location_layout->addWidget(mid_radio);
	location_layout->addWidget(npoint_radio);
	location_layout->addWidget(npoint_edit);
	location_box->setLayout(location_layout);

	auto option_layout = new QHBoxLayout;
	bandwidth_checkbox = new QCheckBox(tr("Add bandwidth"));
	bark_checkbox = new QCheckBox(tr("Add formants in bark"));
	erb_checkbox = new QCheckBox(tr("Add formants in ERB units"));
	option_layout->addWidget(bandwidth_checkbox);
	option_layout->addWidget(erb_checkbox);
	option_layout->addWidget(bark_checkbox);
	option_layout->addStretch(1);

	auto option2_layout = new QHBoxLayout;
	surrounding_checkbox = new QCheckBox(tr("Add surrounding labels"));
	labels_checkbox = new QCheckBox(tr("Add labels from other layers:"));
	labels_edit = new LineEdit("layer indexes separated by a space");
	option2_layout->addWidget(surrounding_checkbox);
	option2_layout->addWidget(labels_checkbox);
	option2_layout->addWidget(labels_edit);
	labels_edit->setEnabled(false);
	//option2_layout->addStretch(1);

	main_layout->addLayout(param_layout);
	main_layout->addWidget(method_box);
	main_layout->addWidget(location_box);
	main_layout->addSpacing(10);
	main_layout->addLayout(option_layout);
	main_layout->addLayout(option2_layout);
	main_layout->addSpacing(10);

	connect(parametric_button, &QRadioButton::clicked, [this](bool) { changeMethod(1); });
	connect(manual_button, &QRadioButton::clicked, [this](bool) { changeMethod(0); });
	connect(labels_checkbox, &QCheckBox::clicked, [this](bool val) { labels_edit->setEnabled(val); });
	connect(mid_radio, &QRadioButton::clicked, [this](bool) {
		npoint_edit->setText(QString());
		npoint_edit->setEnabled(false);
	});
	connect(npoint_radio, &QRadioButton::clicked, [this](bool) {
		npoint_edit->setText("25 50 75");
		npoint_edit->setEnabled(true);
	});
}

void FormantSearchBox::changeMethod(int index)
{
	stack->setCurrentIndex(index);
}

AutoQuerySettings FormantSearchBox::getSettings() const
{
	bool ok;
	int nformant = formant_spinbox->value();
	bool bw = bandwidth_checkbox->isChecked();
	bool bark = bark_checkbox->isChecked();
	bool erb = erb_checkbox->isChecked();
	double win_size = win_edit->text().toDouble(&ok);

	if (!ok || win_size <= 0) {
		throw error("Invalid window size");
	}

	double max_bw = max_bw_edit->text().toDouble(&ok);

	if (!ok || max_bw <= 0) {
		throw error("Invalid maximum bandwidth");
	}

	bool add_surrounding = surrounding_checkbox->isChecked();
	Array<int> label_indexes;

	if (labels_checkbox->isChecked())
	{
		auto labels = labels_edit->text().split(" ");
		for (auto &label : labels)
		{
			int i = label.toInt(&ok);
			if (!ok || i <= 0) {
				throw error("Invalid label index: %", label.toStdString());
			}
			label_indexes.append(i);
		}
	}

	auto method = mid_radio->isChecked() ? AcousticQuerySettings::Method::Mid : AcousticQuerySettings::Method::Average;
	Array<float> measurements;

	if (method != AcousticQuerySettings::Method::Mid)
	{
		auto points = npoint_edit->text().split(' ');
		bool ok;
		for (auto &s : points)
		{
			float p = s.toFloat(&ok);
			if (!ok || p < 0 || p > 100)
			{
				throw error("Invalid measurement point (must be between 0 and 100%)");
			}
			measurements.push_back(p);
		}
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

		return std::make_shared<FormantQuerySettings>(method, add_surrounding, std::move(measurements), std::move(label_indexes), win_size, nformant,
				max_bw, max_freq1, max_freq2, step, lpc_order1, lpc_order2, bw, erb, bark);
	}
	else
	{
		double max_freq = max_freq_edit->text().toDouble(&ok);
		if (!ok) throw error("Invalid maximum frequency");
		int lpc_order = lpc_spinbox->value();

		return std::make_shared<FormantQuerySettings>(method, add_surrounding, std::move(measurements), std::move(label_indexes), win_size, nformant,
				max_bw, max_freq, lpc_order, bw, erb, bark);
	}
}

Query::Type FormantSearchBox::getType() const
{
	return Query::Type::Formants;
}


} // namespace phonometrica
