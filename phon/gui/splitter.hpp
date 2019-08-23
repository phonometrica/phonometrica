/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: splitter with a more native appearance on macOS.                                                           *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef SPLITTERS_HPP
#define SPLITTERS_HPP

#include <QtWidgets>

#ifdef Q_OS_MAC
class MacSplitterHandle : public QSplitterHandle
{
	//Q_OBJECT

public:

	MacSplitterHandle(Qt::Orientation orientation, QSplitter *parent);

	~MacSplitterHandle() override = default;

	void paintEvent(QPaintEvent *) override;

	QSize sizeHint() const override;
};
#endif

class Splitter : public QSplitter
{
public:
	Splitter(Qt::Orientation orientation, QWidget *parent = 0);
	QSplitterHandle *createHandle();

protected:
	void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // SPLITTERS_HPP
