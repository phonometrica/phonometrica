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

#include <QDesktopServices>
#include <QDebug>
#include <QMessageBox>
#include <phon/gui/viewer.hpp>
#include <phon/gui/views/view.hpp>
#include <phon/gui/views/sound_view.hpp>
#include <phon/gui/views/annotation_view.hpp>
#include <phon/gui/views/spreadsheet_view.hpp>
#include <phon/gui/views/start_view.hpp>
#include <phon/gui/console.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Viewer::Viewer(Runtime &rt, QWidget *parent) :
		QTabWidget(parent), runtime(rt)
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    setStartView();
}

void Viewer::setStartView()
{
    if (!runtime.is_text_mode())
    {
        addView(new StartView(nullptr, runtime), tr("Start"));
    }
}

void Viewer::closeTab(int i)
{
    this->closeView(i);

    if (this->count() == 0)
        setStartView();
}

void Viewer::viewModified()
{
    auto i = currentIndex();
    auto text = tabText(i);
    if (!text.endsWith('*')) text.append(" *");
    setTabText(i, text);
}

void Viewer::viewSaved()
{
    auto i = currentIndex();
    auto text = tabText(i);
    QString suffix(" *");
    if (text.endsWith(suffix))
    text.replace(suffix, QString());
    setTabText(i, text);
}

void Viewer::closeView(int index)
{
    View *view = qobject_cast<View *>(this->widget(index));
    if (view->finalize())
    {
	    removeTab(index);
	    delete view;
    }
}

void Viewer::registerView(View *view)
{
	connect(view, &View::statusMessage, this, &Viewer::statusMessage);
	connect(view, &View::sendCommand, runtime.console, &Console::execute);
    connect(view, &View::modified, this, &Viewer::viewModified);
    connect(view, &View::saved, this, &Viewer::viewSaved);
}

void Viewer::unregisterView(View *view)
{
	disconnect(view, &View::statusMessage, this, &Viewer::statusMessage);
	disconnect(view, &View::sendCommand, runtime.console, &Console::execute);
    disconnect(view, &View::modified, this, &Viewer::viewModified);
    disconnect(view, &View::saved, this, &Viewer::viewSaved);
}

void Viewer::addView(View *view, const QString &label)
{
	view->postInitialize();
    registerView(view);
    addTab(view, label);
    setCurrentWidget(view);
    view->makeFocused();
}

void Viewer::showDocumentation(const String &page)
{
	QString url("file:///");
	url.append(QString(page));

	if (!QDesktopServices::openUrl(url))
	{
		auto msg = QString("Cannot open page '%1'").arg(page);
		QMessageBox dlg(QMessageBox::Critical, "Error", msg);
		dlg.exec();
	}
}

void Viewer::openScriptView(AutoScript script)
{
    auto label = script->label();
    auto view = new ScriptView(runtime, std::move(script));
    addView(view, label);
}

void Viewer::openTableView(AutoDataset dataset)
{
	View *view = nullptr;
	auto label = dataset->label();

	if (dataset->is_query_table())
	{
		auto query_view = new QueryView(this, runtime, std::dynamic_pointer_cast<QueryTable>(dataset));
		connect(query_view, &QueryView::openAnnotation, this, &Viewer::editAnnotation);
		view = query_view;
	}
	else if (dataset->is_spreadsheet())
	{
		auto spreadsheet_view = new SpreadsheetView(this, runtime, std::dynamic_pointer_cast<Spreadsheet>(dataset));
		view = spreadsheet_view;
	}
	else
	{
		QString msg = QString("View for %1 files is not implemented").arg(dataset->class_name());
		QMessageBox dlg(QMessageBox::Information, tr("Cannot open view"), msg);
		dlg.exec();
		return;
	}

	addView(view, label);
}

void Viewer::openScript(const String &path)
{
    auto file = Project::instance()->get(path);

    if (file)
    {
        auto script = std::dynamic_pointer_cast<Script>(file);

        if (script)
        {
            openScriptView(downcast<Script>(script));
            return;
        }
    }

    QString message = QString("File `%1` does not seem to be a script").arg(path);
    QMessageBox dlg(QMessageBox::Critical, tr("Cannot load script"), message);
    dlg.exec();
}

void Viewer::newScript()
{
    auto script = std::make_shared<Script>(nullptr);
    Project::updated();
    openScriptView(std::move(script));
}

void Viewer::saveViews()
{
    for (int i = 0; i < count(); i++)
    {
        auto view = static_cast<View*>(widget(i));
        view->save();
    }
}

void Viewer::view(const std::shared_ptr<VFile> &file)
{
    if (file->is_script())
    {
        auto script = downcast<Script>(file);
        openScriptView(std::move(script));
    }
    else if (file->is_sound())
    {
        auto sound = downcast<Sound>(file);
        auto label = sound->label();
        addView(new SoundView(runtime, std::move(sound)), label);
    }
    else if (file->is_annotation())
    {
        auto annot = downcast<Annotation>(file);

        if (annot->has_sound())
        {
            auto label = annot->label();
			PHON_LOG("Creating annotation in viewer");
            addView(new AnnotationView(runtime, std::move(annot)), label);
			PHON_LOG("Annotation created");
        }
        else
        {
            QString msg("You must first bind this annotation to a sound file in order to be able to view it.");
            QMessageBox dlg(QMessageBox::Critical, tr("Cannot open view"), msg);
            dlg.exec();
        }
    }
    else if (file->is_dataset())
    {
    	openTableView(downcast<Dataset>(file));
    }
    else
    {
        QString msg = QString("View for %1 files is not implemented").arg(file->class_name());
        QMessageBox dlg(QMessageBox::Information, tr("Cannot open view"), msg);
        dlg.exec();
    }
}

void Viewer::closeAll()
{
	saveViews();
	int n = this->count();

	for (int i = n - 1; i >= 0; --i) {
		this->removeTab(i);
	}
	setStartView();
}

void Viewer::editAnnotation(AutoAnnotation annot, intptr_t layer, double from, double to)
{
    auto label = annot->label();
	PHON_LOG("Creating annotation view");
    auto view = new AnnotationView(runtime, std::move(annot), this);
	PHON_LOG("Annotation view created");
    addView(view, label);
	view->openSelection(layer, from, to);
}

void Viewer::closeCurrentView()
{
	closeTab(currentIndex());
}

AutoSound Viewer::getCurrentSound() const
{
	auto view = qobject_cast<SpeechView*>(this->widget(currentIndex()));
	return view ? view->sound() : AutoSound();
}

AutoAnnotation Viewer::getCurrentAnnotation() const
{
	auto view = qobject_cast<AnnotationView*>(this->widget(currentIndex()));
	return view ? view->annotation() : AutoAnnotation();
}

bool Viewer::finalize()
{
	for (int i = 0; i < this->count(); i++)
	{
		setCurrentIndex(i);
		auto view = qobject_cast<View*>(widget(i));
		if (!view->finalize()) return false;
	}

	return true;
}

double Viewer::getWindowDuration() const
{
	auto view = qobject_cast<SpeechView*>(this->widget(currentIndex()));
    return view ? view->getWindowDuration() : 0;
}

double Viewer::getSelectionDuration() const
{
	auto view = qobject_cast<SpeechView*>(this->widget(currentIndex()));
    return view ? view->getSelectionDuration() : 0;
}

} // phonometrica
