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
 * Created: 23/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Dialog to create a new layer.                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_NEW_LAYER_DIALOG_HPP
#define PHONOMETRICA_NEW_LAYER_DIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <phon/string.hpp>

namespace phonometrica {

class NewLayerDialog final : public QDialog
{
public:

	NewLayerDialog(QWidget *parent, int layer_count);

	String layerName() const { return name_edit->text(); }

	bool hasInstants() const { return type_combo->currentIndex() == 1; }

	int layerIndex() const { return index_box->value(); }

private:

	QLineEdit *name_edit;

	QComboBox *type_combo;

	QSpinBox *index_box;
};

} // namespace phonometrica

#endif // PHONOMETRICA_NEW_LAYER_DIALOG_HPP
