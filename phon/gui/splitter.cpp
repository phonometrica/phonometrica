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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/splitter.hpp>

#ifdef Q_OS_MAC
MacSplitterHandle::MacSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
: QSplitterHandle(orientation, parent) {   }

// Paint the horizontal handle as a gradient, paint
// the vertical handle as a line.
void MacSplitterHandle::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	QColor topColor(145, 145, 145);
	QColor bottomColor(142, 142, 142);
	QColor gradientStart(252, 252, 252);
	QColor gradientStop(223, 223, 223);

	if (orientation() == Qt::Vertical) {
		painter.setPen(topColor);
		painter.drawLine(0, 0, width(), 0);
		painter.setPen(bottomColor);
		painter.drawLine(0, height() - 1, width(), height() - 1);

		QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height() -3));
		linearGrad.setColorAt(0, gradientStart);
		linearGrad.setColorAt(1, gradientStop);
		painter.fillRect(QRect(QPoint(0,1), size() - QSize(0, 2)), QBrush(linearGrad));
	} else {
		painter.setPen(topColor);
		painter.drawLine(0, 0, 0, height());
	}
}

QSize MacSplitterHandle::sizeHint() const
{
	QSize parent = QSplitterHandle::sizeHint();
	if (orientation() == Qt::Vertical) {
		return parent + QSize(0, 3);
	} else {
		return QSize(1, parent.height());
	}
}
#endif

Splitter::Splitter(Qt::Orientation orientation, QWidget *parent) : QSplitter(orientation, parent)
{

}

QSplitterHandle *Splitter::createHandle()
{
#ifdef Q_OS_MAC
	return new MacSplitterHandle(orientation(), this);
#else
	return new QSplitterHandle(orientation(), this);
#endif
}

void Splitter::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->ignore();
}
