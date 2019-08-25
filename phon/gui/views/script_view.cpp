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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QtGlobal>
#include <QVBoxLayout>
#include <QToolBar>
#include <QFileDialog>
#include <QTextDocumentFragment>
#include <QMessageBox>
#include <phon/file.hpp>
#include <phon/regex.hpp>
#include <phon/gui/views/script_view.hpp>
#include <phon/gui/console.hpp>
#include <phon/gui/font.hpp>
#include <phon/application/settings.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

ScriptView::ScriptView(Runtime &rt, std::shared_ptr<Script> script, QWidget *parent) :
    View(parent), m_script(std::move(script)), rt(rt)
{
    auto font = get_monospace_font();
    QFontMetrics metrics(font);
    m_editor = new CodeEditor(this);
    m_editor->setFont(font);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    m_editor->setTabStopDistance(metrics.width(' ') * 4);
#endif
    m_highlighter = new Highlighter(m_editor->document());
    auto toolbar = createToolbar();

    auto layout = new QVBoxLayout;
    layout->addWidget(toolbar);
    layout->addWidget(m_editor);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    loadScript();

    connect(m_editor, &QPlainTextEdit::textChanged, this, &ScriptView::scriptChanged);
}

void ScriptView::save()
{
    if (m_script->modified() && !isEmpty()) {
        saveScript(false);
    }
}

void ScriptView::runScript(bool)
{
    auto cursor = m_editor->textCursor();

    try
    {
        QString text = cursor.hasSelection() ? cursor.selection().toPlainText() : m_editor->toPlainText();
        rt.console->runCommand(text, true);
    }
    catch (std::exception &e)
    {
        String msg = e.what();
        Regex re("^.*\\s*at \\[string\\]:([0-9]+)");

        for (auto &ln : msg.split("\n"))
        {
            if (re.match(ln))
            {
                bool ok;
                auto num = (int) re.capture(1).to_int(&ok);
                int line_no = 0;

                if (cursor.hasSelection())
                {
                    cursor.setPosition(cursor.selectionStart());
                    line_no = cursor.blockNumber();
                }
                line_no += num - 1;
                m_editor->highlightError(line_no);

                return;
            }
        }
    }
}

void ScriptView::scriptChanged()
{
    m_script->set_pending_modifications();
    emit modified();
}

void ScriptView::saveScript(bool)
{
    if (!m_script->has_path())
    {
        QString dir = Settings::get_string(rt, "last_directory");
        auto path = QFileDialog::getSaveFileName(this, tr("Save script..."), dir, tr("Scripts (*.phon)"));

        if (path.isEmpty()) {
            return; // cancelled
        }
        m_script->set_path(path, true);
        Project::instance()->register_file(path, m_script);
    }

    m_script->set_content(m_editor->toPlainText());
    m_script->save();

    emit saved();
}

void ScriptView::loadScript()
{
    auto path = m_script->path();

    if (filesystem::exists(path))
    {
        auto code = File::read_all(path);
        m_editor->setPlainText(code);
    }
}

void ScriptView::showError(const QString &msg)
{
    QMessageBox dlg(QMessageBox::Critical, tr("Error"), msg);
    dlg.exec();
}

QToolBar *ScriptView::createToolbar()
{
    auto toolbar = new QToolBar;
    auto save_action = toolbar->addAction(QIcon(":/icons/save.png"), tr("Save"));
    save_action->setToolTip(tr("Save script (ctrl+s)"));
    save_action->setShortcut(QKeySequence("ctrl+s"));
    toolbar->addSeparator();

    auto run_action = toolbar->addAction(QIcon(":/icons/start.png"), tr("Run"));
    run_action->setToolTip(tr("Run script or selection (ctrl+r)"));
    run_action->setShortcut(QKeySequence("ctrl+r"));

    connect(run_action, &QAction::triggered, this, &ScriptView::runScript);
    connect(save_action, &QAction::triggered, this, &ScriptView::saveScript);

    return toolbar;
}

bool ScriptView::isEmpty()
{
    return m_editor->document()->isEmpty();
}

void ScriptView::makeFocused()
{
    auto tc = m_editor->textCursor();
    tc.setPosition(0);
    m_editor->setTextCursor(tc);
    m_editor->setFocus();
}

} // phonometrica
