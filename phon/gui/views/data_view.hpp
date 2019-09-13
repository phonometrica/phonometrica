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
 * Created: 14/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: Display tabular data.                                                                                     *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_DATA_VIEW_HPP
#define PHONOMETRICA_DATA_VIEW_HPP

#include <QTableWidget>
#include <QCheckBox>
#include <phon/runtime/runtime.hpp>
#include <phon/gui/views/view.hpp>
#include <phon/application/dataset.hpp>
#include <phon/application/annotation.hpp>
#include <phon/application/query_dataset.hpp>

namespace phonometrica {

class AudioPlayer;


class DataView final : public View
{
    Q_OBJECT

public:

    DataView(QWidget *parent, Runtime &rt, std::shared_ptr<Dataset> data);

	void save() override;

signals:

	void openAnnotation(AutoAnnotation, intptr_t, double, double);


private slots:

	void refreshTable(bool);

	void onCellClicked(int i, int j);

	void onCellDoubleClicked(int i, int j);

	void openMatchInPraat(int i);

protected:

	void keyReleaseEvent(QKeyEvent *event) override;

private:

	void fill_table();

	int getQueryFlags();

	void playMatch(int i);

	void editEvent(int i);

	void openInAnnotation(int i);

	QueryDataset *getQueryDataset() const { return dynamic_cast<QueryDataset*>(m_data.get()); }

	void enableQueryButtons(bool enable);

	void stopPlayer();

	Runtime &runtime;

    std::shared_ptr<Dataset> m_data;

    QTableWidget *m_table;

    QAction *property_action = nullptr;

    QAction *context_action = nullptr;

    QAction *info_action = nullptr;

    QAction *play_action = nullptr;

    QAction *stop_action = nullptr;

    QAction *edit_action = nullptr;

    std::unique_ptr<AudioPlayer> player;
};

} // namespace phonometrica

#endif // PHONOMETRICA_DATA_VIEW_HPP
