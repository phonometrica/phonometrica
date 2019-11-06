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
 * Created: 15/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QGridLayout>
#include <phon/gui/views/start_view.hpp>
#include <phon/gui/button_stylesheet.hpp>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

StartView::StartView(QWidget *parent, Runtime &rt) :
    View(parent), rt(rt)
{
    auto layout = new QVBoxLayout;
    auto grid = new QGridLayout;
    auto new_folder = makeButton(this, QIcon(":/icons/100x100/new_folder.png"), tr("Add files to project"));
    auto folder = makeButton(this, QIcon(":/icons/100x100/folder.png"), tr("Open existing project"));
    auto doc = makeButton(this, QIcon(":/icons/100x100/help.png"), tr("Documentation"));
    auto settings = makeButton(this, QIcon(":/icons/100x100/settings.png"), tr("Settings"));

    connect(new_folder, SIGNAL(clicked(bool)), this, SLOT(onAddFiles()));
    connect(folder, SIGNAL(clicked(bool)), this, SLOT(onOpenProject()));
    connect(doc, SIGNAL(clicked(bool)), this, SLOT(onOpenDoc()));
    connect(settings, SIGNAL(clicked(bool)), this, SLOT(onOpenSettings()));

    auto wgt = new QWidget;
    wgt->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    wgt->setLayout(grid);

    //grid->setSpacing(100);
    grid->addWidget(new_folder, 0, 0);
    grid->addWidget(folder, 0, 1);
    grid->addWidget(doc, 1, 0);
    grid->addWidget(settings, 1, 1);

    layout->addWidget(wgt);
    setLayout(layout);
}

void StartView::onAddFiles()
{
    rt.do_string("phon.add_files()");
}

void StartView::onOpenProject()
{
    rt.do_string("phon.open_project()");
}

void StartView::onOpenSettings()
{
    rt.do_string("phon.edit_settings()");
}

void StartView::onOpenDoc()
{
    rt.do_string("phon.show_help()");
}

QToolButton *StartView::makeButton(QWidget *parent, const QIcon &icon, const QString &text)
{
    auto button = new QToolButton(parent);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(icon);
    button->setText(text);
    button->setIconSize(QSize(100, 100));
    button->setFixedSize(300, 150);
#if PHON_MACOS
	button->setStyleSheet(flat_button_stylesheet);
#else
    button->setAutoRaise(true);
#endif
    return button;
}

} // namespace phonometrica