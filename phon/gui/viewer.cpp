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

#include <QDesktopServices>
#include <QMessageBox>
#include <phon/gui/viewer.hpp>
#include <phon/gui/views/view.hpp>
#include <phon/gui/views/sound_view.hpp>
#include <phon/gui/views/annotation_view.hpp>
#include <phon/gui/views/start_view.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Viewer::Viewer(Runtime &rt, QWidget *parent) :
    QTabWidget(parent), rt(rt)
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    setStartView();
}

void Viewer::setStartView()
{
    if (!rt.is_text_mode())
    {
        addView(new StartView(nullptr, rt), tr("Start"));
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
    view->save();
    removeTab(index);
    delete view;
}

void Viewer::registerView(View *view)
{
    connect(view, &View::modified, this, &Viewer::viewModified);
    connect(view, &View::saved, this, &Viewer::viewSaved);
}

void Viewer::unregisterView(View *view)
{
    disconnect(view, &View::modified, this, &Viewer::viewModified);
    disconnect(view, &View::saved, this, &Viewer::viewSaved);
}

void Viewer::addView(View *view, const QString &label)
{
    view->post_initialize();
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

void Viewer::openScriptView(std::shared_ptr<Script> script)
{
    auto label = script->label();
    auto view = new ScriptView(rt, std::move(script));
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
    auto parent = Project::instance()->scripts();
    auto script = std::make_shared<Script>(parent.get());
    // Don't mutate the parent because we don't want empty scripts to be remembered.
    parent->append(script, false);
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
        addView(new SoundView(rt, std::move(sound)), label);
    }
    else if (file->is_annotation())
    {
        auto annot = downcast<Annotation>(file);

        if (annot->has_sound())
        {
            auto label = annot->label();
            addView(new AnnotationView(rt, std::move(annot)), label);
        }
        else
        {
            QString msg("You must first bind this annotation to a sound file in order to be able to view it.");
            QMessageBox dlg(QMessageBox::Critical, tr("Cannot open view"), msg);
            dlg.exec();
        }
    }
    else
    {
        QString msg = QString("View for %1 files is not implemented").arg(file->class_name());
        QMessageBox dlg(QMessageBox::Information, tr("Cannot open view"), msg);
        dlg.exec();
    }
}


} // phonometrica
