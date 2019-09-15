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
 * Created: 14/03/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QLabel>
#include <QKeyEvent>
#include <QToolBar>
#include <QToolButton>
#include <QHeaderView>
#include <QFontDatabase>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QLayout>
#include <phon/application/project.hpp>
#include <phon/application/audio_player.hpp>
#include <phon/application/praat.hpp>
#include <phon/application/settings.hpp>
#include <phon/gui/views/data_view.hpp>
#include <phon/gui/popup_text_editor.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

DataView::DataView(QWidget *parent, Runtime &rt, std::shared_ptr<Dataset> data) :
    View(parent), runtime(rt), m_data(std::move(data))
{
	auto layout = new QVBoxLayout;

	if (m_data->is_query_dataset())
	{
		// Toolbar.
		auto toolbar = new QToolBar;
		play_action = toolbar->addAction(QIcon(":/icons/play.png"), tr("Play selected match"));
		stop_action = toolbar->addAction(QIcon(":/icons/stop.png"), tr("Stop selected match"));
		auto view_action = toolbar->addAction(QIcon(":/icons/eye.png"), tr("View match in annotation"));
		auto praat_action = toolbar->addAction(QIcon(":/icons/praat.png"), tr("Open match in Praat"));
		auto praat_path = Settings::get_string(runtime, "praat_path");
		praat_action->setEnabled(filesystem::exists(praat_path));

		edit_action = toolbar->addAction(QIcon(":/icons/edit_row.png"), tr("Edit event label"));
		enableQueryButtons(false);

		auto csv_action = toolbar->addAction(QIcon(":/icons/export_csv.png"), tr("Export to tab-separated value file"));

#if PHON_MACOS
		toolbar->setMaximumHeight(30);
        toolbar->setStyleSheet("QToolBar{spacing:0px;}");
	#endif

        auto play = [=](bool) {
        	auto index = m_table->currentIndex();
        	if (index.isValid()) onCellDoubleClicked(index.row(), index.column());
        };

        auto stop = [=](bool) {
        	this->stopPlayer();
        };

        auto edit = [=](bool) {
        	auto index = m_table->currentIndex();
        	if (index.isValid()) editEvent(index.row());
        };

        connect(play_action, &QAction::triggered, play);
        connect(stop_action, &QAction::triggered, stop);
        connect(edit_action, &QAction::triggered, edit);
        connect(csv_action, &QAction::triggered, this, &DataView::exportToCsv);
        layout->addWidget(toolbar);

        auto column_menu = new QMenu;
		info_action = new QAction(tr("Show file information"), this);
        info_action->setCheckable(true);
		context_action = new QAction(tr("Show match context"), this);
		context_action->setCheckable(true);
		property_action = new QAction(tr("Show properties"), this);
		property_action->setCheckable(true);
		column_menu->addAction(info_action);
		column_menu->addAction(context_action);
		column_menu->addAction(property_action);
		info_action->setChecked(true);
		context_action->setChecked(true);

		auto column_button = new QToolButton;
		column_button->setIcon(QIcon(":/icons/select_column.png"));
		column_button->setToolTip(tr("Show/hide columns"));
		column_button->setMenu(column_menu);
		column_button->setPopupMode(QToolButton::InstantPopup);
		toolbar->addWidget(column_button);

		// Match result layout

		auto label = QString("<b>%1 matches").arg(m_data->row_count());
		layout->addWidget(new QLabel(label));

		auto view_event = [=](bool) {
			auto index = m_table->currentIndex();
			if (index.isValid()) openInAnnotation(index.row());
		};

		auto open_in_praat = [=](bool) {
			auto index = m_table->currentIndex();
			if (index.isValid()) openMatchInPraat(index.row());
		};

		connect(info_action, &QAction::triggered, this, &DataView::refreshTable);
		connect(context_action, &QAction::triggered, this, &DataView::refreshTable);
		connect(property_action, &QAction::triggered, this, &DataView::refreshTable);
		connect(view_action, &QAction::triggered, this, view_event);
		connect(praat_action, &QAction::triggered, this, open_in_praat);

		getQueryDataset()->set_flags(getQueryFlags());
	}

	int nrow = m_data->row_count();
	int ncol = m_data->column_count();
	m_table = new QTableWidget(nrow, ncol, this);
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->horizontalHeader()->setHighlightSections(false);

	layout->addWidget(m_table);
	setLayout(layout);
	fill_table();
}

void DataView::save()
{
	// TODO: implement DataView::save()
}

void DataView::fill_table()
{
	//auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	QFont font("Noto Sans Mono");
	//font.setStyleHint(QFont::Monospace);
	auto bold_font = font;
	bold_font.setBold(true);
	QColor red("red");
	QStringList vheaders, hheaders;
	vheaders.reserve(m_table->rowCount());
	hheaders.reserve(m_table->columnCount());
	auto query_set = dynamic_cast<QueryTable*>(m_data.get());

	for (int j = 1; j <= m_table->columnCount(); j++)
	{
		hheaders << m_data->get_header(j);
	}

	for (int i = 1; i <= m_table->rowCount(); i++)
	{
		vheaders << QString("%1").arg(i);

		for (int j = 1; j <= m_table->columnCount(); j++)
		{
			auto label = m_data->get_cell(i, j);
			auto item = new QTableWidgetItem(label);
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			item->setFont(font);
			int jj = (int) query_set->adjust_column(j);
			if (jj == 2)
			{
				item->setTextAlignment(Qt::AlignCenter);
			}
			if (jj == 5)
			{
				item->setTextAlignment(Qt::AlignRight);
			}
			else if (jj == 6)
			{
				item->setTextColor(red);
				item->setFont(bold_font);
				item->setTextAlignment(Qt::AlignCenter);
			}
			m_table->setItem(i-1, j-1, item);
		}
	}

	m_table->setHorizontalHeaderLabels(hheaders);
	m_table->setVerticalHeaderLabels(vheaders);
	m_table->resizeColumnsToContents();

	connect(m_table, &QTableWidget::cellClicked, this, &DataView::onCellClicked);
	connect(m_table, &QTableWidget::cellDoubleClicked, this, &DataView::onCellDoubleClicked);
}

int DataView::getQueryFlags()
{
	int flags = 0;

	if (m_data->is_query_dataset())
	{
		if (info_action->isChecked())
			flags |= QueryTable::ShowFileInfo;
		if (context_action->isChecked())
			flags |= QueryTable::ShowMatchContext;
		if (property_action->isChecked())
			flags |= QueryTable::ShowProperties;
	}

	return flags;
}

void DataView::refreshTable(bool)
{
	m_table->clear();
	dynamic_cast<QueryTable*>(m_data.get())->set_flags(getQueryFlags());
	int ncol = m_data->column_count();
	m_table->setColumnCount(ncol);
	fill_table();
}

void DataView::onCellDoubleClicked(int i, int)
{
	if (m_data->is_query_dataset())
	{
		enableQueryButtons(true);
		playMatch(i);
	}
}

void DataView::playMatch(int i)
{
	auto query_set = getQueryDataset();
	auto &match = query_set->get_match(i+1);
	auto sound = match->annotation()->sound();
	if (!sound) return;
	if (player) player->stop();
	auto h = sound->handle();
	player = std::make_unique<AudioPlayer>(runtime, this, sound->light_data());
	double from = match->start_time();
	double to = match->end_time();
	player->play(from, to);
}

void DataView::onCellClicked(int i, int j)
{
	if (m_data->is_query_dataset())
	{
		enableQueryButtons(true);
	}
}

void DataView::keyPressEvent(QKeyEvent *event)
{
	if (!m_data->is_query_dataset())
	{
		event->ignore();
		return;
	}

	auto index = m_table->currentIndex();

	if (event->key() == Qt::Key_Space)
	{
		if (index.row() >= 0)
		{
			onCellDoubleClicked(index.row(), index.column());
		}
		event->accept();
	}
	else if (event->key() == Qt::Key_Escape)
	{
		stopPlayer();
		event->accept();
	}
	else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		if (index.row() >= 0)
		{
			editEvent(index.row());
		}
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void DataView::editEvent(int i)
{
	auto query_set = getQueryDataset();
	auto &match = query_set->get_match(i+1);
	auto event = match->event();
	QString text = event->text();
	int y = m_table->rowViewportPosition(i) + 100;
	QPoint point(width() / 2, y);

	PopupTextEditor editor(text, mapToGlobal(point), this);

	if (editor.exec() == QDialog::Accepted)
	{
		auto new_text = editor.text();
		if (new_text != text)
		{
			match->annotation()->set_event_text(event, new_text);
			emit modified();
		}
	}
}

void DataView::enableQueryButtons(bool enable)
{
	play_action->setEnabled(enable);
	edit_action->setEnabled(enable);
	stop_action->setEnabled(enable);
}

void DataView::openInAnnotation(int i)
{
	auto query_set = getQueryDataset();
	auto &match = query_set->get_match(i+1);
	auto annot = match->annotation();

	if (annot->has_sound())
	{
		double from = match->start_time();
		double to = match->end_time();

		emit openAnnotation(std::move(annot), match->layer_index(), from, to);
	}
	else
	{
		QMessageBox dlg(QMessageBox::Warning, tr("Cannot open annotation"), tr("The annotation is not bound to any sound file!"));
		dlg.exec();
	}

}

void DataView::openMatchInPraat(int i)
{
	auto query_set = getQueryDataset();
	auto &match = query_set->get_match(i+1);
	auto &annot = match->annotation();

	if (!annot->is_textgrid())
	{
		QMessageBox msg(QMessageBox::Critical, tr("Cannot open in Praat"),
				tr("The file in which this match was found is not a TextGrid!"));
		msg.exec();
		return;
	}

	String sound_path;

	if (annot->has_sound())
	{
		sound_path = annot->sound()->path();
	}

	praat::open_interval(match->layer_index(), match->start_time(), annot->path(), sound_path);
}

void DataView::stopPlayer()
{
	if (player)
	{
		player->stop();
		player = nullptr;
	}
}

void DataView::exportToCsv(bool)
{
	String dir = Settings::get_last_directory(runtime);
	auto p = QFileDialog::getSaveFileName(this, tr("Save as CSV file..."), dir);
	if (p.isNull()) return;
	String path = p;
	if (!path.ends_with(".csv")) path.append(".csv");
	m_data->to_csv(path);

	auto reply = QMessageBox::question(this, tr("Import file?"), tr("Would you like to import this dataset into the current project?"),
			QMessageBox::Yes|QMessageBox::No);

	if (reply == QMessageBox::Yes)
	{
		auto project = Project::instance();
		project->import_file(path);
		emit project->notify_update();
	}
}

} // namespace phonometrica