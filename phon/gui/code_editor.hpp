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
