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
 * Purpose: file manager (left panel in the main window).                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FILE_MANAGER_HPP
#define PHONOMETRICA_FILE_MANAGER_HPP

#include <QFrame>
#include <QLabel>
#include <QDropEvent>
#include <QPushButton>
#include <QStatusBar>
#include <phon/gui/project_ctrl.hpp>

namespace phonometrica {

class FileManager final : public QFrame
{
    Q_OBJECT

public:

    explicit FileManager(Runtime &rt, QWidget *parent = nullptr);

    QPushButton *console_button, *info_button;

    // Update buttons and toggle panel.
    void consoleClicked(bool);

    void infoClicked(bool);

    void initStatusBar(QStatusBar *status_bar);

    ProjectCtrl *tree() { return project_ctrl; }

signals:

    void toggleConsole();

    void toggleInfo();

public slots:

    // Let the file manager update the buttons when the panels are toggled.
    void updateConsoleStatus(bool);

    void updateInfoStatus(bool);

    void refreshProject();

    void refreshLabel();

private:

    Runtime &rt;

    ProjectCtrl *project_ctrl;

#ifdef Q_OS_MAC
    QLabel *label;
#endif

    bool info_shown = true;

    bool console_shown = true;
};

} // phonometrica


#endif // PHONOMETRICA_FILE_MANAGER_HPP
