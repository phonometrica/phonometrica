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
 * Created: 08/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: editor for VFile properties.                                                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef METADATAEDITOR_H
#define METADATAEDITOR_H

#include <QDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>

#include <phon/gui/property_table.hpp>
#include <phon/application/annotation.hpp>
#include <phon/application/sound.hpp>
#include <phon/application/script.hpp>

namespace phonometrica {


class MetadataEditor final : public QDialog
{
    Q_OBJECT

public:

    MetadataEditor(QWidget *parent, VFileList files);

private slots:

    void accept() override;

    void addProperty();

    void removeProperty();

    void updateValues();

    void updateSelectedValue();

    void propertyContentEdited();

    void selectRow(int);

    void checkPropertyIsRemovable();

private:

    QVBoxLayout *layout;
    QPushButton *addProperty_btn, *rmProperty_btn, *ok_btn;
    VFileList m_files;
    PropertyTable *property_table;
    QListWidget *cat_list, *value_list;
    QLineEdit *cat_line, *value_line;

    void setupUi();
};

} // namespace phonometrica

#endif // METADATAEDITOR_H
