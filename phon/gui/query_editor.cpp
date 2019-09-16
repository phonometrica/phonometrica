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
 * Created: 30/08/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QScrollArea>
#include <QMessageBox>
#include <QPushButton>
#include <phon/application/search/query.hpp>
#include <phon/application/property.hpp>
#include <phon/application/project.hpp>
#include <phon/gui/query_editor.hpp>
#include <phon/gui/check_list.hpp>
#include <phon/gui/number_edit.hpp>
#include <phon/gui/boolean_edit.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

enum {
	IndexIsExactly = 0,
	IndexIsNot,
	IndexContains,
	IndexDoesntContain,
	IndexMatches,
	IndexDoesntMatch
};

QueryEditor::QueryEditor(Runtime &rt, QWidget *parent, int context_length) :
	QDialog(parent), runtime(rt)
{
	setupUi(context_length);
	setWindowFlags(Qt::Window);
}

void QueryEditor::setupUi(int context_length)
{
	auto layout = new QVBoxLayout;
	main_widget = new QWidget;

	auto name_layout = new QHBoxLayout;
	name_layout->addSpacing(10);
	name_layout->addWidget(new QLabel("Query name:"));
	auto query_label = QString("Query %1").arg(Query::current_id());
	query_name_edit = new QLineEdit(query_label);
	name_layout->addWidget(query_name_edit);
	name_layout->addStretch(1);
	auto help_button = new QPushButton(QIcon(":/icons/question.png"), QString());
	help_button->setToolTip(tr("Show help page"));
	name_layout->addWidget(help_button);

	search_box = new DefaultSearchBox(main_widget, context_length);
	search_box->postInitialize();

	auto scroll_area = new QScrollArea;
	scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scroll_area->setWidgetResizable(true);
	scroll_area->setFrameShape(QFrame::NoFrame); // remove borders
	scroll_area->setWidget(main_widget);

	auto file_box = createFileBox();
	auto property_box = createProperties();
	auto button_box = new QDialogButtonBox;
	button_box->setOrientation(Qt::Horizontal);
	button_box->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

	layout->addLayout(name_layout);
	layout->addWidget(search_box);
	layout->addWidget(file_box);
	layout->addWidget(property_box);
	layout->addStretch(1);
	main_widget->setLayout(layout);

	auto main_layout = new QVBoxLayout;
	main_layout->addWidget(scroll_area);
	main_layout->addWidget(button_box);

	setWindowTitle("Query editor");
	this->setLayout(main_layout);

	connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));
	connect(help_button, &QPushButton::clicked, this, &QueryEditor::showHelp);
}

QWidget * QueryEditor::createFileBox()
{
	auto widget = new QWidget(main_widget);
	auto layout = new QHBoxLayout; // main box layout

	// File list.
	auto project = Project::instance();
	auto file_box = new QGroupBox(tr("Annotation files"), main_widget);
	auto file_box_layout = new QVBoxLayout;

	Array<String> short_paths, full_paths;

	for (auto &annot : project->annotations()) {
		full_paths.append(annot->path());
	}
	std::sort(full_paths.begin(), full_paths.end());
	for (auto &path : full_paths) {
		short_paths.append(filesystem::base_name(path));
	}
	selected_files_box = new CheckList(this, short_paths, full_paths);

	// description
	auto desc_box = new QGroupBox(tr("File description"), main_widget);
	auto desc_outer_layout = new QVBoxLayout;
	auto desc_inner_layout = new QHBoxLayout;
	auto desc_label = new QLabel(tr("Description "));

	description_box = new QComboBox;
	description_box->addItem(tr("is exactly"));
	description_box->addItem(tr("is not"));
	description_box->addItem(tr("contains"));
	description_box->addItem(tr("doesn't contain"));
	description_box->addItem(tr("matches"));
	description_box->addItem(tr("doesn't match"));
	description_box->setCurrentIndex(2);
	description_line = new QLineEdit;
	description_line->setToolTip(tr("Filter files based on whether their description is (or not)\nan exact text or contains (or not) a regular expression."));

	desc_inner_layout->addWidget(desc_label);
	desc_inner_layout->addWidget(description_box);
	desc_inner_layout->addWidget(description_line);
	desc_outer_layout->addLayout(desc_inner_layout);
	desc_outer_layout->addStretch(1);
	desc_box->setLayout(desc_outer_layout);

	file_box_layout->addWidget(selected_files_box);
	file_box->setLayout(file_box_layout);

	layout->addWidget(file_box);
	layout->addWidget(desc_box);
	layout->setStretchFactor(file_box, 1);
	layout->setStretchFactor(desc_box, 2);
	layout->setContentsMargins(0, 0, 0, 0);
	widget->setLayout(layout);

	return widget;
}

void QueryEditor::accept()
{
	try
	{
		auto query = buildQuery();
		this->hide();
		emit queryReady(std::move(query));
	}
	catch (std::exception &e)
	{
		QMessageBox dlg(QMessageBox::Critical, tr("Invalid query"), e.what(),QMessageBox::NoButton, this);
		dlg.exec();
	}

}

QGroupBox *QueryEditor::createProperties()
{
	auto categories = Property::get_categories();
	auto property_box = new QGroupBox(tr("Properties"), main_widget);

	if (categories.empty())
	{
		auto layout = new QVBoxLayout;
		auto label = new QLabel(tr("The current project doesn't have any property."));
		label->setStyleSheet("QLineEdit { color: gray; font-style: italic }");
		layout->addStretch(1);
		layout->addWidget(label);
		layout->addStretch(1);
		property_box->setLayout(layout);

		return property_box;
	}

	auto layout = new QGridLayout;
	layout->setVerticalSpacing(10);
	layout->setHorizontalSpacing(10);
	layout->setContentsMargins(0, 0, 0, 0);

	int col = 0;
	int row = 0;

	for (auto &category : categories)
	{
		if (Property::is_boolean(category))
		{
			auto bool_box = new BooleanEdit(category);
			layout->addWidget(bool_box, row, col);

			boolean_properties.append(bool_box);
		}
		else if (Property::is_numeric(category))
		{
			auto num_box = new NumberEdit(category);
			layout->addWidget(num_box, row, col);

			numeric_properties.append(num_box);
		}
		else // String
		{
			Array<String> values;
			for (auto &v : Property::get_values(category))
			{
				values.append(v);
			}
			auto list_box = new CheckListBox(category, values);

			layout->addWidget(list_box, row, col);
			text_properties.append(list_box);
		}

		if (++col == properties_per_row)
		{
			row++;
			col = 0;
		}
	}
	property_box->setLayout(layout);

	return property_box;
}

AutoQuery QueryEditor::buildQuery()
{
	String label = query_name_edit->text().trimmed();
	if (label.empty()) label = "Untitled query";

	return std::make_shared<Query>(label, getAnnotations(), getMetadata(), getSearchTree());
}

Array<AutoMetaNode> QueryEditor::getMetadata()
{
	Array<AutoMetaNode> nodes;
	String desc_value = description_line->text();

	if (!desc_value.empty())
	{
		DescOperator desc_op;
		bool truth;

		switch (description_box->currentIndex())
		{
			case IndexIsExactly:
			case IndexIsNot:
				desc_op = DescOperator::Equals;
				break;
			case IndexContains:
			case IndexDoesntContain:
				desc_op = DescOperator::Contains;
				break;
			default:
				desc_op = DescOperator::Matches;
		}

		switch (description_box->currentIndex())
		{
			case IndexIsExactly:
			case IndexMatches:
			case IndexContains:
				truth = true;
				break;
			default:
				truth = false;
		}

		nodes.append(std::make_unique<DescriptionNode>(desc_value, desc_op, truth));
	}

	for (auto edit : boolean_properties)
	{
		auto value = edit->value();

		if (value.has_value())
		{
			nodes.append(std::make_unique<BooleanPropertyNode>(edit->description(), *value));
		}
	}

	for (auto edit : numeric_properties)
	{
		if (edit->hasValue())
		{
			nodes.append(std::make_unique<NumericPropertyNode>(edit->description(), edit->functor()));
		}
	}

	for (auto edit : text_properties)
	{
		auto labels = edit->checkedLabels();

		if (!labels.empty())
		{
			std::unordered_set<String> values;
			for (auto &label : labels) { values.insert(std::move(label)); }
			String category = edit->title();
			nodes.append(std::make_unique<TextPropertyNode>(category, std::move(values)));
		}
	}

	return nodes;
}

AnnotationSet QueryEditor::getAnnotations()
{
	auto project = Project::instance();
	auto paths = selected_files_box->checkedToolTips();
	AnnotationSet annotations;

	// If the user didn't check any file, get all the annotations in the project.
	if (paths.empty())
	{
		for (auto &annot : project->annotations())
		{
			annotations.insert(std::move(annot));
		}
	}
	else
	{
		for (auto &path : paths)
		{
			annotations.insert(std::dynamic_pointer_cast<Annotation>(project->get(path)));
		}
	}

	return annotations;
}

AutoSearchNode QueryEditor::getSearchTree()
{
	return search_box->buildSearchTree();
}

void QueryEditor::resurrect()
{
	auto query_label = QString("Query %1").arg(Query::current_id());
	query_name_edit->setText(query_label);
	show();
}

void QueryEditor::showHelp(bool)
{
	auto script = R"__(
	var page = phon.config.get_documentation_page("concordance.html")
	phon.show_documentation(page)
	)__";
	runtime.do_string(script);
}
} // namespace phonometrica