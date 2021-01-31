/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: Information panel. This panel is located on the right in the main window and displays contextual           *
 * information about the file(s) that is/are currently selected.                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_INFO_PANEL_HPP
#define PHONOMETRICA_INFO_PANEL_HPP

#include <wx/simplebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/listctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <phon/runtime.hpp>
#include <phon/application/vfs.hpp>

namespace phonometrica {


class InfoPanel final : public wxPanel
{
public:

	explicit InfoPanel(Runtime &rt, wxWindow *parent);

	void OnSetFileSelection(VFileList files);

	void ImportMetadata();

	void ExportMetadata();

private:

	void SetupBook();

	void SetEmptyPage();

	void SetSingleFilePage();

	void SetMultipleFilesPage();

	void UpdateInformation();

	void DisplaySingleFile();

	void DisplayMultipleFiles();

	void ClearPanel(wxPanel *panel);

	void OnDescriptionEdited(wxCommandEvent &);

	void OnAddProperty(wxCommandEvent &);

	void OnRemoveProperty(wxCommandEvent &);

	void OnImportMetadata(wxCommandEvent &);

	void OnExportMetadata(wxCommandEvent &);

	void AddDescription(const wxString &desc);

	void AddPropertyButtons(wxPanel *panel);

	void AddSectionHeading(wxPanel *panel, const wxString &header, bool add_space);

	void AddLabel(wxPanel *panel, const wxString &label, const wxString &tooltip = wxString());

	void AddSoundLabel(wxPanel *panel, const wxString &label, const wxString &path);

	void AddProperties(wxPanel *panel, bool shared);

	void AddMetadataButtons(wxPanel *panel);

	void OnSaveDescription(wxCommandEvent &);

	void OnBindSound(wxCommandEvent &);

	void OnPropertySelected(wxListEvent &);

	void OnResizeProperties(wxSizeEvent &);

	void ResizeProperties();

	void EnablePropertyEditing(bool value);

	void OnOpenHelp(wxCommandEvent &);

	void OnTypeChosen(wxCommandEvent &);

	void OnCategoryPressEnter(wxCommandEvent &);

	void OnCategoryChanged(wxCommandEvent &);

	void OnValidateProperty(wxCommandEvent &);

	void ValidateProperty();

	void OnClearProperty(wxCommandEvent &);

	void UpdateValues();

	void SetProperties(const wxString &selected);

	void OnValueComboActivated(wxFocusEvent &);

	void OnCategoryComboKeyPressed(wxKeyEvent &);

	void OnValueComboKeyPressed(wxKeyEvent &);


	Runtime &runtime;

	VFileList m_files;

	wxSimplebook *m_book;

	wxPanel *empty_page, *single_page, *multiple_page;

	wxButton *prop_rm_btn, *save_desc_btn, *validate_btn, *clear_btn, *help_btn;

	wxRichTextCtrl *ctrl_desc;

	wxListCtrl *prop_ctrl = nullptr;

	wxChoice *type_choice;

	wxComboBox *category_combo, *value_combo;

	bool has_unsaved_property = false;
};

} // namespace phonometrica

#endif // PHONOMETRICA_INFO_PANEL_HPP


