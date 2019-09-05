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
 * Created: O8/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/gui/property_editor.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {

PropertyEditor::PropertyEditor(QWidget *parent, VFileList files) :
    QDialog(parent), m_files(std::move(files))
{
    setupUi();
}

void PropertyEditor::setupUi()
{
    setWindowTitle(tr("Property editor"));
    if (objectName().isEmpty())
        setObjectName(QString::fromUtf8("PropertyEditor"));
    resize(430, 500);

    layout = new QVBoxLayout;
    layout->setContentsMargins(15,20,15,10);

    ok_btn = new QPushButton("OK");
	QHBoxLayout *btn_layout = new QHBoxLayout;
	btn_layout->addStretch();
    btn_layout->addWidget(ok_btn);

    type_box = new QComboBox;
    type_box->addItem("Text");
    type_box->addItem("Numeric");
    type_box->addItem("Boolean");
    type_box->setCurrentIndex(0);

    addProperty_btn = new QPushButton(tr("Add"));
    addProperty_btn->setEnabled(false);
    rmProperty_btn  = new QPushButton(tr("Remove"));
    rmProperty_btn->setEnabled(false);

    property_table = new PropertyTable(Project::get_shared_properties(m_files));
    property_table->setMaximumHeight(200);
    property_table->horizontalHeader()->resizeSection(0, 150);

    QHBoxLayout *add_rm_layout = new QHBoxLayout;
    add_rm_layout->setContentsMargins(0,0,0,0);
    add_rm_layout->setSpacing(2);

    add_rm_layout->addWidget(addProperty_btn);
    add_rm_layout->addWidget(rmProperty_btn);
    add_rm_layout->addStretch();

    cat_line = new QLineEdit;
    value_line = new QLineEdit;

    cat_list = new QListWidget;
    int HEIGHT = 55;
    cat_list->setMinimumHeight(HEIGHT);
    cat_list->setMaximumHeight(HEIGHT);
    value_list = new QListWidget;
    value_list->setMinimumHeight(HEIGHT);
    value_list->setMaximumHeight(HEIGHT);

	updateCategories();

    int row = 0;
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Type:")), row, 0);
    grid->addWidget(type_box, row++, 1);
    grid->addWidget(new QLabel(tr("Category:")), row, 0);
    grid->addWidget(cat_line, row++, 1);
    grid->addWidget(cat_list, row++, 1);
    grid->addWidget(new QLabel(tr("Value:")), row, 0);
    grid->addWidget(value_line, row++, 1);
    grid->addWidget(value_list, row++, 1);
    grid->addWidget(property_table, row++, 1);
    grid->addLayout(add_rm_layout, row, 1);

    layout->addLayout(grid);
    layout->addStretch();
    layout->addLayout(btn_layout);

    this->setLayout(layout);

    connect(cat_list, SIGNAL(clicked(QModelIndex)), this, SLOT(updateValues()));
    connect(value_list, SIGNAL(clicked(QModelIndex)), this, SLOT(updateSelectedValue()));
    connect(cat_line, SIGNAL(textChanged(QString)), this, SLOT(propertyContentEdited()));
    //connect(cat_line, SIGNAL(textChanged(QString)), this, SLOT(checkPropertyIsRemovable()));
    connect(value_line, SIGNAL(textChanged(QString)), this, SLOT(propertyContentEdited()));
    //connect(value_line, SIGNAL(textChanged(QString)), this, SLOT(checkPropertyIsRemovable()));
    connect(property_table, SIGNAL(cellClicked(int,int)), this, SLOT(selectRow(int)));
    connect(ok_btn, SIGNAL(clicked()), this, SLOT(accept()));
//	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(addProperty_btn, SIGNAL(clicked()), this, SLOT(addProperty()));
	connect(rmProperty_btn, SIGNAL(clicked()), this, SLOT(removeProperty()));
}

void PropertyEditor::updateValues()
{
    if (cat_list->count() > 0)
    {
        QString category = cat_list->currentItem()->text();
        cat_line->setText(category);
        cat_line->setFocus();
        cat_line->selectAll();

        QStringList values;

        for (auto &prop : Property::get_values(category)) {
            values.append(prop);
        }
        value_list->clear();
        value_list->addItems(values);
        value_line->clear();
    }
}

void PropertyEditor::propertyContentEdited()
{
    if (!cat_line->text().isEmpty() && !value_line->text().isEmpty())
    {
        addProperty_btn->setEnabled(true);
    }
}

void PropertyEditor::updateSelectedValue()
{
    if (value_list->count() > 0)
    {
        QString value = value_list->currentItem()->text();
        value_line->setText(value);
        value_line->setFocus();
        value_line->selectAll();
    }
}

void PropertyEditor::addProperty()
{
    String category = cat_line->text();
    String value = value_line->text();
    Property prop;

    try
    {
	    switch (type_box->currentIndex())
	    {
		    case 1:
			    prop = Property(category, value.to_float());
			    break;
		    case 2:
			    prop = Property(category, value.to_bool(true));
			    break;
		    default:
			    prop = Property(category, value);
	    }
    }
	catch (std::exception &e)
	{
		auto msg = utils::format("Invalid value: %", e.what());
		QMessageBox dlg(QMessageBox::Critical, "Error", QString::fromStdString(msg));
		dlg.exec();
		return;
	}

    for (auto &vf : m_files) {
        vf->add_property(prop);
    }

    // refresh all properties: this avoids having duplicates if the value of a property has been
    // overwritten by another one.
    property_table->resetProperties(Project::get_shared_properties(m_files));


    cat_line->clear();
    value_line->clear();
    addProperty_btn->setEnabled(false);
    rmProperty_btn->setEnabled(false);
}

void PropertyEditor::removeProperty()
{
    int row = property_table->currentRow();

    String cat = property_table->category(row);
    String val = property_table->value(row);

    property_table->removeRow(row);

    for (auto &vf : m_files) {
        vf->remove_property(cat, val);
    }

    rmProperty_btn->setEnabled(false);
}

void PropertyEditor::selectRow(int)
{
    rmProperty_btn->setEnabled(true);
}

void PropertyEditor::checkPropertyIsRemovable()
{
    if (!cat_line->text().isEmpty() && !value_line->text().isEmpty())
        rmProperty_btn->setEnabled(true);
}


void PropertyEditor::accept()
{
	QDialog::accept();
}

void PropertyEditor::updateCategories()
{
	cat_list->clear();
	auto i = cat_list->currentIndex();

	for (auto &cat : Property::get_categories()) {
		cat_list->addItem(cat);
	}
	cat_list->setCurrentIndex(i);
}

} // namespace phonometrica
