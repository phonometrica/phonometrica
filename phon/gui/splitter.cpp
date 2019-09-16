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
