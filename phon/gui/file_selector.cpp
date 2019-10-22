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
