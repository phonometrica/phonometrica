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
 * Created: 16/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QtGlobal>
#include <QPainter>
#include <QTextBlock>
#include <phon/gui/code_editor.hpp>


// This is based on code from Qt's documentation.
// https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html
//
// Copyright (C) 2018 The Qt Company Ltd.
//
// Redistribution and use in source and binary forms, with or without
//		modification, are permitted provided that the following conditions are
//		met:
// * Redistributions of source code must retain the above copyright
//		notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//		notice, this list of conditions and the following disclaimer in
//		the documentation and/or other materials provided with the
//		distribution.
// * Neither the name of The Qt Company Ltd nor the names of its
//		contributors may be used to endorse or promote products derived
//		from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//		LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//		LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//		THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//		(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace phonometrica {

phonometrica::LineNumberArea::LineNumberArea(CodeEditor *editor) :
        QWidget(editor)
{
    codeEditor = editor;
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlight()));

    updateLineNumberAreaWidth(0);
    highlight();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 3;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    int space = 3 + fontMetrics().width('9') * digits;
#else
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
#endif
    
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlight()
{
    QColor color = QColor(Qt::blue).lighter(190);
    color.setAlpha(80);
    highlightCurrentLine(color);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::transparent);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            number.append(' ');
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::highlightError(int num)
{
    QTextCursor cursor(document()->findBlockByLineNumber(num));
    setTextCursor(cursor);
    QColor color = QColor(Qt::red).lighter(160);
    color.setAlpha(80);
    highlightCurrentLine(color);
}

void CodeEditor::highlightCurrentLine(const QColor &color)
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(color);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
	{
		// Add the same level of indentation as the previous line.
		auto pos = textCursor().position();
		auto lines = this->toPlainText().left(pos).split('\n');
		if (!lines.isEmpty())
		{
			auto &ln = lines.last();
			QString spacing;
			int count = 0;

			for (QChar c : ln)
			{
				if (c.isSpace()) count++;
				else break;
			}
			// First insert the new line.
			QPlainTextEdit::keyPressEvent(event);
			// Then insert the spacing.
			textCursor().insertText(ln.left(count));
			if (ln.endsWith(" then") || ln.endsWith(" do") || ln.trimmed().startsWith("function ")) {
				textCursor().insertText("\t");
			}
		}
	}
	else
	{
		QPlainTextEdit::keyPressEvent(event);
	}
}

} // namespace phonometrica
