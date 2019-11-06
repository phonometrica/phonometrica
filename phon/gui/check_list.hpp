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
 * Purpose: implement a custom list of checkable items. The list is presented in a group box, with an additional       *
 * button to check all the items on or off.                                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHON_CHECKLIST_HPP
#define PHON_CHECKLIST_HPP

#include <QLabel>
#include <QListWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QStringList>
#include <QDebug>
#include <phon/string.hpp>

namespace phonometrica {

// Helper class for CheckListBox
class CheckList : public QListWidget
{
	Q_OBJECT

public:

	CheckList(QWidget *parent, const Array<String> &labels, const Array<String> &toolTips = Array<String>());

	QList<QCheckBox *> buttons();

	QList<QCheckBox *> checkedItems();

	void appendItem(QString label, QString tooltip = QString());

	void removeItem(QString text);

	void resetLabels(const Array<String> &labels, const Array<String> &toolTips = Array<String>());

	Array<String> checkedToolTips();

	std::vector<std::string> jsonSelection();

signals:

	void stateChanged(int index, int state);

private slots:

	void forwardState(int);

private:

	QButtonGroup *group = nullptr;

	QStringList m_labels, m_tooltips;

	bool hasTips;

	int indexFromCheckbox(QCheckBox *box);
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class CheckListBox : public QWidget
{
	Q_OBJECT

public:

	CheckListBox(const QString &title, const Array <String> &labels, QWidget *parent = nullptr);

	QString text(int index) const;

	int index(QString text) const;

	void addItem(QString item);

	void removeItem(QString item);

	QString title() const;

	Array<String> checkedLabels();

	Array<int> checkedIndexes();

	void setChecked(bool value);

	void checkItem(int i, bool value);

signals:

	void stateChanged(int index, int state);

public slots:

	void checkAll(int);

private:

	QVBoxLayout *layout;

	QCheckBox *switch_button;

	CheckList *checkList;

	QString m_title;
};


} // namespace phonometrica

#endif // PHON_CHECKLIST_HPP
