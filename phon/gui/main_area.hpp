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
 * Purpose: panel which holds the viewer and the console in the main window.                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MAIN_AREA_HPP
#define PHONOMETRICA_MAIN_AREA_HPP

#include <QWidget>
#include <QSplitter>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/viewer.hpp>
#include <phon/gui/console.hpp>
#include <phon/gui/info_panel.hpp>
#include <phon/gui/splitter.hpp>

#define DEFAULT_CONSOLE_RATIO 0.8
#define DEFAULT_INFO_RATIO 0.8
#define DEFAULT_FILE_MANAGER_RATIO 0.17

namespace phonometrica {

class MainArea final : public QWidget
{
    Q_OBJECT

public:

    explicit MainArea(Runtime &rt, QWidget *parent = nullptr);

    Console *console() const { return m_console; }

    InfoPanel *infoPanel() const { return m_info_panel; }

    Viewer *viewer() const { return m_viewer; }

    void layout(double info_ratio, double console_ratio);

    std::pair<double,double> ratios();

    void focusConsole();

    void adjustSplitters();

signals:

public slots:

    void toggleConsole();

    void toggleInfo();

    void setDefaultLayout();

private:

    Runtime &rt;

    Splitter *vsplitter, *hsplitter;

    Viewer *m_viewer;

    Console *m_console;

    InfoPanel *m_info_panel;
};


} // phonometrica

#endif // PHONOMETRICA_MAIN_AREA_HPP
