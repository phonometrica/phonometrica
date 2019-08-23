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
 * Purpose: panel which holds the viewer and the console in the main window.                                          *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef MAIN_AREA_HPP
#define MAIN_AREA_HPP

#include <QWidget>
#include <QSplitter>
#include <phon/runtime/environment.hpp>
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

    explicit MainArea(Environment &env, QWidget *parent = nullptr);

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

    Environment &env;

    Splitter *vsplitter, *hsplitter;

    Viewer *m_viewer;

    Console *m_console;

    InfoPanel *m_info_panel;
};


} // phonometrica

#endif // MAIN_AREA_HPP
