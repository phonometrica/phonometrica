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
 * Created: 31/08/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
static constexpr int OPERATOR_POS = 0;
static constexpr int NOT_POS = OPERATOR_POS + 1;
static constexpr int RELATION_POS = NOT_POS + 1;
static constexpr int OPEN_PAREN_POS = RELATION_POS + 1;

SearchBox::SearchBox(QWidget *parent, const QString &title, int context_length) :
	QGroupBox(title, parent)
{
	this->context_length = context_length;
}

void SearchBox::postInitialize(Runtime &rt)
{
	setupUi(rt);
}

//----------------------------------------------------------------------------------------------------------------------

DefaultSearchBox::DefaultSearchBox(QWidget *parent, int context_length) :
	SearchBox(parent, tr("Search constraints"), context_length)
{
}

void DefaultSearchBox::setupUi(Runtime &rt)
{
	auto add_remove_layout = new QHBoxLayout;
	add_button = new QPushButton(QIcon(":/icons/plus.png"), QString());
	remove_button = new QPushButton(QIcon(":/icons/minus.png"), QString());
	add_button->setEnabled(false);

	add_remove_layout->addWidget(add_button);
	add_remove_layout->addWidget(remove_button);
	add_remove_layout->addStretch(1);

	main_layout = new QVBoxLayout(this);
	addSearchConstraint();
	main_layout->addLayout(add_remove_layout);

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

	auto not_box = new QComboBox;
	not_box->addItem("");
	not_box->addItem("NOT");

	auto start_box = new QComboBox;
	start_box->addItem("");
	start_box->addItem("(");

	auto end_box = new QComboBox;
	end_box->addItem("");
	end_box->addItem(")");

	auto match_box = new QComboBox;
	match_box->addItem(tr("match pattern"));
	match_box->addItem(tr("find text"));

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
	layout->addWidget(not_box);
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
		not_box->setEnabled(false);
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
	constexpr int layer_pos = 5;
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
	int open_index = 3;
	int close_index = layout->count() - 1;

	auto w0 = layout->itemAt(0)->widget();
	auto w1 = layout->itemAt(1)->widget();
	auto w2 = layout->itemAt(2)->widget();
	auto w3 = layout->itemAt(open_index)->widget();
	auto w4 = layout->itemAt(close_index)->widget();

	const bool fill_space = enable;

	w0->setHidden(true);
	w1->setHidden(true);
	w2->setHidden(true);
	w3->setHidden(!enable);
	w4->setHidden(!enable);
	retainWhenHidden(w0, fill_space);
	retainWhenHidden(w1, fill_space);
	retainWhenHidden(w2, fill_space);
	retainWhenHidden(w3, fill_space);
	retainWhenHidden(w4, fill_space);

	w3->setEnabled(enable);
	w4->setEnabled(enable);


	if (!enable)
	{
		cast<QComboBox>(layout, open_index)->setCurrentIndex(0);
		cast<QComboBox>(layout, close_index)->setCurrentIndex(0);
	}
}

void DefaultSearchBox::retainWhenHidden(QWidget *w, bool value)
{
	auto sp = w->sizePolicy();
	sp.setRetainSizeWhenHidden(value);
	w->setSizePolicy(sp);
}

AutoSearchNode DefaultSearchBox::buildSearchTree()
{
	open_parentheses = 0;
	closed_parentheses = 0;
	QueryParser parser(getTokens());

	if (closed_parentheses != open_parentheses)
	{
		throw error("Invalid parentheses (found % open, % closed)", open_parentheses, closed_parentheses);
	}

	return parser.parse();
}

AutoSearchNode DefaultSearchBox::parseConstraint(int layout_index)
{
	auto layout = constraint_layouts[layout_index];
	int i = 2; // skip Boolean operators, which are handled separately.

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
	auto open_paren = (open_box->currentIndex() == 1);
	if (open_paren)
		open_parentheses++;

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
	auto op = (match_box->currentIndex() == 0) ? SearchConstraint::Opcode::Matches : SearchConstraint::Opcode::Contains;

	// Text value.
	auto text_edit = cast<LineEdit>(layout, i++);
	String value = text_edit->text();

	if (value.empty())
	{
		throw error("Empty text in constraint #%", layout_index);
	}

	// Case.
	auto case_box = cast<QCheckBox>(layout, i++);
	auto case_sensitive = case_box->isChecked();

	// Closing parenthesis.
	auto closing_box = cast<QComboBox>(layout, i++);
	auto closing_paren = (closing_box->currentIndex() == 1);

	if (closing_paren)
	{
		closed_parentheses++;

		if (closed_parentheses > open_parentheses)
		{
			throw error("Invalid closing parenthesis (found % open, % closed)", open_parentheses, closed_parentheses);
		}
	}

	return std::make_shared<SearchConstraint>(nullptr, context_length, layout_index, layer_index, layer_name,
			case_sensitive, op, rel, std::move(value));
}

std::pair<bool, bool> DefaultSearchBox::getParentheses(int layout_index)
{
	std::pair<bool, bool> paren;
	auto layout = constraint_layouts[layout_index];
	// Opening parenthesis.
	auto open_box = cast<QComboBox>(layout, OPEN_PAREN_POS);
	paren.first = (open_box->currentIndex() == 1);
	int last = layout->count() - 1;
	auto closing_box = cast<QComboBox>(layout, last);
	paren.second = (closing_box->currentIndex() == 1);

	return paren;
}

DefaultSearchBox::Type DefaultSearchBox::getOperator(int i)
{
	if (i == 1 || i > constraint_layouts.size()) {
		return Type::Null;
	}
	auto layout = constraint_layouts[i];

	return (cast<QComboBox>(layout, OPERATOR_POS)->currentIndex()) == 0 ? Type::And : Type::Or;
}

Array<DefaultSearchBox::Token> DefaultSearchBox::getTokens()
{
	// Build a stream of tokens from the constraint layouts.
	Array<Token> tokens(64);

	for (int i = 1; i <= constraint_layouts.size(); i++)
	{
		auto paren = getParentheses(i);

		if (i > 1)
		{
			tokens.append({ getOperator(i), nullptr });

			if (hasNotOperator(i)) {
				tokens.append({ Type::Not, nullptr });
			}
		}

		if (paren.first && !paren.second) {
			tokens.append({ Type::LParen, nullptr });
		}

		tokens.append({ Type::Constraint, parseConstraint(i) });

		if (paren.second && !paren.first) {
			tokens.append({ Type::Rparen, nullptr });
		}
	}

	return tokens;
}

bool DefaultSearchBox::hasNotOperator(int index)
{
	auto layout = constraint_layouts[index];
	auto not_box = cast<QComboBox>(layout, NOT_POS);

	return (not_box->currentIndex() == 1);
}

AutoQuerySettings DefaultSearchBox::getSettings() const
{
	return std::make_shared<Query::Settings>(getType());
}

Query::Type DefaultSearchBox::getType() const
{
	return Query::Type::Default;
}

} // namespace phonometrica