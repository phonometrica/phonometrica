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
 * Created: 12/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVariant>
#include <QGroupBox>
#include <phon/gui/user_dialog.hpp>

namespace phonometrica {

UserDialog::UserDialog(Runtime &rt, const String &str, QWidget *parent) :
	QDialog(parent), runtime(rt)
{
	layout = new QVBoxLayout;
	parse(str);
	layout->addStretch();
	layout->addSpacing(10);
	addButtonBox();
	setLayout(layout);
}

void UserDialog::parse(const String &str)
{
	auto js = Json::parse(str.data());
	Json::iterator it;

	it = js.find("title");
	if (it != js.end()) {
		auto title = it->get<std::string>();
		setWindowTitle(QString::fromStdString(title));
	}

	it = js.find("width");
	if (it != js.end()) {
		auto w = it->get<int64_t>();
		setMinimumWidth(w);
	}
	it = js.find("height");
	if (it != js.end()) {
		auto h = it->get<int64_t>();
		setMinimumHeight(h);
	}

	it = js.find("items");
	if (it == js.end()) return;
	for (auto item : *it)
	{
		if (!item.is_object()) {
			throw error("User dialog items must be objects");
		}

		auto it2 = item.find("type");
		if (it2 == item.end()) {
			throw error("User dialog item has no \"type\" key");
		}
		auto type = it2->get<std::string>();

		if (type == "label")
		{
			addLabel(item);
		}
		else if (type == "button")
		{
			addPushButton(item);
		}
		else if (type == "checkbox")
		{
			addCheckBox(item);
		}
		else if (type == "combobox")
		{
			addComboBox(item);
		}
		else if (type == "line_edit")
		{
			addLineEdit(item);
		}
		else if (type == "check_list")
		{
			addCheckList(item);
		}
		else if (type == "radio_buttons")
		{
			addRadioButtons(item);
		}
		else if (type == "file_selector")
		{
			addFileSelector(item);
		}
		else
		{
			throw error("Unknown item type in user dialog: \"%\"", type);
		}
	}
}

void UserDialog::addButtonBox()
{
	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	layout->addWidget(button_box);
	connect(button_box, &QDialogButtonBox::accepted, this, &UserDialog::accept);
	connect(button_box, &QDialogButtonBox::rejected, this, &UserDialog::reject);
}

QString UserDialog::getName(nlohmann::json item)
{
	auto it = item.find("name");
	if (it == item.end()) {
		throw error("User dialog item has no \"name\" attribute");
	}

	return QString::fromStdString(it->get<std::string>());
}

void UserDialog::addLabel(Json item)
{
	auto it = item.find("text");
	if (it == item.end()) {
		throw error("User dialog label has no \"text\" attribute");
	}
	auto text = it->get<std::string>();
	layout->addWidget(new QLabel(QString::fromStdString(text)));
}

void UserDialog::addCheckBox(Json item)
{
	auto name = getName(item);
	QString text;

	auto it = item.find("text");
	if (it != item.end()) {
		text = QString::fromStdString(it->get<std::string>());
	}

	auto cb = new QCheckBox(text);
	cb->setProperty("name", name);

	it = item.find("default");
	if (it != item.end()) {
		bool state = it->get<bool>();
		cb->setChecked(state);
	}

	layout->addWidget(cb);
	check_boxes.push_back(cb);
}

String UserDialog::get()
{
	Json js;

	for (auto cb : check_boxes)
	{
		auto name = cb->property("name").toString().toStdString();
		bool value = cb->isChecked();
		js[name] = value;
	}

	for (auto box : combo_boxes)
	{
		auto name = box->property("name").toString().toStdString();
		auto i = box->currentIndex() + 1;
		js[name] = i;
	}

	for (auto line : line_edits)
	{
		auto name = line->property("name").toString().toStdString();
		js[name] = line->text().toStdString();
	}

	for (auto list : check_lists)
	{
		auto name = list->property("name").toString().toStdString();
		js[name] = list->jsonSelection();
	}

	for (auto group : radio_buttons)
	{
		auto name = group->property("name").toString().toStdString();
		js[name] = group->checkedId();
	}

	for (auto sel : file_selectors)
	{
		auto name = sel->property("name").toString().toStdString();
		js[name] = sel->text().toStdString();
	}

	return js.dump();
}

void UserDialog::addCheckList(Json item)
{
	auto name = getName(item);
	Array<String> labels, tooltips;

	auto it = item.find("labels");
	if (it == item.end()) {
		throw error("User dialog check list has no \"labels\" attribute");
	}
	for (auto &label : *it)
	{
		labels.push_back(label.get<std::string>());
	}
	it = item.find("values");
	if (it != item.end())
	{
		for (auto &value : *it)
		{
			tooltips.append(value.get<std::string>());
		}
	}

	auto list = new CheckList(nullptr, labels, tooltips);
	list->setProperty("name", name);
	check_lists.push_back(list);
	layout->addWidget(list);
}

void UserDialog::addComboBox(Json item)
{
	auto name = getName(item);
	auto box = new QComboBox;
	box->setProperty("name", name);

	auto it = item.find("values");
	if (it == item.end()) {
		throw error("User dialog combo box has no \"values\" attribute");
	}
	if (!it->is_array()) {
		throw error("Values should be an array");
	}

	for (auto &value : *it)
	{
		auto txt = value.get<std::string>();
		box->addItem(QString::fromStdString(txt));
	}
	it = item.find("default");
	if (it != item.end()) {
		auto i = it->get<int64_t>() - 1;
		box->setCurrentIndex(i);
	}

	combo_boxes.push_back(box);
	layout->addWidget(box);
}

void UserDialog::addLineEdit(Json item)
{
	auto name = getName(item);
	auto line = new QLineEdit;
	line->setProperty("name", name);
	auto it = item.find("default");
	if (it != item.end()) {
		auto txt = it->get<std::string>();
		line->setText(QString::fromStdString(txt));
	}
	line_edits.push_back(line);
	layout->addWidget(line);
}

void UserDialog::addRadioButtons(Json item)
{
	auto name = getName(item);
	auto group = new QButtonGroup(this);
	group->setProperty("name", name);
	auto box = new QGroupBox;

	auto it = item.find("title");
	if (it != item.end()) {
		auto title = it->get<std::string>();
		box->setTitle(QString::fromStdString(title));
	}

	auto values = item.find("values");
	if (values == item.end()) {
		throw error("User dialog radio button group has no \"values\" attribute");
	}

	int sel = 1;
	it = item.find("default");
	if (it != item.end()) {
		sel = it->get<int64_t>();
	}

	auto vlayout = new QVBoxLayout;
	int id = 0;
	for (auto &value : *values)
	{
		auto txt = value.get<std::string>();
		auto btn = new QRadioButton(QString::fromStdString(txt));
		group->addButton(btn);
		group->setId(btn, ++id);
		if (id == sel) btn->setChecked(true);
		vlayout->addWidget(btn);
	}

	radio_buttons.push_back(group);
	box->setLayout(vlayout);
	layout->addWidget(box);
}

void UserDialog::addPushButton(Json item)
{
	auto it = item.find("label");
	if (it == item.end()) {
		throw error("User dialog button has no \"label\" attribute");
	}
	auto label = it->get<std::string>();
	auto btn = new QPushButton(QString::fromStdString(label));
	it = item.find("action");
	if (it != item.end())
	{
		String action = it->get<std::string>();
		connect(btn, &QPushButton::clicked, [=](bool) {
			runtime.do_string(action);
		});
	}
	String pos;
	it = item.find("position");
	if (it != item.end()) {
		pos = it->get<std::string>();
	}
	auto hl = new QHBoxLayout;
	if (pos == "right" || pos == "center") {
		hl->addStretch(1);
	}
	hl->addWidget(btn);
	if (pos == "left" || pos == "center") {
		hl->addStretch(1);
	}
	layout->addLayout(hl);
}

void UserDialog::addFileSelector(Json item)
{
	auto name = getName(item);
	auto it = item.find("title");
	if (it == item.end()) {
		throw error("User dialog file selector has no \"title\" attribute");
	}
	auto title = QString::fromStdString(it->get<std::string>());
	QString text, filter;
	it = item.find("text");
	if (it != item.end()) {
		text = QString::fromStdString(it->get<std::string>());
	}
	it = item.find("filter");
	if (it != item.end()) {
		filter = QString::fromStdString(it->get<std::string>());
	}
	auto sel = new FileSelector(title, text, filter);
	sel->setProperty("name", name);
	file_selectors.push_back(sel);
	layout->addWidget(sel);
}

} // namespace phonometrica
