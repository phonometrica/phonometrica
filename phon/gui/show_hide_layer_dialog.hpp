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
 * Created: 22/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: dialog to show/hide annotation layers in an annotation view.                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SHOW_HIDE_LAYER_DIALOG_HPP
#define PHONOMETRICA_SHOW_HIDE_LAYER_DIALOG_HPP

#include <QDialog>
#include <phon/gui/check_list.hpp>

namespace phonometrica {

class ShowHideLayerDialog : public QDialog
{
	Q_OBJECT

public:

	ShowHideLayerDialog(const Array<String> &labels, const Array<bool> &visibility, QWidget *parent = nullptr);

	Array<int> selectedIndexes();

private:

	CheckListBox *list_box;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SHOW_HIDE_LAYER_DIALOG_HPP
