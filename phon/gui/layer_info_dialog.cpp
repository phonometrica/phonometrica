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
 * Created: 26/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <phon/gui/layer_info_dialog.hpp>

namespace phonometrica {

LayerInfoDialog::LayerInfoDialog(QWidget *parent, AutoLayer layer) :
	QDialog(parent, Qt::Window|Qt::FramelessWindowHint), layer(std::move(layer))
{
	info_label = new QLabel(label());
	auto vl = new QVBoxLayout;
	vl->addWidget(info_label);
	setLayout(vl);
}

QString LayerInfoDialog::label() const
{
	QString text = QString("<b>Layer %1</b>").arg(layer->index);
	if (!layer->label.empty()) text.append(QString(": %1").arg(layer->label));
	text.append("<br/><b>Event type</b>: ");
	if (layer->has_instants) {
		text.append("instants");
	}
	else {
		text.append("intervals");
	}
	text.append("<br/><b>Number of events</b>: ");
	text.append(QString::number(layer->count()));

	return text;
}

QString LayerInfoDialog::updateInfo()
{
	auto text = label();
	info_label->setText(text);

	return text;
}
} // namespace phonometrica