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