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
 * Purpose: script editor.                                                                                            *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef SCRIPT_VIEW_HPP
#define SCRIPT_VIEW_HPP

#include <QPlainTextEdit>
#include <phon/gui/views/view.hpp>
#include <phon/gui/highlighter.hpp>
#include <phon/gui/code_editor.hpp>
#include <phon/application/script.hpp>
#include <phon/runtime/runtime.hpp>

class QToolBar;

namespace phonometrica {

class Console;


class ScriptView final : public View
{
    Q_OBJECT

public:

    explicit ScriptView(Runtime &rt, std::shared_ptr<Script> script, QWidget *parent = nullptr);

    void save() override;

    void makeFocused() override;

private slots:

    void runScript(bool);

    void scriptChanged();

    void saveScript(bool);

private:

    void loadScript();

    void showError(const QString &msg);

    QToolBar *createToolbar();

    bool isEmpty();

    std::shared_ptr<Script> m_script;

    Runtime &rt;

    CodeEditor *m_editor;

    Highlighter *m_highlighter;
};


} // phonometrica

#endif // SCRIPT_VIEW_HPP
