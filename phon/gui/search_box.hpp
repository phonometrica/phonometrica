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
 * Created: 31/08/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Search box in a query editor. This is a virtual class, whose default implementation provides search func- *
 * tionality for single and multiple layer queries. Subclasses can override this behavior and present a custom layout,*
 * e.g. to implement query protocols.                                                                                 *
 *                                                                                                                    *
 **********************************************************************************************************************/

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

namespace phonometrica {

class LineEdit;


class SearchBox : public QGroupBox
{
	Q_OBJECT

public:

	SearchBox(QWidget *parent, const QString &title);

	void postInitialize();

	virtual AutoSearchNode buildSearchTree() = 0;

protected:

	virtual void setupUi() = 0;
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

	DefaultSearchBox(QWidget *parent);

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
