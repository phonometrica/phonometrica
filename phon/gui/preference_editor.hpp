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
 * Created: 08/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: editor for user settings. The settings are stored in a JSON file in the application directory.            *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_PREFERENCE_EDITOR_HPP
#define PHONOMETRICA_PREFERENCE_EDITOR_HPP

#include <QAction>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {


class PreferenceEditor final : public QDialog
{
Q_OBJECT

public:
    PreferenceEditor(QWidget *parent, Runtime &rt);

public slots:

    void accept();

private slots:

    void setPraatPath();

    void setResourcesPath();

private:

    Runtime &rt;
    QDialogButtonBox *box_buttons;
    QVBoxLayout *main_layout, *gen_layout, *sound_layout;
    QTabWidget *tabs;
    QLineEdit *edit_praat, *edit_resources, *edit_quality;
    QWidget *tab_general, *tab_advanced;
    QSpinBox *spinbox_match_context_window;
    QCheckBox *checkbox_mouse_tracking, *checkbox_autosave, *checkbox_autoload;

    void setupUi();

    void setupGeneralTab();

    void setupSoundTab();

    void setLineEditFolder(QLineEdit *);

    void setLineEditFile(QLineEdit *);
};


} // namespace phonometrica

#endif // PHONOMETRICA_PREFERENCE_EDITOR_HPP
