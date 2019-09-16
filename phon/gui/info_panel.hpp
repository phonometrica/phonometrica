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

    Runtime &rt;

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
