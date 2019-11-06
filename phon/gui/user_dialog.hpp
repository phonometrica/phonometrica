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
 * Created: 12/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: let users create a dialog from a JSON object.                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_USER_DIALOG_HPP
#define PHONOMETRICA_USER_DIALOG_HPP

#include <vector>
#include <QDialog>
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/file_selector.hpp>
#include <phon/gui/check_list.hpp>
#include <phon/third_party/json.hpp>

namespace phonometrica {

using Json = nlohmann::json;

class UserDialog final : public QDialog
{
	Q_OBJECT

public:

	UserDialog(Runtime &rt, const String &str, QWidget *parent = nullptr);

	String get();

private:

	bool parse(const String &str);

	bool parse(Json js);

	void parseItem(Json item);

	void addButtonBox(bool yes_no);

	QString getName(Json js);

	void addLabel(Json item);

	void addCheckBox(Json item);

	void addComboBox(Json item);

	void addLineEdit(Json item);

	void addCheckList(Json item);

	void addRadioButtons(Json item);

	void addPushButton(Json item);

	void addFileSelector(Json item);

	void addContainer(Json item);

	void addSpacing(Json item);

	Runtime &runtime;

	QBoxLayout *main_layout;

	QBoxLayout *layout = nullptr;

	std::vector<QCheckBox*> check_boxes;

	std::vector<QComboBox*> combo_boxes;

	std::vector<QLineEdit*> line_edits;

	std::vector<CheckList*> check_lists;

	std::vector<QButtonGroup*> radio_buttons;

	std::vector<FileSelector*> file_selectors;
};

} // namespace phonometrica

#endif // PHONOMETRICA_USER_DIALOG_HPP
