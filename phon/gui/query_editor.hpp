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
#include <phon/application/search/query.hpp>
#include <phon/gui/search_box.hpp>

class QGroupBox;
class QLineEdit;

namespace phonometrica {

class NumberEdit;
class BooleanEdit;
class CheckList;
class CheckListBox;


class QueryEditor final : public QDialog
{
	Q_OBJECT

public:

	QueryEditor(QWidget *parent, int context_length = 30);

signals:

	void queryReady(AutoQuery);

public slots:

	void accept();

private:

	void setupUi(int context_length);

	QWidget * createFileBox();

	QGroupBox *createProperties();

	AnnotationSet getAnnotations();

	AutoQuery buildQuery();

	Array<AutoMetaNode> getMetadata();

	AutoSearchNode getSearchTree();

	QLineEdit *query_name_edit;

	SearchBox *search_box;

	QWidget *main_widget;

	CheckList *selected_files_box;

	QComboBox *description_box;

	QLineEdit *description_line;

	Array<NumberEdit*> numeric_properties;

	Array<BooleanEdit*> boolean_properties;

	Array<CheckListBox*> text_properties;

	int properties_per_row = 3;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_EDITOR_HPP
