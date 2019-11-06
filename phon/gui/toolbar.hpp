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
 * Created: 20/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Qt's toolbar looks out of place on macOS, and we can't use native toolbars in views, which are widgets and *
 * not windows, so we roll out our own.                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TOOLBAR_HPP
#define PHONOMETRICA_TOOLBAR_HPP

#include <type_traits>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QPainter>
#include <QHBoxLayout>

namespace phonometrica {

namespace detail {
#if PHON_MACOS
static constexpr bool is_mac = true;
#else
static constexpr bool is_mac = false;
#endif
}

class MacToolbar final : public QWidget
{

	class Separator : public QWidget
	{
	public:

		Separator() : QWidget()
		{
			setFixedWidth(1);
		}

		static QColor lineColor()
		{
			auto col = QColor(Qt::black);
			col.setAlpha(50);

			return col;
		}

	protected:

		void paintEvent(QPaintEvent *event) override
		{
			QPainter painter(this);

			// Set background color
			QPalette pal = palette();
			auto col = lineColor();
			pal.setColor(QPalette::Background, col);
			setAutoFillBackground(true);
			setPalette(pal);
		}
	};

public:

	MacToolbar(QWidget *parent = nullptr);

	QAction *addAction(const QIcon &icon, const QString &text);

	void addAction(QAction *action);

	void addWidget(QWidget *w);

	void addStretch(int factor = 0);

	void addSpacing(int spacing);

	void addSeparator();

private:

	QHBoxLayout *layout;

	QString stylesheet;
};

using Toolbar = std::conditional_t<detail::is_mac, MacToolbar, QToolBar>;

} // namespace phonometrica

#endif // PHONOMETRICA_TOOLBAR_HPP
