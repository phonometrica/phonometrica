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
 * Purpose: Convenience widget to display and select a file name.                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FILE_SELECTOR_HPP
#define PHONOMETRICA_FILE_SELECTOR_HPP

#include <QLineEdit>
#include <QPushButton>

namespace phonometrica {

class FileSelector final : public QWidget
{
	Q_OBJECT

public:

	FileSelector(const QString &title, const QString &path = QString(), const QString &filter = QString(), QWidget *parent = nullptr);

	QString text() const;

	void setText(const QString &value);

private:

	QLineEdit *line;

	QString m_filter;

};

} // namespace phonometrica



#endif // PHONOMETRICA_FILE_SELECTOR_HPP
