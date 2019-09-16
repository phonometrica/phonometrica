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

#include <QLayout>
#include <QPushButton>
#include <phon/gui/main_area.hpp>
#include <phon/application/settings.hpp>
#include "main_area.hpp"


namespace phonometrica {

MainArea::MainArea(Runtime &rt, QWidget *parent) :
    QWidget(parent), rt(rt)
{
    hsplitter = new Splitter(Qt::Horizontal);
    vsplitter = new Splitter(Qt::Vertical);
    m_info_panel = new InfoPanel(rt);
    m_console = new Console(rt);
    m_viewer = new Viewer(rt);

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
    Settings::set_value(rt, "hide_console", !new_state);
}

void MainArea::toggleInfo()
{
    bool new_state = !m_info_panel->isVisible();
    m_info_panel->setVisible(new_state);
    Settings::set_value(rt, "hide_info", !new_state);
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
    auto info_ratio = Settings::get_number(rt, "info_ratio");
    auto console_ratio = Settings::get_number(rt, "console_ratio");
    layout(info_ratio, console_ratio);
}


} // phonometrica
