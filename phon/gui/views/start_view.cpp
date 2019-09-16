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
 * Created: 15/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QGridLayout>
#include <phon/gui/views/start_view.hpp>
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

void StartView::save()
{
    // nothing to do
}

QToolButton *StartView::makeButton(QWidget *parent, const QIcon &icon, const QString &text)
{
    auto button = new QToolButton(parent);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(icon);
    button->setText(text);
    button->setIconSize(QSize(100, 100));
    button->setFixedSize(300, 150);
    button->setAutoRaise(true);

    return button;
}

} // namespace phonometrica