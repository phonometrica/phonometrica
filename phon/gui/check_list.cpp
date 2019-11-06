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
 * Created: 05/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/check_list.hpp>

namespace phonometrica {

CheckList::CheckList(QWidget *parent, const Array <String> &labels, const Array <String> &toolTips) :
    QListWidget(parent)
{
	group = new QButtonGroup;
	group->setExclusive(false);
    //this->setContentsMargins(0,0,0,0);
#ifdef Q_OS_WIN
    this->setSpacing(1);
#endif
    resetLabels(labels, toolTips);
}

void CheckList::resetLabels(const Array<String> &labels, const Array <String> &toolTips)
{
    m_tooltips.clear();
    m_labels.clear();
    this->clear();

    hasTips = (labels.size() == toolTips.size());

    for (intptr_t i = 1; i <= labels.size(); i++)
    {
        QString label = labels[i];
        String tip = hasTips? toolTips[i]: String();
        this->appendItem(label, tip);
    }
}

void CheckList::appendItem(QString label, QString tooltip)
{
	QListWidgetItem *item = new QListWidgetItem(this);
	QCheckBox *button = new QCheckBox(label);
//	QFont font("Liberation Mono");
//	font.setStyleHint(QFont::TypeWriter);
//	button->setFont(font);
	if (hasTips)
	{
		button->setToolTip(tooltip);
		m_tooltips << tooltip;
	}
	group->addButton(button);
	connect(button, SIGNAL(stateChanged(int)), this, SLOT(forwardState(int)));
	setItemWidget(item, button);

	m_labels << label;
}

void CheckList::removeItem(QString text)
{
	// try to remove item, starting from the end
	int index = -1;

	for (int i = this->count()-1; i >= 0; --i)
	{
		QCheckBox *box = qobject_cast<QCheckBox*>(itemWidget(item(i)));
		if (box->text() == text)
		{
			index = i;
			break;
		}
	}

	if (index != -1 && index < m_labels.count())
	{
		QListWidgetItem *oldItem = this->takeItem(index);
		if (oldItem)
			delete oldItem;
		m_labels.removeAt(index);
		if (hasTips)
			m_tooltips.removeAt(index);
	}
}

void CheckList::forwardState(int state)
{
	int index = indexFromCheckbox(qobject_cast<QCheckBox*>(sender()));
	emit stateChanged(index, state);
}

QList<QCheckBox*> CheckList::buttons()
{
	QList<QCheckBox*> buttons;

	foreach (QAbstractButton *btn, group->buttons())
		buttons << static_cast<QCheckBox*>(btn);

	return buttons;
}

QList<QCheckBox*> CheckList::checkedItems()
{
	QList<QCheckBox*> items;

	foreach (QCheckBox *item, buttons())
	{
		if (item->isChecked())
			items << item;
	}

	return items;
}

int CheckList::indexFromCheckbox(QCheckBox *box)
{
	int i;
	for (i = 0; i < this->count(); ++i)
	{
		if (qobject_cast<QCheckBox*>(this->itemWidget(item(i))) == box)
			return i;
	}

	return -1;
}

Array<String> CheckList::checkedToolTips()
{
	Array<String> results;

	for (QCheckBox *btn : this->buttons())
	{
		if (btn->isChecked())
		{
			results.append(btn->toolTip());
		}
	}

	return results;
}

std::vector<std::string> CheckList::jsonSelection()
{
	std::vector<std::string> values;

	for (QCheckBox *btn : this->buttons())
	{
		if (btn->isChecked())
		{
			QString txt = btn->toolTip();
			if (txt.isEmpty()) txt = btn->text();
			values.push_back(txt.toStdString());
		}
	}

	return values;
}

/*****************************************************************************************************
 *****************************************************************************************************
 *****************************************************************************************************/

CheckListBox::CheckListBox(const QString &title, const Array<String> &labels, QWidget *parent) :
        QWidget(parent)
{
    m_title = title;
	layout = new QVBoxLayout;
    layout->setContentsMargins(5,5,5,5);
    //layout->setSpacing(2);
    QHBoxLayout *hl = new QHBoxLayout;
    switch_button = new QCheckBox;
#ifdef Q_OS_WIN
    hl->addSpacing(2);
#endif
    hl->setSpacing(0);
    hl->setContentsMargins(0, 0, 0, 0);
    hl->addWidget(new QLabel("<b>" + title + "</b>"));
	hl->addStretch();
	hl->addWidget(switch_button);
	checkList = new CheckList(nullptr, labels);
    checkList->setContentsMargins(0,0,0,0);
    this->setContentsMargins(0,0,0,10);

    layout->addLayout(hl);
	layout->addWidget(checkList);
	setLayout(layout);

	connect(switch_button, SIGNAL(stateChanged(int)), this, SLOT(checkAll(int)));
	connect(checkList, SIGNAL(stateChanged(int,int)), this, SIGNAL(stateChanged(int,int)));
}

QString CheckListBox::title() const
{
    return m_title;
}

void CheckListBox::checkAll(int state)
{
	if (state == Qt::Checked)
	{
		foreach (QCheckBox *btn, checkList->buttons())
			btn->setChecked(true);
	}
	else if (state == Qt::Unchecked)
	{
		for (QCheckBox *btn : checkList->buttons())
			btn->setChecked(false);
	}
	else
		qDebug() << tr("Unmanaged state in CheckListBox::checkAll()");

}

Array<String> CheckListBox::checkedLabels()
{
	Array<String> results;

	for (QCheckBox *btn : checkList->buttons())
	{
		if (btn->isChecked())
		{
			results.append(btn->text());
		}
	}

	return results;
}

QString CheckListBox::text(int index) const
{
	auto box = qobject_cast<QCheckBox*>(checkList->itemWidget(checkList->item(index)));
	return box->text();
}

int CheckListBox::index(QString text) const
{
	int i;
	for (i = 0; i < checkList->count(); ++i)
	{
		QCheckBox *box = qobject_cast<QCheckBox*>(checkList->itemWidget(checkList->item(i)));
		if (box->text() == text)
			return i;
	}

	return -1;
}

void CheckListBox::addItem(QString item)
{
	checkList->appendItem(item);
}

void CheckListBox::removeItem(QString item)
{
	checkList->removeItem(item);
}

Array<int> CheckListBox::checkedIndexes()
{
	Array<int> results;
	auto buttons = checkList->buttons();

	for (int i = 0; i < buttons.size(); i++)
	{
		auto btn = buttons[i];

		if (btn->isChecked())
		{
			results.append(i+1);
		}
	}

	return results;
}

void CheckListBox::setChecked(bool value)
{
	switch_button->setChecked(value);
}

void CheckListBox::checkItem(int i, bool value)
{
	checkList->buttons()[i]->setChecked(value);
}

} // namespace phonometrica

