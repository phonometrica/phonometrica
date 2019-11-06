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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Information panel which displays file metadata, located on the right-hand side in the main window.         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_INFO_PANEL_HPP
#define PHONOMETRICA_INFO_PANEL_HPP

#include <QFrame>
#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <phon/application/vfs.hpp>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

class InfoPanel final : public QFrame
{
    Q_OBJECT

public:

    InfoPanel(Runtime &rt, QWidget *parent = nullptr);

signals:

    void shown(bool);

public slots:

    void showEmptySelection();

    void showSelection(VFileList files);

    void reset();

private slots:

    void setFileDescription(bool);

    void editProperties();

    void refresh();

    void importMetadata();

	void enableSaveDescription();

	void bindAnnotation();

private:

    void setupUi();

    void setNothingToDisplay();

    void clearWidgets();

    void showSingleSelection(const std::shared_ptr<VFile> &file);

    void showMultipleSelection();

    void setWidgets(bool showTimes);

    void displaySelection();

    QTabWidget *m_tabs;

    Runtime &runtime;

    VFileList m_files;

    QLabel *main_label, *file_label, *soundRef_label, *properties_label, *start_label, *end_label;
    QLabel *samplerate_label, *channels_label, *duration_label;
    QPushButton *properties_btn, *import_btn, *save_desc_btn;
    QPushButton *bind_btn = nullptr;
    QTabWidget *tabs;
    QWidget *info_tab;
    QTextEdit *desc_edit;

    bool description_is_editable; // prevent a VFile's description from being modified when it is set in the widget

};


} // phonometrica

#endif // PHONOMETRICA_INFO_PANEL_HPP
