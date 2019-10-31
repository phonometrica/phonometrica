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
 * Created: 30/08/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: a query editor is a dialog which allows the user to create and edit queries.                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_EDITOR_HPP
#define PHONOMETRICA_QUERY_EDITOR_HPP

#include <QDialog>
#include <QDialogButtonBox>
#include <phon/runtime/runtime.hpp>
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

	enum class Type
	{
		Default,
		CodingProtocol,
		PitchMeasurement,
		IntensityMeasurement,
		FormantMeasurement
	};

	QueryEditor(Runtime &rt, AutoProtocol protocol, QWidget *parent, Type type, int context_length = 30);

	void resurrect();

signals:

	void queryReady(AutoQuery);

public slots:

	void accept();

private slots:

	void showHelp(bool);

private:

	void setupUi(int context_length);

	QWidget * createFileBox();

	QGroupBox *createProperties();

	AnnotationSet getAnnotations();

	AutoQuery buildQuery();

	Array<AutoMetaNode> getMetadata();

	AutoSearchNode getSearchTree();

	Runtime &runtime;

	AutoProtocol protocol;

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

	Type type;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_EDITOR_HPP
