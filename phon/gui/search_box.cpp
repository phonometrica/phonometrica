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
#include <QSpinBox>
#include <QPushButton>
#include <phon/gui/search_box.hpp>
#include <phon/gui/line_edit.hpp>

namespace phonometrica {

static constexpr int SPINBOX_SIZE = 100;

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
	auto hlayout = new QHBoxLayout;
	add_button = new QPushButton(QIcon(":/icons/plus.png"), QString());
	remove_button = new QPushButton(QIcon(":/icons/minus.png"), QString());

	hlayout->addWidget(add_button);
	hlayout->addWidget(remove_button);
	hlayout->addStretch(1);

	main_layout = new QVBoxLayout(this);
	addSearchConstraint();
	main_layout->addLayout(hlayout);

	this->setLayout(main_layout);

	connect(add_button, &QPushButton::clicked, this, &DefaultSearchBox::addSearchConstraint);
	connect(remove_button, &QPushButton::clicked, this, &DefaultSearchBox::removeSearchConstraint);
}

void DefaultSearchBox::addSearchConstraint(bool )
{
	int i = constraint_count++;
	auto op_box = new QComboBox;
	if (i == 0) op_box->addItem("");
	op_box->addItem("AND");
	op_box->addItem("OR");

	auto start_box = new QComboBox;
	start_box->addItem("");
	start_box->addItem("(");

	auto end_box = new QComboBox;
	end_box->addItem("");
	end_box->addItem(")");

	auto match_box = new QComboBox;
	match_box->addItem(tr("matches"));
	match_box->addItem(tr("equals"));
	auto search_line = new LineEdit(tr("text or regular expression..."));
	auto case_box = new QCheckBox(tr("case sensitive"));
	case_box->setChecked(true);

	auto layer_combo = new QComboBox;
	layer_combo->addItem(tr("Layer index"));
	layer_combo->addItem(tr("Layer name (regex)"));

	auto relation = new QComboBox;
	if (i == 0) relation->addItem(tr(""));
	relation->addItem(tr("is aligned with"));
	relation->addItem(tr("precedes"));
	relation->addItem(tr("dominates"));

	auto layout = new QHBoxLayout;
	layout->addWidget(op_box);
	layout->addWidget(relation);
	layout->addWidget(start_box);
	layout->addWidget(layer_combo);
	layout->addWidget(createLayerSpinBox());
	layout->addWidget(match_box);
	layout->addWidget(search_line);
	layout->addWidget(case_box);
	layout->addWidget(end_box);

	int index = int(constraint_layouts.size());
	main_layout->insertLayout(index, layout);

	constraint_layouts.append(layout);
	relations.append(relation);

	remove_button->setEnabled(constraint_count > 1);

	if (i == 0)
	{
		op_box->setEnabled(false);
		relation->setEnabled(false);
	}

	updateFirstLayout();

    auto change_layer = [=](int index) {
    	this->changeLayerDisplay(i, index);
    };

    connect(layer_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), change_layer);
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
	relations.pop_last();
	--constraint_count;
	updateFirstLayout();
	remove_button->setEnabled(constraint_count > 1);
}

QSpinBox *DefaultSearchBox::createLayerSpinBox(int start)
{
	auto spinbox = new QSpinBox;
	spinbox->setRange(start, 100);
	spinbox->setSingleStep(1);
	if (start == 0)
		spinbox->setSpecialValueText(tr("any"));
	spinbox->setValue(start);
	spinbox->setFixedWidth(SPINBOX_SIZE);

	return spinbox;
}

LineEdit *DefaultSearchBox::createLayerEdit()
{
	auto layer_name = new LineEdit(tr("pattern"));
	layer_name->setFixedWidth(SPINBOX_SIZE);

	return layer_name;
}

void DefaultSearchBox::changeLayerDisplay(int constraint_index, int selection)
{
	constexpr int layer_pos = 4;
	auto layout = constraint_layouts[constraint_index + 1];
	auto item = layout->takeAt(layer_pos);
	auto new_widget = (selection == 0) ? (QWidget*)createLayerSpinBox(0) : (QWidget*)createLayerEdit();
	delete item->widget(); // FIXME: if we don't delete the widget explicitly, it still shows up when we add layers.
	delete item;
	layout->insertWidget(layer_pos, new_widget);
}

void DefaultSearchBox::updateFirstLayout()
{
	auto enable = (constraint_count != 1);
	auto layout = constraint_layouts.first();
	int open_index = 2;
	int close_index = layout->count() - 1;

	auto w0 = layout->itemAt(0)->widget();
	auto w1 = layout->itemAt(1)->widget();
	auto w2 = layout->itemAt(open_index)->widget();
	auto w3 = layout->itemAt(close_index)->widget();

	const bool fill_space = enable;

	w0->setHidden(true);
	w1->setHidden(true);
	w2->setHidden(!enable);
	w3->setHidden(!enable);
	retainWhenHidden(w0, fill_space);
	retainWhenHidden(w1, fill_space);
	retainWhenHidden(w2, fill_space);
	retainWhenHidden(w3, fill_space);

	w2->setEnabled(enable);
	w3->setEnabled(enable);


	if (!enable)
	{
		cast<QComboBox>(layout, open_index)->setCurrentIndex(0);
		cast<QComboBox>(layout, close_index)->setCurrentIndex(0);
	}
}

AutoSearchNode DefaultSearchBox::buildSearchTree()
{
	int i = 1;
	open_parentheses = 0;
	closed_parentheses = 0;
	return buildSearchTree(i);
}

AutoSearchNode DefaultSearchBox::buildSearchTree(int &layout_index)
{
	Array<AutoSearchNode> constraints;

	Array<AutoSearchNode> nodes;
	auto previous_operator = SearchOperator::Opcode::And;

	while (layout_index <= constraint_layouts.size())
	{
		auto layout = constraint_layouts[layout_index];
		auto constraint = parseConstraint(layout);

		if (nodes.empty())
		{
			nodes.append(std::move(constraint));
		}
		// TODO: here
//		else
//		if (current_operator != previous_operator)
//		{
//			Array<AutoSearchNode> tmp;
//			tmp.swap(nodes);
//			auto op_node = std::make_shared<SearchOperator>(previous_operator);
//			op_node->set_constraints(std::move(tmp));
//			nodes.append(std::move(op_node));
//			previous_operator = current_operator;
//		}

		nodes.append(std::move(constraint));

		layout_index++;
	}

	if (closed_parentheses != open_parentheses)
	{
		throw error("Invalid parentheses (found % open, % closed)", open_parentheses, closed_parentheses);
	}

	if (nodes.size() == 1)
	{
		return nodes.take_first();
	}
	else
	{
		auto op_node = std::make_shared<SearchOperator>(previous_operator);
		op_node->set_constraints(std::move(nodes));

		return op_node;
	}
}

AutoSearchConstraint DefaultSearchBox::parseConstraint(QHBoxLayout *layout)
{
	int i = 0;

	// Boolean operator.
	auto bool_box = cast<QComboBox>(layout, i++);
	auto current_operator = SearchOperator::Opcode::And;
	if (bool_box->currentIndex() != 0) current_operator = SearchOperator::Opcode::Or;

	// Relation.
	auto rel = SearchConstraint::Relation::None;
	auto rel_box = cast<QComboBox>(layout, i++);

	if (rel_box->isEnabled())
	{
		switch (rel_box->currentIndex())
		{
			case 0:
				rel = SearchConstraint::Relation::Alignment;
				break;
			case 1:
				rel = SearchConstraint::Relation::Precedence;
				break;
			default:
				rel = SearchConstraint::Relation::Dominance;
		}
	}

	// Opening parenthesis.
	auto open_box = cast<QComboBox>(layout, i++);
	bool open_paren = (open_box->currentIndex() == 1);
	if (open_paren) open_parentheses++;

	// Layer number or name.
	auto layer_choice = cast<QComboBox>(layout, i++);
	int layer_index = -1;
	String layer_name;

	if (layer_choice->currentIndex() == 0)
	{
		auto spinbox = cast<QSpinBox>(layout, i++);
		layer_index = spinbox->value();
	}
	else
	{
		auto edit = cast<LineEdit>(layout, i++);
		layer_name = edit->text();
	}

	// Match operator.
	auto match_box = cast<QComboBox>(layout, i++);
	auto op = (match_box->currentIndex() == 0) ? SearchConstraint::Opcode::Matches : SearchConstraint::Opcode::Equals;

	// Text value.
	auto text_edit = cast<LineEdit>(layout, i++);
	String value = text_edit->text();

	if (value.empty())
	{
		throw error("Empty text in constraint #%", layer_index);
	}

	// Case.
	auto case_box = cast<QCheckBox>(layout, i++);
	auto case_sensitive = case_box->isChecked();

	// Closing parenthesis.
	auto closing_box = cast<QComboBox>(layout, i++);
	bool closing_paren = (closing_box->currentIndex() == 1);

	if (closing_paren)
	{
		closed_parentheses++;

		if (closed_parentheses > open_parentheses)
		{
			throw error("Invalid closing parenthesis (found % open, % closed)", open_parentheses, closed_parentheses);
		}
	}

	return std::make_shared<SearchConstraint>(layer_index, layer_name, case_sensitive, op, rel, std::move(value));
}

void DefaultSearchBox::retainWhenHidden(QWidget *w, bool value)
{
	auto sp = w->sizePolicy();
	sp.setRetainSizeWhenHidden(value);
	w->setSizePolicy(sp);
}

} // namespace phonometrica