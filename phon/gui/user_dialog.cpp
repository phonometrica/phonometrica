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
 * Created: 06/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/


#include <wx/stattext.h>
#include <phon/gui/user_dialog.hpp>

namespace phonometrica {

UserDialog::UserDialog(wxWindow *parent, Runtime &rt, const Json &js) :
	wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER),
	runtime(rt)
{
	current_sizer = new VBoxSizer;
	bool yes_no = Parse(js);
	current_sizer->AddStretchSpacer();
	current_sizer->AddSpacer(10);
	AddButtons(yes_no);
	SetSizer(current_sizer);
}

UserDialog::UserDialog(wxWindow *parent, Runtime &rt, const String &s) :
	UserDialog(parent, rt, rt.do_string(s))
{

}

void UserDialog::AddButtons(bool yes_no)
{
	wxString yes_label, no_label;
	int yes_id, no_id;

	if (yes_no)
	{
		yes_label = _("Yes");
		no_label = _("No");
		yes_id = wxID_YES;
		no_id = wxID_NO;
	}
	else
	{
		yes_label = _("OK");
		no_label = _("Cancel");
		yes_id = wxID_OK;
		no_id = wxID_CANCEL;
	}

	auto cancel_btn = new wxButton(this, no_id, no_label);
	auto ok_btn = new wxButton(this, yes_id, yes_label);
	wxSize btn_size(80, 50);
	ok_btn->SetMaxSize(btn_size);
	cancel_btn->SetMaxSize(btn_size);
	auto hsizer = new wxBoxSizer(wxHORIZONTAL);
	hsizer->AddStretchSpacer(1);
	hsizer->Add(cancel_btn, 1, 0, 0);
	hsizer->AddSpacer(10);
	hsizer->Add(ok_btn, 1, 0, 0);
#if PHON_MACOS
	int spacing = 15;
#else
	int spacing = 10;
#endif
	current_sizer->Add(hsizer, 0, wxEXPAND | wxLEFT | wxRIGHT, spacing);
	current_sizer->AddSpacer(10);

	cancel_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UserDialog::OnCancel, this);
	ok_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &UserDialog::OnOk, this);
}

Variant UserDialog::GetJson() const
{
	Json::Object js;

	for (auto &cb : checkboxes) {
		js[cb.first] = cb.second->IsChecked();
	}

	for (auto &box : comboboxes)
	{
		js[box.first] = box.second->GetSelection() + 1;
	}

	for (auto &line : fields)
	{
		js[line.first] = String(line.second->GetValue());
	}

	for (auto &list : checklists)
	{
		wxArrayInt items;
		list.second->GetCheckedItems(items);
		Array<Variant> values;
		for (int i : items)
		{
			values.append(list.second->GetJsonValue(i));
		}
		js[list.first] = make_handle<List>(&runtime, std::move(values));
	}

	for (auto &group : radioboxes)
	{
		js[group.first] = (intptr_t) group.second->GetSelection() + 1;
	}

	for (auto &sel : filepickers)
	{
		js[sel.first] = String(sel.second->GetPath());
	}

	return Variant(make_handle<Table>(&runtime, std::move(js)));
}

bool UserDialog::Parse(const Json &js)
{
	if (!js.is_object()) {
		throw error("Invalid JSON object passed to create_dialog()");
	}
	bool yes_no = false;

	auto it = js.find("title");
	if (it != js.end())
	{
		auto title = it.get_string();
		SetTitle(title);
	}

	int width = -1, height = -1;
	it = js.find("width");
	if (it != js.end())
	{
		width = (int) it.get_integer();
	}

	it = js.find("height");
	if (it != js.end()) {
		height = (int) it.get_integer();
	}

	if (width > 0 || height > 0) {
		SetSize(FromDIP(wxSize(width, height)));
	}

	it = js.find("yes_no");
	if (it != js.end()) {
		yes_no = it.get_boolean();
	}

	it = js.find("items");
	if (it != js.end())
	{
		for (auto &item : it.get_array()) {
			ParseItem(item);
		}
	}

	return yes_no;
}

void UserDialog::ParseItem(Json item)
{
	if (!item.is_object()) {
		throw error("User dialog items must be tables");
	}

	auto it2 = item.find("type");
	if (it2 == item.end()) {
		throw error("User dialog item has no \"type\" key");
	}
	auto type = it2.get_string();

	if (type == "label")
	{
		AddLabel(item);
	}
	else if (type == "button")
	{
		AddButton(item);
	}
	else if (type == "check_box")
	{
		AddCheckBox(item);
	}
	else if (type == "combo_box")
	{
		AddComboBox(item);
	}
	else if (type == "field")
	{
		AddLineEdit(item);
	}
	else if (type == "check_list")
	{
		AddCheckList(item);
	}
	else if (type == "radio_buttons")
	{
		AddRadioButtons(item);
	}
	else if (type == "file_selector")
	{
		AddFileSelector(item);
	}
	else if (type == "container")
	{
		AddContainer(item);
	}
	else if (type == "stretch")
	{
		current_sizer->AddStretchSpacer();
	}
	else if (type == "spacing")
	{
		AddSpacing(item);
	}
	else
	{
		throw error("Unknown item type in user dialog: \"%\"", type);
	}
}

void UserDialog::OnOk(wxCommandEvent &)
{
	EndModal(wxID_OK);
}

void UserDialog::OnCancel(wxCommandEvent &)
{
	EndModal(wxID_CANCEL);
}

void UserDialog::Add(wxWindow *win)
{
	current_sizer->Add(win, 0, sizer_flag, sizer_border);
}

void UserDialog::Add(wxSizer *sizer)
{
	current_sizer->Add(sizer, 0, sizer_flag, sizer_border);
}

void UserDialog::AddLabel(const Json &item)
{
	auto it = item.find("text");
	if (it == item.end()) {
		throw error("User dialog label has no \"text\" attribute");
	}
	auto text = it.get_string();
	Add(new wxStaticText(this, wxID_ANY, text));
}

void UserDialog::AddButton(const Json &item)
{
	auto it = item.find("label");
	if (it == item.end()) {
		throw error("User dialog button has no \"label\" attribute");
	}
	auto label = it.get_string();
	auto btn = new wxButton(this, wxID_ANY, label);
	it = item.find("action");
	if (it != item.end())
	{
		String action = it.get_string();
		btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent &) { runtime.do_string(action); });
	}
	String pos;
	it = item.find("position");
	if (it != item.end()) {
		pos = it.get_string();
	}
	auto hl = new HBoxSizer;
	if (pos == "right" || pos == "center") {
		hl->AddStretchSpacer(1);
	}
	hl->Add(btn);
	if (pos == "left" || pos == "center") {
		hl->AddStretchSpacer(1);
	}
	Add(hl);
}

void UserDialog::AddCheckBox(const Json &item)
{
	auto name = GetName(item);
	String text;

	auto it = item.find("text");
	if (it != item.end()) {
		text = it.get_string();
	}

	auto cb = new wxCheckBox(this, wxID_ANY, text);
	checkboxes[name] = cb;

	it = item.find("default");
	if (it != item.end()) {
		bool state = it.get_boolean();
		cb->SetValue(state);
	}

	Add(cb);
}

String UserDialog::GetName(const Json &item)
{
	auto it = item.find("name");
	if (it == item.end()) {
		throw error("User dialog item has no \"name\" attribute");
	}

	return it.get_string();
}

void UserDialog::AddComboBox(const Json &item)
{
	auto name = GetName(item);

	auto it = item.find("values");
	if (it == item.end()) {
		throw error("User dialog combo box has no \"values\" attribute");
	}
	auto values = it.value();
	if (!values.is_array()) {
		throw error("Values should be a list");
	}

	wxArrayString choices;
	for (Json value : values.get_array())
	{
		try {
			choices.Add(value.get_string());
		}
		catch (...) {
			throw error("Invalid string value in combo box");
		}
	}
	auto box = new wxComboBox(this, wxID_ANY);

	it = item.find("default");
	if (it != item.end()) {
		auto i = int(it.get_integer() - 1);
		box->SetSelection(i);
	}

	comboboxes[name] = box;
	Add(box);
}

void UserDialog::AddLineEdit(const Json &item)
{
	auto name = GetName(item);
	auto line = new wxTextCtrl(this, wxID_ANY);

	auto it = item.find("default");
	if (it != item.end()) {
		line->SetValue(it.get_string());
	}

	fields[name] = line;
	Add(line);
}

void UserDialog::AddCheckList(const Json &item)
{
	auto name = GetName(item);
	wxArrayString labels, tooltips;

	auto it = item.find("values");
	if (it == item.end()) {
		throw error("User dialog check list has no \"values\" attribute");
	}
	auto values = it.value();
	if (!values.is_array()) {
		throw error("\"values\" in checklist must be a list");
	}
	for (Json value : values.get_array())
	{
		try {
			tooltips.Add(value.get_string());
		}
		catch (...) {
			throw error("Invalid string value in checklist");
		}
	}
	it = item.find("labels");

	if (it != item.end())
	{
		if (!it.value().is_array()) {
			throw error("\"labels\" in checklist must be a list");
		}
		for (Json label : it.get_array())
		{
			try {
				labels.Add(label.get_string());
			}
			catch (...) {
				throw error("Invalid string label in checklist");
			}
		}
	}
	else
	{
		std::swap(labels, tooltips);
	}

	if (!tooltips.empty() && labels.size() != tooltips.size()) {
		throw error("Inconsistent number of labels and values in user dialog list box (% vs %)",
		            labels.size(), tooltips.size());
	}
	auto list = new CheckListBox(this, labels, tooltips);
	list->SetMaxSize(FromDIP(wxSize(1000, 150)));
	Add(list);
	checklists[name] = list;
}

void UserDialog::AddRadioButtons(const Json &item)
{
	auto name = GetName(item);
	String title;

	auto it = item.find("title");
	if (it != item.end()) {
		title = it.get_string();
	}

	auto values = item.find("values");
	if (values == item.end()) {
		throw error("User dialog radio button group has no \"values\" attribute");
	}

	int sel = 1;
	it = item.find("default");
	if (it != item.end()) {
		sel = (int) it.get_integer();
	}

	if (!values.value().is_array()) {
		throw error("\"values\" must be a list in radio buttons");
	}
	wxArrayString choices;
	for (Json value : values.get_array()) {
		choices.Add(value.get_string());
	}

	auto box = new wxRadioBox(this, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, choices, 1, wxHORIZONTAL);
	box->SetSelection(sel);
	Add(box);
	radioboxes[name] = box;
}

void UserDialog::AddFileSelector(const Json &item)
{
	auto name = GetName(item);
	auto it = item.find("title");
	if (it == item.end()) {
		throw error("User dialog file selector has no \"title\" attribute");
	}
	auto title = it.get_string();
	wxString text, filter;
	it = item.find("default");
	if (it != item.end()) {
		text = it.get_string();
	}
	it = item.find("filter");
	if (it != item.end()) {
		filter = it.get_string();
	}

	it = item.find("save");
	int flag = wxFLP_DEFAULT_STYLE;
	if (it != item.end() && it.get_boolean()) {
		flag = wxFLP_SAVE|wxFLP_OVERWRITE_PROMPT;
	}
	// TODO: Add option to save!
	auto sel = new wxFilePickerCtrl(this, wxID_ANY, text, title, filter, wxDefaultPosition, wxDefaultSize, flag|wxFLP_USE_TEXTCTRL);
	Add(sel);
	filepickers[name] = sel;
}

void UserDialog::AddContainer(const Json &item)
{
	auto previous_sizer = current_sizer;
	int previous_flag = sizer_flag;
	int previous_border = sizer_border;
	sizer_flag = sizer_border = 0;
	auto it = item.find("orientation");
	if (it != item.end() && it.get_string() == "vertical")
	{
		current_sizer = new VBoxSizer;
	}
	else
	{
		current_sizer = new HBoxSizer;
	}


	it = item.find("items");
	if (it == item.end()) {
		throw error("User dialog container has no \"items\" attribute");
	}
	for (Json itm : it.get_array())
	{
		ParseItem(std::move(itm));
	}

	auto sizer = current_sizer;
	current_sizer = previous_sizer;
	sizer_flag = previous_flag;
	sizer_border = previous_border;
	Add(sizer);
}

void UserDialog::AddSpacing(const Json &item)
{
	auto it = item.find("size");
	if (it == item.end()) {
		throw error("User dialog spacing has no \"size\" attribute");
	}
	current_sizer->AddSpacer((int)it.get_integer());
}

} // namespace phonometrica
