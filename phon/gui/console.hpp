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
 * Purpose: Scripting console, located at the bottom of the main window.                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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

    // Flag used to indicate that text has been printed to the console.
    bool dirty = false;
};

} // phonometrica

#endif // CONSOLE_HPP
