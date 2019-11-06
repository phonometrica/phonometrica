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
 * Purpose: the viewer stores and manages views in the main window.                                                    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VIEWER_HPP
#define PHONOMETRICA_VIEWER_HPP

#include <QTabWidget>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/views/script_view.hpp>
#include <phon/gui/views/query_view.hpp>

namespace phonometrica {

class View;

class Viewer final : public QTabWidget
{
    Q_OBJECT

public:

    explicit Viewer(Runtime &rt, QWidget *parent = nullptr);

    void closeCurrentView();

    AutoSound getCurrentSound() const;

    AutoAnnotation getCurrentAnnotation() const;

    bool finalize();

signals:

	void statusMessage(const QString &);

public slots:

    void closeTab(int index);

    void viewModified();

    void viewSaved();

    void showDocumentation(const String &page);

    void openScriptView(AutoScript script);

    void openTableView(AutoDataset dataset);

    void openScript(const String &path);

    void newScript();

    void saveViews();

    void view(const std::shared_ptr<VFile> &file);

    void editAnnotation(AutoAnnotation, intptr_t, double, double);

    void closeAll();

private:

    void setStartView();

    void closeView(int index);

    void registerView(View *view);

    void unregisterView(View *view);

    void addView(View *view, const QString &label);

    Runtime &runtime;
};


} // phonometrica

#endif // PHONOMETRICA_VIEWER_HPP
