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
 * Created: 20/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QToolButton>
#include <phon/gui/button_stylesheet.hpp>
#include <phon/gui/toolbar.hpp>

namespace phonometrica {

MacToolbar::MacToolbar(QWidget *parent)
{
	stylesheet = flat_button_stylesheet;
	layout = new QHBoxLayout;
	setLayout(layout);
	setFixedHeight(50);
	layout->setContentsMargins(0, 5, 0, 5);
	layout->setSpacing(0);
}

QAction *MacToolbar::addAction(const QIcon &icon, const QString &text)
{
	auto action = new QAction(icon, text);
	addAction(action);

	return action;
}

void MacToolbar::addWidget(QWidget *w)
{
	w->setFixedSize(40, 40);
	w->setStyleSheet(stylesheet);
	auto button = dynamic_cast<QToolButton*>(w);
	if (button) button->setIconSize(QSize(20, 20));

	layout->addWidget(w);
}

void MacToolbar::addStretch(int factor)
{
	layout->addStretch(factor);
}

void MacToolbar::addSpacing(int spacing)
{
	layout->addSpacing(spacing);
}

void MacToolbar::addAction(QAction *action)
{
	auto button = new QToolButton;
	button->setDefaultAction(action);
	addWidget(button);
}

void MacToolbar::addSeparator()
{
	layout->addWidget(new Separator);
}
} // namespace phonometrica