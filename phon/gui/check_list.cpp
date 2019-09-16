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

} // namespace phonometrica

