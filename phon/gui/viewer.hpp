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

    ~Viewer() override;

    void closeCurrentView();

signals:

public slots:

    void closeTab(int index);

    void viewModified();

    void viewSaved();

    void showDocumentation(const String &page);

    void openScriptView(AutoScript script);

    void openTableView(AutoDataset table);

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
