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
 * Created: 12/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QFileDialog>
#include <phon/gui/file_selector.hpp>

namespace phonometrica {

FileSelector::FileSelector(const QString &title, const QString &path, const QString &filter, QWidget *parent) :
	QWidget(parent), m_filter(filter)
{
	auto layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	line = new QLineEdit;
	line->setText(path);
	auto btn = new QPushButton(tr("Choose..."));
	layout->addWidget(line);
	layout->addWidget(btn);
	setLayout(layout);

	connect(btn, &QPushButton::clicked, [=](bool) {
		QFileDialog dialog(this);
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setNameFilter(filter);
		dialog.setWindowTitle(title);
		if (dialog.exec())
		{
			auto file = dialog.selectedFiles().first();
			line->setText(file);
		}
	});
}

QString FileSelector::text() const
{
	return line->text();
}

void FileSelector::setText(const QString &value)
{
	line->setText(value);
}
} // namespace phonometrica
