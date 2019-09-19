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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/field_box.hpp>

namespace phonometrica {

FieldValueWidget::FieldValueWidget(const SearchValue &value, QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setSpacing(5);
    vl->setContentsMargins(0,0,0,0);
    choice_box = nullptr;

    m_button = new QCheckBox(value.text);
    matchall = value.match;
	layer_name = value.layer_name;
    vl->addWidget(m_button);

    if (!value.choices.empty())
    {
       choice_box = new QxtCheckComboBox;
        foreach(SearchChoice c, value.choices)
        {
            choice_box->addItem(c.text);
            choice_hash.insert(c.text, c.match);
        }
        vl->addWidget(choice_box);
    }

    this->setLayout(vl);
}


bool FieldValueWidget::isChecked() const
{
    return m_button->isChecked();
}

void FieldValueWidget::setChecked(bool checked)
{
    m_button->setChecked(checked);
}

void FieldValueWidget::toggle(bool checked)
{
    m_button->setChecked(checked);
}

QString FieldValueWidget::match() const
{
    if (choice_box == nullptr)
        return matchall;

    else
    {
        QStringList checked_values = choice_box->checkedItems();

        if (checked_values.isEmpty())
            return matchall;
        else
        {
            QStringList choices;
            for (QString &choice : checked_values)
                choices << choice_hash.value(choice);

            if (choices.count() == 1)
                return choices.at(0);
            else
                return QString("(%1)").arg(choices.join("|"));
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


FieldBox::FieldBox(const SearchField &field, QString allValues, QWidget *parent) :
    QWidget(parent)
{
    matchall = "(" + QString(field.match_all) + ")";
    m_name = field.name;
	m_layer_pattern = field.layer_pattern;
    QVBoxLayout *dummy = new QVBoxLayout;
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setSpacing(1);
    vl->setContentsMargins(10,10,10,10);

    QGroupBox *group = new QGroupBox(m_name);

    QCheckBox *header_box = new QCheckBox;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0,0,0,0);
    hlayout->addWidget(header_box);
    hlayout->addSpacing(10);
    hlayout->addWidget(new QLabel("<b>" + allValues + "</b>"));
    hlayout->addStretch();
    vl->addLayout(hlayout);
    vl->addSpacing(10);

    foreach(SearchValue v, field.values)
    {
        FieldValueWidget *fieldValue = new FieldValueWidget(v);
        value_list.append(fieldValue);
        vl->addWidget(fieldValue);
    }
    vl->addStretch();
    group->setLayout(vl);
    dummy->addWidget(group);
    this->setLayout(dummy);

    connect(header_box, SIGNAL(toggled(bool)), this, SLOT(toggleAll(bool)));
}

void FieldBox::toggleAll(bool checked)
{
    for (FieldValueWidget *value : value_list)
        value->setChecked(checked);
}

QString FieldBox::get_pattern() const
{
    if (allChecked() || noneChecked())
    {
	    return matchall;
    }

    QStringList values;

    for (FieldValueWidget *box : value_list)
    {
        if (box->isChecked())
            values << box->match();
    }

    return QString("(%1)").arg(values.join("|"));
}

QString FieldBox::tierNamePattern() const
{
    if (allChecked() || noneChecked())
        return m_layer_pattern;

    QStringList names;

    for (FieldValueWidget *box : value_list)
    {
        if (box->isChecked())
            names << box->layerName();
    }

    return names.join("|");
}

QString FieldBox::name() const
{
    return m_name;
}

bool FieldBox::allChecked() const
{
    for (FieldValueWidget *value : value_list)
    {
        if (!value->isChecked())
            return false;
    }

    // if there's only one value (which may contain a choice list), use this value instead of "matchall"
    if (value_list.size() == 1)
        return false;

    return true;
}

bool FieldBox::noneChecked() const
{
    for (FieldValueWidget *value : value_list)
    {
        if (value->isChecked())
            return false;
    }

    return true;
}

void FieldBox::checkAll()
{
    for (FieldValueWidget *value : value_list)
    {
        value->setChecked(true);
    }
}

} // namespace phonometrica