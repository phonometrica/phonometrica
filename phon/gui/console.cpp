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

#include <QTextBlock>
#include <phon/gui/console.hpp>
#include <phon/gui/font.hpp>

namespace phonometrica {

static const size_t COMMAND_MAX = 30;

Console::Console(Runtime &rt, QWidget *parent) :
		QPlainTextEdit(parent), runtime(rt), prompt(">> ")
{
	runtime.initialize_script = [=]() {
		this->dirty = false;
	};

	runtime.finalize_script = [=]() {
		if (this->dirty) this->setPrompt();
	};

    auto font = get_monospace_font();
    setFont(font);

    // Redirect stdout to console.
    if (!rt.is_text_mode())
    {
        rt.print = [&](const String &s) {
            this->print(s);
            this->dirty = true;
        };

        // Clear console.
        auto clear = [=](Runtime &) { this->clear(); };
        rt.new_native_function(clear, "clear", 0);
        rt.def_global("clear", PHON_DONTENUM);
    }
    rt.console = this;
    setPrompt();
}

void Console::runCommand(QString cmd, bool from_script)
{
    if (!this->isVisible())
    {
        this->show();
        emit shown(false);
    }
    interpretCommand(cmd, from_script);
}

void Console::interpretCommand(const QString &command, bool from_script)
{
    if (!from_script) {
        addCommand(command);
    }

    try
    {
        auto s = command.toStdString();

        runtime.load_string("[string]", command);
        runtime.push_null();
        runtime.call(0);
        if (runtime.is_defined(-1) && !from_script)
        {
            print(runtime.to_string(-1));
        }
        runtime.pop(1);
    }
    catch (std::exception &e)
    {
        auto error = QString::fromUtf8(e.what());
        printError(error);

        // Let the script view proces the error
        if (from_script) {
            throw;
        }
    }

    setPrompt();
}

void Console::print(const String &str)
{
    moveToEnd();
    textCursor().insertText("\n");
    textCursor().insertText(str);
}

void Console::setPrompt()
{
    appendPlainText(">> ");
}

void Console::printError(const QString &msg)
{
    QString error = QString("<b><font color=\"red\">") + msg + String("</font></b>");
    appendHtml(error);
}

void Console::keyPressEvent(QKeyEvent *e)
{
    auto key = e->key();

    if (key == Qt::Key_Return)
    {
        moveToEnd();
        auto cmd = currentBlock().text();
        if (cmd.startsWith(prompt)) {
            cmd = cmd.mid(3);
        }
        e->accept();
        runCommand(cmd, false);
    }
    else if (key == Qt::Key_Up)
    {
        if (!history.empty())
        {
            if (current_cmd == 1)
            {
                setCurrentCommand(history.front());
            }
            else
            {
                auto cmd = history[--current_cmd];
                setCurrentCommand(cmd);
            }
        }
        e->ignore();
    }
    else if (key == Qt::Key_Down)
    {
        if (!history.empty())
        {
            if (current_cmd >= history.size())
            {
                current_cmd = history.size() + 1;
                setCurrentCommand(String());
            }
            else
            {
                auto cmd = history[++current_cmd];
                setCurrentCommand(cmd);
            }
        }
        e->ignore();
    }
    else
    {
        if (key == Qt::Key_Backspace && currentBlock().text() == prompt)
        {
            e->ignore();
        }
        else
        {
            QPlainTextEdit::keyPressEvent(e);
        }
    }
}

void Console::addCommand(const String &cmd)
{
    if (history.empty())
    {
        history.append(cmd);
    }
    else if (history.back() != cmd)
    {
        if (history.size() == COMMAND_MAX) {
            history.pop_first();
        }
        history.append(cmd);
    }

    current_cmd = history.size() + 1;
}

void Console::setCurrentCommand(const String &cmd)
{
    auto b = currentBlock();

    if (b.isValid())
    {
        QTextCursor cursor(b);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
    }
    appendPlainText(prompt + cmd);
}

QTextBlock Console::currentBlock()
{
    auto ln = document()->lineCount();
    return document()->findBlockByLineNumber(ln-1);
}

void Console::moveToEnd()
{
    auto c = textCursor();
    c.movePosition(QTextCursor::EndOfLine);
    setTextCursor(c);
}

void Console::warn(const String &str)
{
	printError(str);
}


} // phonometrica
