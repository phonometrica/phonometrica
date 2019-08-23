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

#include <QLayout>
#include <QPushButton>
#include <phon/gui/main_area.hpp>
#include <phon/application/settings.hpp>
#include "main_area.hpp"


namespace phonometrica {

MainArea::MainArea(Environment &env, QWidget *parent) :
    QWidget(parent), env(env)
{
    hsplitter = new Splitter(Qt::Horizontal);
    vsplitter = new Splitter(Qt::Vertical);
    m_info_panel = new InfoPanel(env);
    m_console = new Console(env);
    m_viewer = new Viewer(env);

    vsplitter->addWidget(m_viewer);
    vsplitter->addWidget(m_console);

    hsplitter->addWidget(vsplitter);
    hsplitter->addWidget(m_info_panel);

    auto main_layout = new QHBoxLayout;
    main_layout->addWidget(hsplitter);
    main_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

void MainArea::toggleConsole()
{
    bool new_state = !m_console->isVisible();
    m_console->setVisible(new_state);
    Settings::set_value(env, "hide_console", !new_state);
}

void MainArea::toggleInfo()
{
    bool new_state = !m_info_panel->isVisible();
    m_info_panel->setVisible(new_state);
    Settings::set_value(env, "hide_info", !new_state);
}

void MainArea::setDefaultLayout()
{
    layout(DEFAULT_INFO_RATIO, DEFAULT_CONSOLE_RATIO);
}

void MainArea::layout(double info_ratio, double console_ratio)
{
    m_info_panel->setMinimumWidth(100);
    auto width = hsplitter->width();
    auto viewer_size = int(width * info_ratio);
    hsplitter->setSizes({viewer_size, width-viewer_size});
    auto w2 = hsplitter->width();
    auto s = hsplitter->sizes()[0];

    auto height = vsplitter->height();
    auto viewer_height = int(height * console_ratio);
    vsplitter->setSizes({viewer_height, height-viewer_height});
    ratios();
}

std::pair<double, double> MainArea::ratios()
{
    auto sizes = hsplitter->sizes();
    auto info_ratio = double(sizes[0]) / hsplitter->width();
    sizes = vsplitter->sizes();
    auto console_ratio = double(sizes[0]) / vsplitter->height();

    return { info_ratio, console_ratio };
}

void MainArea::focusConsole()
{
    m_console->setFocus();
}

void MainArea::adjustSplitters()
{
    auto info_ratio = Settings::get_number(env, "info_ratio");
    auto console_ratio = Settings::get_number(env, "console_ratio");
    layout(info_ratio, console_ratio);
}


} // phonometrica
