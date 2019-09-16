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
 * Created: 18/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QPainter>
#include <phon/gui/sound_zoom.hpp>
#include "sound_zoom.hpp"


namespace phonometrica {

SoundZoom::SoundZoom(QWidget *parent) :
    QWidget(parent)
{
    setFixedHeight(40);
    from = to = 0.0;
}

void SoundZoom::setXAxisSelection(double from, double to)
{
    this->from = from;
    this->to = to;
    this->repaint();
}

void SoundZoom::paintEvent(QPaintEvent *event)
{
    if (from == 0.0 && to == 0.0) {
        return;
    }

    QPainterPath path;
    path.moveTo(QPointF(0.0, 0.0));
    path.lineTo(QPointF(from, height()));
    path.lineTo(QPointF(to, height()));
    path.lineTo(QPointF(width(), 0.0));
    path.lineTo(QPointF(0.0, 0.0));

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor col(Qt::blue);
    col = col.lighter(160);
    col.setAlpha(60);
    painter.fillPath(path, col);

    QWidget::paintEvent(event);
}

void SoundZoom::hideSelection()
{
    from = to = 0.0;
    repaint();
}
} // namespace phonometrica