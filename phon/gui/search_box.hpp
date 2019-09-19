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
 * Created: 31/08/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Search box in a query editor. This is a virtual class, whose default implementation provides search func-  *
 * tionality for single and multiple layer queries. Subclasses can override this behavior and present a custom layout, *
 * e.g. to implement query protocols.                                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SEARCH_BOX_HPP
#define PHONOMETRICA_SEARCH_BOX_HPP

#include <QGroupBox>
#include <QLayout>
#include <phon/application/search/search_node.hpp>
#include <phon/application/search/query_parser.hpp>
#include <phon/utils/any.hpp>

class QPushButton;
class QTextEdit;
class QSpinBox;
class QComboBox;

namespace phonometrica {

class LineEdit;


class SearchBox : public QGroupBox
{
	Q_OBJECT

public:

	SearchBox(QWidget *parent, const QString &title, int context_length);

	void postInitialize();

	virtual AutoSearchNode buildSearchTree() = 0;

protected:

	virtual void setupUi() = 0;

	int context_length;
};


//----------------------------------------------------------------------------------------------------------------------


class DefaultSearchBox final : public SearchBox
{
	Q_OBJECT

	enum class Operator
	{
		And,
		Or,
		Custom
	};

public:

	DefaultSearchBox(QWidget *parent, int context_length);

	AutoSearchNode buildSearchTree() override;

protected:

	void setupUi() override;

private slots:

	void addSearchConstraint(bool dummy = true);

	void removeSearchConstraint(bool dummy);

private:

	using Type = QueryParser::Type;
	using Token = QueryParser::Token;

	template<class T>
	T *cast(QHBoxLayout *layout, int i)
	{
		auto w = dynamic_cast<T*>(layout->itemAt(i)->widget());
		assert(w); return w;
	}

	QSpinBox *createLayerSpinBox(int start = 0);

	LineEdit *createLayerEdit();

	void retainWhenHidden(QWidget *w, bool value);

	void changeLayerDisplay(int constraint_index, int selection);

	void updateFirstLayout();

	std::pair<bool,bool> getParentheses(int layout_index);


	AutoSearchNode parseConstraint(int layout_index);

	Type getOperator(int index);

	bool hasNotOperator(int index);

	Array<Token> getTokens();

	QVBoxLayout *main_layout;

	QPushButton *add_button, *remove_button;

	Array<QHBoxLayout*> constraint_layouts;

	Array<QComboBox*> relations;

	int constraint_count = 0;

	int open_parentheses, closed_parentheses;
};

} // namespace phonometrica

#endif // PHONOMETRICA_SEARCH_BOX_HPP
