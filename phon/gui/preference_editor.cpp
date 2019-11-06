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
 * Created: 08/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QLayout>
#include <phon/gui/preference_editor.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

PreferenceEditor::PreferenceEditor(QWidget *parent, Runtime &rt) :
		QDialog(parent), runtime(rt)
{
	this->setupUi();
}

void PreferenceEditor::setupUi()
{
	setWindowTitle(tr("Edit preferences..."));
	resize(400, 300);
    auto main_layout = new QVBoxLayout(this);
	tabs = new QTabWidget;

    tab_general = new QWidget;
	this->setupGeneralTab();
    tabs->addTab(tab_general, tr("General"));

    tab_advanced = new QWidget;
	this->setupSoundTab();
    tabs->addTab(tab_advanced, tr("Sound"));

    auto hl = new QHBoxLayout;
	auto reset_button = new QPushButton("Reset");
    box_buttons = new QDialogButtonBox(this);
    box_buttons->setOrientation(Qt::Horizontal);
    box_buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    hl->addWidget(reset_button);
    hl->addStretch();
	hl->addWidget(box_buttons);

    main_layout->addWidget(tabs);
	main_layout->addLayout(hl);

    connect(box_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(box_buttons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(reset_button, SIGNAL(clicked(bool)), this, SLOT(resetSettings(bool)));
}

void PreferenceEditor::setupGeneralTab()
{
	auto gen_layout = new QVBoxLayout(tab_general);
    // Resources path
    auto hbox_resources = new QHBoxLayout;
    auto resources_btn = new QPushButton("Choose...");
    edit_resources = new QLineEdit;
    edit_resources->setText(Settings::get_string(runtime, "resources_directory"));
    connect(resources_btn, SIGNAL(clicked()), this, SLOT(setResourcesPath()));
    hbox_resources->addWidget(edit_resources);
    hbox_resources->addWidget(resources_btn);

    // Context window
    auto hbox_context = new QHBoxLayout;
	auto contextLabel = new QLabel(tr("Match context window:"));
    spinbox_match_context_window = new QSpinBox;
    spinbox_match_context_window->setRange(10, 100);
    spinbox_match_context_window->setSingleStep(1);
    int len = Settings::get_int(runtime, "match_window_length");
    spinbox_match_context_window->setValue(len);

    hbox_context->addWidget(contextLabel);
    hbox_context->addWidget(spinbox_match_context_window);

    // Autoload/autosave
    checkbox_autoload = new QCheckBox(tr("Load most recent project on startup"));
    checkbox_autoload->setChecked(Settings::get_boolean(runtime, "autoload"));
    checkbox_autosave = new QCheckBox(tr("Automatically save project on exit"));
    checkbox_autosave->setChecked(Settings::get_boolean(runtime, "autosave"));

    gen_layout->addWidget(new QLabel(tr("Resources folder:")));
    gen_layout->addLayout(hbox_resources);
    gen_layout->addSpacing(10);
    gen_layout->addLayout(hbox_context);
    gen_layout->addWidget(checkbox_autoload);
    gen_layout->addWidget(checkbox_autosave);
    gen_layout->addStretch();
}

void PreferenceEditor::setupSoundTab()
{
    checkbox_mouse_tracking = new QCheckBox(tr("Enable mouse tracking in sound and annotation views"));
    checkbox_mouse_tracking->setChecked(Settings::get_boolean(runtime, "enable_mouse_tracking"));

	auto sound_layout = new QVBoxLayout(tab_advanced);
    sound_layout->addWidget(checkbox_mouse_tracking);
    sound_layout->addStretch(0);
}

void PreferenceEditor::accept()
{
	// General tab

	Settings::set_value(runtime, "resources_directory", edit_resources->text());

	double len = spinbox_match_context_window->value();
    Settings::set_value(runtime, "match_window_length", len);

    bool autoload = checkbox_autoload->isChecked();
    Settings::set_value(runtime, "autoload", autoload);
    bool autosave = checkbox_autosave->isChecked();
    Settings::set_value(runtime, "autosave", autosave);

	// Sound tab

    bool tracking = checkbox_mouse_tracking->isChecked();
    Settings::set_value(runtime, "enable_mouse_tracking", tracking);

	QDialog::accept();
}


void PreferenceEditor::resetSettings(bool)
{
	if (tabs->currentIndex() == 0)
	{
		Settings::reset_general_settings(runtime);

		auto dir = Settings::get_string(runtime, "resources_directory");
		edit_resources->setText(dir);

		int len = Settings::get_int(runtime, "match_window_length");
		spinbox_match_context_window->setValue(len);

		bool autoload = Settings::get_boolean(runtime, "autoload");
		checkbox_autoload->setChecked(autoload);

		bool autosave = Settings::get_boolean(runtime, "autosave");
		checkbox_autosave->setChecked(autosave);
	}
	else
	{
		Settings::reset_sound_settings(runtime);

		bool tracking = Settings::get_boolean(runtime, "enable_mouse_tracking");
		checkbox_mouse_tracking->setChecked(tracking);
	}
}

void PreferenceEditor::setResourcesPath()
{
    this->setLineEditFolder(edit_resources);
}

void PreferenceEditor::setLineEditFolder(QLineEdit *line)
{
    auto dir = Settings::get_string(runtime, "last_directory");
	QString path = QFileDialog::getExistingDirectory(this->parentWidget(), tr("Choose folder..."), dir);

	if (!path.isEmpty())
		line->setText(path);
}

void PreferenceEditor::setLineEditFile(QLineEdit *line)
{
    QString dir = Settings::get_string(runtime, "last_directory");
    QString path = QFileDialog::getOpenFileName(this->parentWidget(), tr("Choose file..."), dir);

	if (!path.isEmpty())
		line->setText(path);
}

} // namespace phonometrica
