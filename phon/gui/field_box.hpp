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

	QString regex() const;

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
};

} // namespace phonometrica

#endif // PHONOMETRICA_FIELD_BOX_HPP
