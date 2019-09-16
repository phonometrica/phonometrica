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
 * Created: 24/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QVBoxLayout>
#include <phon/gui/popup_text_editor.hpp>
#include "popup_text_editor.hpp"


namespace phonometrica {

PopupTextEditor::PopupTextEditor(const QString &text, QPoint pos, QWidget *parent) :
    QDialog(parent)
{
    auto layout = new QVBoxLayout;
    edit = new EventLine;
    edit->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(edit);
    setLayout(layout);
    edit->setText(text);
    int xdelta = 400;
    int ydelta = 50;
    int x = pos.x() - xdelta;
    ypos = pos.y() - ydelta*2 - 10;
    setGeometry(x, ypos, xdelta*2, ydelta*2);
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
    setWindowOpacity(0.95);

    connect(edit, &EventLine::accepted, this, &PopupTextEditor::accept);
}

void PopupTextEditor::paintEvent(QPaintEvent *event)
{
    QPalette pal = palette();
    auto col = QColor(Qt::blue).lighter(160);
    col.setAlpha(200);
    pal.setColor(QPalette::Background, col);
    setAutoFillBackground(true);
    setPalette(pal);

    QWidget::paintEvent(event);
}

int PopupTextEditor::yshift() const
{
    return y() - ypos;
}


} // namespace phonometrica