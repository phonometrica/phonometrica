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
 * Created: 31/08/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <phon/gui/search_box.hpp>
#include <phon/gui/line_edit.hpp>

namespace phonometrica {

SearchBox::SearchBox(QWidget *parent, const QString &title) :
	QGroupBox(title, parent)
{

}

void SearchBox::postInitialize()
{
	setupUi();
}

//----------------------------------------------------------------------------------------------------------------------

DefaultSearchBox::DefaultSearchBox(QWidget *parent) :
	SearchBox(parent, tr("Search constraints"))
{
}

void DefaultSearchBox::setupUi()
{
	and_button = new QRadioButton("Use AND operator");
	or_button = new QRadioButton("Use OR operator");
	custom_button = new QRadioButton("Custom (you can use AND, OR, NOT, as well as parentheses for grouping)");
	and_button->setChecked(true);
	auto boolean_group = new QButtonGroup;
	boolean_group->addButton(and_button, 0);
	boolean_group->addButton(or_button, 1);
	boolean_group->addButton(custom_button, 2);

	auto hlayout = new QHBoxLayout;
	add_button = new QPushButton(QIcon(":/icons/plus.png"), QString());
	remove_button = new QPushButton(QIcon(":/icons/minus.png"), QString());
	hlayout->addWidget(add_button);
	hlayout->addWidget(remove_button);
	hlayout->addStretch(1);

	query_display = new QTextEdit;
	query_display->setMaximumHeight(40);

	main_layout = new QVBoxLayout(this);
	addSearchConstraint();
	main_layout->addLayout(hlayout);
	main_layout->addSpacing(10);
	main_layout->addWidget(and_button);
	main_layout->addWidget(or_button);
	main_layout->addWidget(custom_button);
	main_layout->addWidget(query_display);

	this->setLayout(main_layout);

	connect(add_button, &QPushButton::clicked, this, &DefaultSearchBox::addSearchConstraint);
	connect(remove_button, &QPushButton::clicked, this, &DefaultSearchBox::removeSearchConstraint);
	connect(and_button, &QRadioButton::clicked, this, &DefaultSearchBox::updateQueryString);
	connect(or_button, &QRadioButton::clicked, this, &DefaultSearchBox::updateQueryString);
	connect(custom_button, &QRadioButton::clicked, this, &DefaultSearchBox::updateQueryString);
}

void DefaultSearchBox::addSearchConstraint(bool )
{
	int i = constraint_count++;

	auto id_label = new QLabel(QString("#%1").arg(constraint_count));
	auto match_box = new QComboBox;
	match_box->addItem(tr("matches"));
	match_box->addItem(tr("equals"));
	auto search_line = new LineEdit(tr("text or regular expression..."));
	auto case_box = new QCheckBox(tr("case sensitive"));
	case_box->setChecked(true);

	auto layer_combo = new QComboBox;
	layer_combo->addItem(tr("Layer index"));
	layer_combo->addItem(tr("Layer name (regex)"));

	auto layer_name = new LineEdit(tr("any layer"));
	layer_name->setMaximumWidth(120);
	auto relation = new QComboBox;
	relation->addItem(tr("is aligned with"));
	relation->addItem(tr("precedes"));
	relation->addItem(tr("dominates"));

	auto layout = new QHBoxLayout;
	layout->addWidget(id_label);
	layout->addWidget(layer_combo);
	layout->addWidget(layer_name);
	layout->addWidget(match_box);
	layout->addWidget(search_line);
	layout->addWidget(case_box);
	layout->addWidget(relation);
	int index = int(constraint_layouts.size());
	main_layout->insertLayout(index, layout);
	constraint_layouts.append(layout);

	remove_button->setEnabled(constraint_count > 1);

	// Single-layer queries don't use relations
    if (i == 0) relation->setEnabled(false);
    updateQueryString(true);
}

void DefaultSearchBox::removeSearchConstraint(bool)
{
	auto layout = constraint_layouts.take_last();

	QLayoutItem *item = nullptr;
	while ((item = layout->takeAt(0)) != nullptr)
	{
		QWidget *widget = item->widget();
		if (widget)
		{
			widget->hide();
			delete widget;
		}
	}
	delete layout;

	--constraint_count;
	remove_button->setEnabled(constraint_count > 1);
	updateQueryString(true);
}

void DefaultSearchBox::updateQueryString(bool)
{
	Operator op;

	if (and_button->isChecked())
		op = Operator::And;
	else if (or_button->isChecked())
		op = Operator::Or;
	else
		op = Operator::Custom;

	QString query;
	QString infix = (op == Operator::Or) ? " OR " : " AND ";

	for (int i = 1; i <= constraint_count; i++)
	{
		auto constr = QString("#%1").arg(i);
		if (i > 1) query.append(infix);
		query.append(constr);
	}

	query_display->setText(query);
	query_display->setEnabled(op == Operator::Custom);
}


} // namespace phonometrica