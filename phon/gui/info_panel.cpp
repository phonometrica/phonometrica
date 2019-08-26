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
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QLayout>
#include <QLabel>
#include <QFileDialog>
#include <phon/gui/info_panel.hpp>
#include <phon/gui/metadata_editor.hpp>
#include <phon/application/project.hpp>
#include <phon/application/settings.hpp>
#include <phon/utils/file_system.hpp>
#include "info_panel.hpp"


namespace phonometrica {

InfoPanel::InfoPanel(Runtime &rt, QWidget *parent) :
    QFrame(parent), rt(rt)
{
    setupUi();
}

void InfoPanel::setupUi()
{
    m_tabs = new QTabWidget(this);
    m_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    info_tab = new QWidget;
    auto main_layout = new QVBoxLayout;
    main_layout->addWidget(m_tabs);
    main_layout->setContentsMargins(0, 0, 0, 0);

    setNothingToDisplay();

    m_tabs->addTab(info_tab, tr("File metadata"));
    this->setLayout(main_layout);
}

void InfoPanel::setNothingToDisplay()
{
    QVBoxLayout *vl = new QVBoxLayout;
    QHBoxLayout *headerLayout = new QHBoxLayout;
    auto main_label = new QLabel(tr("<font color=\"grey\">No metadata to display</font>"));

    headerLayout->addStretch();
    headerLayout->addWidget(main_label);
    headerLayout->addStretch();

    vl->addStretch();
    vl->addLayout(headerLayout);
    vl->addStretch();

    info_tab->setLayout(vl);
}

void InfoPanel::showEmptySelection()
{
    clearWidgets();
    setNothingToDisplay();
}

void InfoPanel::clearWidgets()
{
    delete info_tab->layout();
    for(QObject *child : info_tab->children())
    {
        delete child;
    }
    main_label = file_label = soundRef_label = properties_label = start_label = end_label = nullptr;
    samplerate_label = channels_label = duration_label = nullptr;
}

void InfoPanel::showSelection(VFileList files)
{
    Q_ASSERT(!files.empty());
    m_files.swap(files);
    displaySelection();
}

void InfoPanel::showSingleSelection(const std::shared_ptr<VFile> &file)
{
    setWidgets(false);


    file_label->setText(tr("<b>File:</b><br/>") + QString(file->label()));
    file_label->setToolTip(file->path());

    if (file->is_annotation())
    {
        auto annot = dynamic_cast<Annotation*>(file.get());
        QString sndfile = tr("<b>Sound file:</b><br/>");
        if (annot->has_sound())
        {
            auto sound_path = annot->sound()->path();
            sndfile += filesystem::base_name(sound_path);
            soundRef_label->setText(sndfile);
            soundRef_label->setToolTip(sound_path);
        }
        else
        {
            soundRef_label->setText(sndfile + tr("None"));
        }
    }
    else
    {
        soundRef_label->hide();
    }

    if (file->is_sound())
    {
        try
        {
            auto sound = dynamic_cast<Sound*>(file.get());
            auto handle = sound->handle();

            auto text = QString("<b>Sample rate:</b><br/>%1 Hz").arg(handle.samplerate());
            samplerate_label->setText(text);

            text = QString("<b>Number of channels:</b><br/>%1").arg(handle.channels());
            channels_label->setText(text);

            double duration = double(handle.frames()) / handle.samplerate();
            text = QString("<b>Duration:</b><br/>%1 seconds").arg(QString::number(duration, 'f'));
            duration_label->setText(text);
        }
        catch (...)
        {
            samplerate_label->setText("<b>Sample rate:</b><br/>Unknown");
            channels_label->setText("<b>Number of channels:</b><br/>unknown");
            duration_label->setText("<b>Duration:</b><br/>unknown");
        }
    }
    else
    {
        samplerate_label->hide();
        channels_label->hide();
        duration_label->hide();
    }

    if (file->has_properties())
    {
        auto properties = QString("<b>Properties:</b><br/>") + String::join(file->property_list(), "<br/>");
        properties_label->setText(properties);
    }

    desc_edit->setText(file->description());
    description_is_editable = true;
}

void InfoPanel::showMultipleSelection()
{
    setNothingToDisplay();
}

void InfoPanel::setWidgets(bool showTimes)
{
    clearWidgets();

    auto vl = new QVBoxLayout;

    file_label  = new QLabel;
    properties_label  = new QLabel;
    soundRef_label = new QLabel;
    properties_btn = new QPushButton(tr("Edit properties..."));
    import_btn = new QPushButton(tr("Import metadata..."));
    desc_edit = new QTextEdit;
    description_is_editable = false;
    samplerate_label = new QLabel;
    channels_label = new QLabel;
    duration_label = new QLabel;
    vl->addWidget(file_label);
    vl->addWidget(soundRef_label);
    vl->addWidget(duration_label);
    vl->addWidget(samplerate_label);
    vl->addWidget(channels_label);

    vl->addWidget(properties_label);
    properties_label->setWordWrap(true);

    connect(properties_btn, SIGNAL(clicked()), this, SLOT(editProperties()));
    connect(import_btn, SIGNAL(clicked()), this, SLOT(importMetadata()));

    if (showTimes)
    {
        start_label = new QLabel;
        end_label   = new QLabel;

        vl->addWidget(start_label);
        vl->addWidget(end_label);
    }
    else
    {
        start_label = nullptr;
        end_label = nullptr;
    }

    auto hl2 = new QHBoxLayout;
    hl2->addWidget(properties_btn);
    hl2->addSpacing(5);
    hl2->addWidget(import_btn);
    hl2->addStretch();
    hl2->setSpacing(0);
    vl->addLayout(hl2);

    vl->addWidget(new QLabel(tr("<b>Description:<b>")));
    vl->addWidget(desc_edit);
    save_desc_btn = new QPushButton(tr("Save description"));
	connect(save_desc_btn, &QPushButton::clicked, this, &InfoPanel::setFileDescription);
    vl->addWidget(save_desc_btn);
    info_tab->setLayout(vl);
}

void InfoPanel::setFileDescription(bool)
{
    bool first_modification;

    if (description_is_editable)
    {
        String desc = desc_edit->toPlainText();

        if (!m_files.empty())
        {
            auto file = m_files.front();
	        first_modification = !file->modified();
            file->set_description(desc);

            if (first_modification) {
                emit Project::updated();
            }
        }
    }
}

void InfoPanel::editProperties()
{
    MetadataEditor *editor = new MetadataEditor(this, m_files);
    connect(editor, &MetadataEditor::accepted, this, &InfoPanel::refresh);
    editor->show();
}

void InfoPanel::displaySelection()
{
    clearWidgets();

    if (m_files.size() == 1)
    {
        showSingleSelection(m_files.front());
    }
    else
    {
        showMultipleSelection();
    }
}

void InfoPanel::refresh()
{
    displaySelection();
    Project::updated();
}

void InfoPanel::importMetadata()
{
    QString dir = Settings::get_string(rt, "last_directory");
    auto p = QFileDialog::getOpenFileName(this, "Import metadata from CSV file...", dir, "CSV (*.csv)");

    if (!p.isNull())
    {
        String path = p;
        String dir = filesystem::directory_name(path);
        Settings::set_value(rt, "last_directory", dir);
        Project::instance()->import_metadata(path);
    }
}

void InfoPanel::reset()
{
    displaySelection();
}


} // phonometrica
