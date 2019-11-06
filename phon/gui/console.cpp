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
    setPrompt();
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

void Console::execute(const String &cmd)
{
	insertPlainText(cmd);
	runCommand(cmd, false);
}


} // phonometrica
