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
 * Created: 18/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: represent a field in a protocol search box.                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FIELD_BOX_HPP
#define PHONOMETRICA_FIELD_BOX_HPP

#include <QWidget>
#include <QCheckBox>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <phon/application/protocol.hpp>
#include <phon/third_party/qxt/qxtcheckcombobox.h>

namespace phonometrica {

// Helper class for FieldBox
class FieldValueWidget : public QWidget
{
	Q_OBJECT

public:

	FieldValueWidget(const SearchValue &value, QWidget *parent = nullptr);

	bool isChecked() const;

	void setChecked(bool checked);

	QString match() const;

	QString layerName() const { return layer_name; }

public slots:

	void toggle(bool checked);

private:

	QCheckBox *m_button;
	QxtCheckComboBox *choice_box;
	QString matchall, layer_name;
	QHash<QString, QString> choice_hash;
	bool hasChoices;
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class FieldBox : public QWidget
{
Q_OBJECT

public:
	explicit FieldBox(const SearchField &field, QString allValues = tr("All values"), QWidget *parent = nullptr);

	QString name() const;

	QString get_pattern() const;

	QString tierNamePattern() const;

signals:

public slots:

private slots:

	void toggleAll(bool);

private:

	QString matchall, m_name, m_layer_pattern;

	QList<FieldValueWidget *> value_list;

	bool allChecked() const;

	bool noneChecked() const;

	void checkAll();
};

} // namespace phonometrica

#endif // PHONOMETRICA_FIELD_BOX_HPP
