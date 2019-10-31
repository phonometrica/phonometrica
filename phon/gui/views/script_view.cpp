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

bool ScriptView::save()
{
    if (m_script->modified() && !isEmpty()) {
        saveScript(false);
    }

	return true;
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

	    auto reply = QMessageBox::question(this, tr("Import file?"),
	                                       tr("Would you like to import this script into the current project?"),
	                                       QMessageBox::Yes|QMessageBox::No);

	    if (reply == QMessageBox::Yes)
	    {
		    auto project = Project::instance();
		    project->import_file(m_script->path());
		    emit project->notify_update();
	    }
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
    toolbar->addSeparator();

    auto comment_action = toolbar->addAction(QIcon(":/icons/toggle_off.png"), tr("Comment"));
    comment_action->setToolTip(tr("Comment selection (ctrl+/)"));
    comment_action->setShortcut(QKeySequence("ctrl+/"));

    auto uncomment_action = toolbar->addAction(QIcon(":/icons/toggle_on.png"), tr("Uncomment"));
    uncomment_action->setToolTip(tr("Uncomment selection (ctrl+shift+/)"));
	uncomment_action->setShortcut(QKeySequence("ctrl+shift+/"));

    connect(run_action, &QAction::triggered, this, &ScriptView::runScript);
    connect(save_action, &QAction::triggered, this, &ScriptView::saveScript);
    connect(comment_action, &QAction::triggered, this, &ScriptView::commentCode);
    connect(uncomment_action, &QAction::triggered, this, &ScriptView::uncommentCode);

#if PHON_MACOS || PHON_WINDOWS
	toolbar->setMaximumHeight(30);
#endif
#if PHON_MACOS
    toolbar->setStyleSheet("QToolBar{spacing:0px;}");
#endif

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

bool ScriptView::finalize()
{
	if (m_script->modified() && !isEmpty())
	{
		auto reply = QMessageBox::question(this, tr("Save script?"),
				tr("The current script has unsaved modifications. Would you like to save it?"),
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if (reply == QMessageBox::Yes)
		{
			if (!m_script->has_path())
			{
				QString dir = Settings::get_string(rt, "last_directory");
				auto path = QFileDialog::getSaveFileName(this, tr("Save script..."), dir, tr("Scripts (*.phon)"));

				if (path.isEmpty()) {
					return false; // cancelled
				}
				m_script->set_path(path, true);
			}

			m_script->set_content(m_editor->toPlainText());
			m_script->save();
		}
		else if (reply == QMessageBox::Cancel)
		{
			return false;
		}
	}

	return true;
}

void ScriptView::commentCode(bool)
{
	auto cursor = m_editor->textCursor();
	if (cursor.hasSelection())
	{
		auto lines = cursor.selection().toPlainText().split("\n");

		for (auto &ln : lines)
		{
			ln.prepend('#');
		}
		cursor.clearSelection();
		m_editor->insertPlainText(lines.join('\n'));
	}
}

void ScriptView::uncommentCode(bool)
{
	auto cursor = m_editor->textCursor();
	if (cursor.hasSelection())
	{
		auto lines = cursor.selection().toPlainText().split("\n");

		for (auto &ln : lines)
		{
			if (ln.startsWith('#')) ln.remove(0, 1);
		}
		cursor.clearSelection();
		m_editor->insertPlainText(lines.join('\n'));
	}
}

} // phonometrica
