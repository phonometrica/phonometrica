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
 * Created: 08/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: editor for user settings. The settings are stored in a JSON file in the application directory.             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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
#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include <QSlider>
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

    void setResourcesPath();

	void updateResamplingQuality(int value);

	void resetSettings(bool);

private:

    Runtime &runtime;
    QDialogButtonBox *box_buttons;
    QTabWidget *tabs;
    QLineEdit *edit_resources;
    QWidget *tab_general, *tab_advanced;
    QSpinBox *spinbox_match_context_window;
    QCheckBox *checkbox_mouse_tracking, *checkbox_autosave, *checkbox_autoload;
    QSlider *resampling_slider;
    QLabel *quality_label;

    void setupUi();

    void setupGeneralTab();

    void setupSoundTab();

    void setLineEditFolder(QLineEdit *);

    void setLineEditFile(QLineEdit *);
};


} // namespace phonometrica

#endif // PHONOMETRICA_PREFERENCE_EDITOR_HPP
