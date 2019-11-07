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
 * Created: 31/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: query editor search box for formant analysis.                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FORMANT_SEARCH_BOX_HPP
#define PHONOMETRICA_FORMANT_SEARCH_BOX_HPP

#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QStackedLayout>
#include <phon/application/search/formant_query_settings.hpp>
#include <phon/gui/line_edit.hpp>
#include <phon/gui/search_box.hpp>

namespace phonometrica {

class FormantSearchBox final : public DefaultSearchBox
{
	Q_OBJECT

public:

	FormantSearchBox(QWidget *parent);

	AutoSearchNode buildSearchTree() override;

	AutoQuerySettings getSettings() const override;

protected:

	Query::Type getType() const override;

private slots:

	void changeMethod(int index);

private:

	void setupUi(Runtime &rt) override;

	QStackedLayout *stack;

	QRadioButton *parametric_button;

	QSpinBox *formant_spinbox, *lpc_spinbox;

	QCheckBox *bark_checkbox, *erb_checkbox, *bandwidth_checkbox, *surrounding_checkbox, *labels_checkbox;

	QLineEdit *max_freq_edit, *win_edit, *max_bw_edit;

	QLineEdit *param_min_freq_edit, *param_max_freq_edit, *param_step_freq_edit;

	LineEdit *labels_edit;

	QSpinBox *param_lpc_min_spinbox, *param_lpc_max_spinbox;

};

} // namespace phonometrica



#endif // PHONOMETRICA_FORMANT_SEARCH_BOX_HPP
