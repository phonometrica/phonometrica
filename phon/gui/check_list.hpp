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
