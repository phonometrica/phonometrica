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
