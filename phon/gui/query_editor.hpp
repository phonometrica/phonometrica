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
 * Created: 30/08/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: a query editor is a dialog which allows the user to create and edit queries.                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_EDITOR_HPP
#define PHONOMETRICA_QUERY_EDITOR_HPP

#include <QDialog>
#include <QDialogButtonBox>
#include <phon/gui/search_box.hpp>

class QGroupBox;

namespace phonometrica {

class NumberEdit;
class BooleanEdit;
class CheckListBox;

class QueryEditor final : public QDialog
{
	Q_OBJECT

public:

	QueryEditor(QWidget *parent);

public slots:

	void accept();

private:

	void setupUi();

	QWidget * createFileBox();

	SearchBox *createSearchBox();

	QGroupBox *createProperties();

	SearchBox *search_box;

	QWidget *main_widget;

	Array<NumberEdit*> numeric_properties;

	Array<BooleanEdit*> boolean_properties;

	Array<CheckListBox*> text_properties;

	int properties_per_row = 3;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_EDITOR_HPP
