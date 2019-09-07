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
 * Purpose: Scripting console, located at the bottom of the main window.                                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <QPlainTextEdit>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

class Console final : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console(Runtime &rt, QWidget *parent = nullptr);

    void runCommand(QString cmd, bool from_script);

signals:

    void shown(bool);

public slots:

    void print(const String &str);

	void warn(const String &str);

	void setPrompt();

protected:

    void keyPressEvent(QKeyEvent *e) override;

private:

    void interpretCommand(const QString &command, bool from_script);

    void printError(const QString &msg);

    void addCommand(const String &cmd);

    void setCurrentCommand(const String &cmd);

    QTextBlock currentBlock();

    void moveToEnd();

    Runtime &runtime;

    const QString prompt;

    Array<String> history;

    intptr_t current_cmd = 0;
};

} // phonometrica

#endif // CONSOLE_HPP
