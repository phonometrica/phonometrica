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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/preference_editor.hpp>
#include <phon/application/settings.hpp>

namespace phonometrica {

PreferenceEditor::PreferenceEditor(QWidget *parent, Runtime &rt) :
		QDialog(parent), rt(rt)
{
	this->setupUi();
}

void PreferenceEditor::setupUi()
{
	setWindowTitle(tr("Edit preferences..."));
	resize(400, 300);
    main_layout = new QVBoxLayout(this);
	tabs = new QTabWidget;

    tab_general = new QWidget;
    gen_layout = new QVBoxLayout(tab_general);
    tabs->addTab(tab_general, tr("General"));
	this->setupGeneralTab();

    tab_advanced = new QWidget;
    sound_layout = new QVBoxLayout(tab_advanced);
    tabs->addTab(tab_advanced, tr("Sound"));
    this->setupSoundTab();


    box_buttons = new QDialogButtonBox(this);
//	buttonBox->setGeometry(QRect(30, 200, 341, 32));
    box_buttons->setOrientation(Qt::Horizontal);
    box_buttons->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    main_layout->addWidget(tabs);
    main_layout->addWidget(box_buttons);

//	labEdit->setFocus(Qt::OtherFocusReason);

    connect(box_buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(box_buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void PreferenceEditor::setupGeneralTab()
{
    // Resources path
    auto hbox_resources = new QHBoxLayout;
    auto resources_btn = new QPushButton("Choose...");
    edit_resources = new QLineEdit;
    edit_resources->setText(Settings::get_string(rt, "resources_directory"));
    connect(resources_btn, SIGNAL(clicked()), this, SLOT(setResourcesPath()));
    hbox_resources->addWidget(edit_resources);
    hbox_resources->addWidget(resources_btn);

    // Praat Path
    auto hbox_praat = new QHBoxLayout;
    auto praat_path_btn = new QPushButton("Choose...");
    edit_praat = new QLineEdit;
    edit_praat->setText(Settings::get_string(rt, "praat_path"));
    hbox_praat->addWidget(edit_praat);
    hbox_praat->addWidget(praat_path_btn);
    connect(praat_path_btn, SIGNAL(clicked()), this, SLOT(setPraatPath()));

    // Context window
    QHBoxLayout *hbox_context = new QHBoxLayout;
	QLabel *contextLabel = new QLabel(tr("Match context window:"));
    spinbox_match_context_window = new QSpinBox;
    spinbox_match_context_window->setRange(10, 100);
    spinbox_match_context_window->setSingleStep(1);
    int len = Settings::get_int(rt, "match_window_length");
    spinbox_match_context_window->setValue(len);

    hbox_context->addWidget(contextLabel);
    hbox_context->addWidget(spinbox_match_context_window);

    // Autoload/autosave
    checkbox_autoload = new QCheckBox(tr("Load most recent project on startup"));
    checkbox_autoload->setChecked(Settings::get_boolean(rt, "autoload"));
    checkbox_autosave = new QCheckBox(tr("Automatically save project on exit"));
    checkbox_autosave->setChecked(Settings::get_boolean(rt, "autosave"));

    gen_layout->addWidget(new QLabel(tr("Resources folder:")));
    gen_layout->addLayout(hbox_resources);
    gen_layout->addSpacing(20);
    gen_layout->addWidget(new QLabel(tr("Praat path:")));
    gen_layout->addLayout(hbox_praat);
    gen_layout->addSpacing(10);
    gen_layout->addLayout(hbox_context);
    gen_layout->addWidget(checkbox_autoload);
    gen_layout->addWidget(checkbox_autosave);
    gen_layout->addStretch();
}

void PreferenceEditor::setupSoundTab()
{
    checkbox_mouse_tracking = new QCheckBox(tr("Enable mouse tracking in sound and annotation views"));
    checkbox_mouse_tracking->setChecked(Settings::get_boolean(rt, "enable_mouse_tracking"));

    sound_layout->addWidget(checkbox_mouse_tracking);
    auto quality_layout = new QHBoxLayout;
    quality_layout->addWidget(new QLabel("Resampling quality (0 = worst, 10 = best)"));
    edit_quality = new QLineEdit;
    edit_quality->setText(QString::number(Settings::get_int(rt, "resampling_quality")));
    quality_layout->addWidget(edit_quality);
    sound_layout->addLayout(quality_layout);
    sound_layout->addStretch(0);
}

void PreferenceEditor::accept()
{
	// General tab

	Settings::set_value(rt, "resources_directory", edit_resources->text());
    Settings::set_value(rt, "praat_path", edit_praat->text());

	double len = spinbox_match_context_window->value();
    Settings::set_value(rt, "match_window_length", len);

    bool autoload = checkbox_autoload->isChecked();
    Settings::set_value(rt, "autoload", autoload);
    bool autosave = checkbox_autosave->isChecked();
    Settings::set_value(rt, "autosave", autosave);

	// Sound tab

    bool tracking = checkbox_mouse_tracking->isChecked();
    Settings::set_value(rt, "enable_mouse_tracking", tracking);
    bool ok;
    auto quality = edit_quality->text().toInt(&ok);
    if (!ok) quality = 5;
    if (quality < 0) quality = 0;
    if (quality > 10) quality = 10;
    Settings::set_value(rt, "resampling_quality", double(quality));

	QDialog::accept();
}

void PreferenceEditor::setPraatPath()
{
    this->setLineEditFile(edit_praat);
}

void PreferenceEditor::setResourcesPath()
{
    this->setLineEditFolder(edit_resources);
}

void PreferenceEditor::setLineEditFolder(QLineEdit *line)
{
    auto dir = Settings::get_string(rt, "last_directory");
	QString path = QFileDialog::getExistingDirectory(this->parentWidget(), tr("Choose folder..."), dir);

	if (!path.isEmpty())
		line->setText(path);
}

void PreferenceEditor::setLineEditFile(QLineEdit *line)
{
    QString dir = Settings::get_string(rt, "last_directory");
    QString path = QFileDialog::getOpenFileName(this->parentWidget(), tr("Choose file..."), dir);

	if (!path.isEmpty())
		line->setText(path);
}

} // namespace phonometrica
