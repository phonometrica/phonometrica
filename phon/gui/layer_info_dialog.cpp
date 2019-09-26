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