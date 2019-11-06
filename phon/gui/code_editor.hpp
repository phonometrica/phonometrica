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
 * Created: 16/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Code editor for Phonometrica scripts.                                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CODE_EDITOR_HPP
#define PHONOMETRICA_CODE_EDITOR_HPP

#include <QPlainTextEdit>

namespace phonometrica {

class CodeEditor;


class LineNumberArea : public QWidget
{
public:

    LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:

    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CodeEditor final : public QPlainTextEdit
{
    Q_OBJECT

public:

    CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();

    void highlightError(int num);

protected:

    void resizeEvent(QResizeEvent *event) override;

	void keyPressEvent(QKeyEvent *event) override;

private slots:

    void updateLineNumberAreaWidth(int newBlockCount);

    void highlight();

    void updateLineNumberArea(const QRect &, int);

private:

    void highlightCurrentLine(const QColor &color);

    LineNumberArea *lineNumberArea;

};


} // namespace phonometrica

#endif // PHONOMETRICA_CODE_EDITOR_HPP
