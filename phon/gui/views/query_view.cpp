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
 * Created: 14/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLabel>
#include <QKeyEvent>
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
#include <phon/gui/toolbar.hpp>
#include <phon/gui/views/query_view.hpp>
#include <phon/gui/bookmark_editor.hpp>
#include <phon/gui/font.hpp>
#include <phon/gui/popup_text_editor.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

QueryView::QueryView(QWidget *parent, Runtime &rt, AutoQueryTable data) :
    View(parent), runtime(rt), m_data(std::move(data))
{
	bool use_praat = m_data->has_textgrid();
	auto layout = new QVBoxLayout;

	// Toolbar.
	auto toolbar = new Toolbar;
	play_action = toolbar->addAction(QIcon(":/icons/play.png"), tr("Play selected match"));
	stop_action = toolbar->addAction(QIcon(":/icons/stop.png"), tr("Stop selected match"));
	auto view_action = toolbar->addAction(QIcon(":/icons/eye.png"), tr("View match in annotation"));

	if (use_praat)
	{
		auto praat_action = toolbar->addAction(QIcon(":/icons/praat.png"), tr("Open match in Praat"));

		auto open_in_praat = [=](bool) {
			auto index = m_table->currentIndex();
			if (index.isValid()) openMatchInPraat(index.row());
		};

		connect(praat_action, &QAction::triggered, this, open_in_praat);
	}

	edit_action = toolbar->addAction(QIcon(":/icons/edit_row.png"), tr("Edit event label"));
	enableQueryButtons(false);

	auto csv_action = toolbar->addAction(QIcon(":/icons/export_csv.png"), tr("Export to tab-separated value file"));

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

#if PHON_MACOS
    toolbar->addStretch();
#endif

    connect(play_action, &QAction::triggered, play);
    connect(stop_action, &QAction::triggered, stop);
    connect(edit_action, &QAction::triggered, edit);
    connect(csv_action, &QAction::triggered, this, &QueryView::exportToCsv);
    layout->addWidget(toolbar);

    auto column_menu = new QMenu;
	info_action = new QAction(tr("Show file information"), this);
    info_action->setCheckable(true);
    match_action = new QAction(tr("Split match"), this);
    match_action->setCheckable(true);
    match_action->setChecked(m_data->has_protocol());
    match_action->setEnabled(m_data->has_protocol());
	context_action = new QAction(tr("Show match context"), this);
	context_action->setCheckable(true);
	property_action = new QAction(tr("Show properties"), this);
	property_action->setCheckable(true);
	column_menu->addAction(match_action);
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

	auto bookmark_action = toolbar->addAction(QIcon(":/icons/favorite.png"), tr("Bookmark match"));

#if PHON_MACOS || PHON_WINDOWS
	toolbar->setMaximumHeight(30);
#endif


	// Match result layout

	auto label = QString("<b>%1 matches").arg(m_data->row_count());
	layout->addWidget(new QLabel(label));

	auto view_event = [=](bool) {
		auto index = m_table->currentIndex();
		if (index.isValid()) openInAnnotation(index.row());
	};

	auto bookmark_match = [=](bool) {
		auto index = m_table->currentIndex();
		if (index.isValid()) bookmarkMatch(index.row());
	};

	connect(info_action, &QAction::triggered, this, &QueryView::refreshTable);
	connect(match_action, &QAction::triggered, this, &QueryView::refreshTable);
	connect(context_action, &QAction::triggered, this, &QueryView::refreshTable);
	connect(property_action, &QAction::triggered, this, &QueryView::refreshTable);
	connect(view_action, &QAction::triggered, this, view_event);
	connect(bookmark_action, &QAction::triggered, this, &QueryView::bookmarkMatch);

	m_data->set_flags(getQueryFlags());

	int nrow = m_data->row_count();
	int ncol = m_data->column_count();
	m_table = new QTableWidget(nrow, ncol, this);
	// FIXME: on linux, the cell in the selected row becomes white instead of transparent.
	//m_table->setStyleSheet("QTableWidget::item:hover { background-color: transparent; }");
	m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_table->horizontalHeader()->setHighlightSections(false);

	layout->setContentsMargins(10, 5, 10, 10);
	layout->addWidget(m_table);
	setLayout(layout);
	fillTable();

	m_table->setContextMenuPolicy(Qt::CustomContextMenu);
	m_table->setFocusPolicy(Qt::StrongFocus);
	connect(m_table, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(provideContextMenu(const QPoint &)));
	connect(m_table, &QTableWidget::cellClicked, this, &QueryView::onCellClicked);
	connect(m_table, &QTableWidget::cellDoubleClicked, this, &QueryView::onCellDoubleClicked);
}

bool QueryView::save()
{
	return true;
	// TODO: implement QueryView::save()
}

void QueryView::fillTable()
{
	QFont font = get_monospace_font();
	auto bold_font = font;
	bold_font.setBold(true);
	QColor red("red");
	QStringList vheaders, hheaders;
	vheaders.reserve(m_table->rowCount());
	hheaders.reserve(m_table->columnCount());

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
			int jj = (int) m_data->adjust_column(j);
			if (jj == 2)
			{
				item->setTextAlignment(Qt::AlignCenter);
			}
			if (jj == 5)
			{
				// FIXME: there seems to be a bug on Windows whereby the text is slightly higher than the other cells in the row
				//  if we right-align the cell.
#if !PHON_WINDOWS
				item->setTextAlignment(Qt::AlignRight);
#endif
			}
			else if (isMatchCell(jj))
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

#if PHON_WINDOWS
	m_table->horizontalHeader()->setFont(bold_font);
	m_table->verticalHeader()->setFont(bold_font);
#endif

	m_table->resizeColumnsToContents();
}

int QueryView::getQueryFlags()
{
	int flags = QueryTable::ShowNothing;

	if (match_action->isChecked())
		flags |= QueryTable::ShowFields;
	if (info_action->isChecked())
		flags |= QueryTable::ShowFileInfo;
	if (context_action->isChecked())
		flags |= QueryTable::ShowMatchContext;
	if (property_action->isChecked())
		flags |= QueryTable::ShowProperties;

	return flags;
}

void QueryView::refreshTable(bool)
{
	m_table->clear();
	m_data->set_flags(getQueryFlags());
	int ncol = m_data->column_count();
	m_table->setColumnCount(ncol);
	fillTable();
}

void QueryView::onCellDoubleClicked(int i, int)
{
	enableQueryButtons(true);
	playMatch(i);
}

void QueryView::playMatch(int i)
{
	auto &match = m_data->get_match(i+1);
	auto sound = match->annotation()->sound();
	if (!sound) return;
	stopPlayer();
	player = std::make_unique<AudioPlayer>(runtime, this, sound->light_data());
	double from = match->start_time();
	double to = match->end_time();
	player->play(from, to);
}

void QueryView::onCellClicked(int i, int j)
{
	enableQueryButtons(true);
}

void QueryView::keyPressEvent(QKeyEvent *event)
{
	auto index = m_table->currentIndex();

	if (event->key() == Qt::Key_Space)
	{
		if (index.isValid())
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
		if (index.isValid())
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

void QueryView::editEvent(int i)
{
	auto &match = m_data->get_match(i+1);
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

void QueryView::enableQueryButtons(bool enable)
{
	play_action->setEnabled(enable);
	edit_action->setEnabled(enable);
	stop_action->setEnabled(enable);
}

void QueryView::openInAnnotation(int i)
{
	auto &match = m_data->get_match(i+1);
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

void QueryView::openMatchInPraat(int i)
{
	auto &match = m_data->get_match(i+1);
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
	
	try {
		praat::open_interval(match->layer_index(), match->start_time(), annot->path(), sound_path);		
	}
	catch (std::exception &e)
	{
		QMessageBox msg(QMessageBox::Critical, tr("Cannot open interval in Praat"), e.what());
		msg.exec();
	}
}

void QueryView::stopPlayer()
{
	if (player)
	{
		player->stop();
		player = nullptr;
	}
}

void QueryView::exportToCsv(bool)
{
	String name = m_data->label();
	name.append(".csv");
	String dir = filesystem::join(Settings::get_last_directory(runtime), name);
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

void QueryView::provideContextMenu(const QPoint &pos)
{
	auto index = m_table->currentIndex();
	if (!index.isValid()) return;
	int row = index.row();
	auto &event = m_data->get_match(row + 1);
	auto &annot = event->annotation();
	QMenu menu;

	auto play_action = menu.addAction(tr("Play selection"));
	menu.addSeparator();
	auto annot_action = menu.addAction(tr("Open in annotation"));

	if (annot->is_textgrid())
	{
		auto praat_action = menu.addAction(tr("Open in Praat"));
		connect(praat_action, &QAction::triggered, [=](bool) {
			openMatchInPraat(row);
		});
	}

	menu.addSeparator();
	auto event_action = menu.addAction(tr("Edit event text"));
	menu.addSeparator();
	auto bookmark_action = menu.addAction(tr("Bookmark match"));

	connect(play_action, &QAction::triggered, [=](bool) {
		playMatch(row);
	});

	connect(annot_action, &QAction::triggered, [=](bool) {
		openInAnnotation(row);
	});

	connect(event_action, &QAction::triggered, [=](bool) {
		editEvent(row);
	});

	connect(bookmark_action, &QAction::triggered, [=](bool) {
		bookmarkMatch(row);
	});

	menu.exec(m_table->mapToGlobal(pos));
}

bool QueryView::isMatchCell(int jj) const
{
	const int match_pos = 6;
	if (jj < match_pos) return false;

	return jj == match_pos || (m_data->has_split_fields() && jj - match_pos < m_data->field_count());
}

void QueryView::bookmarkMatch(int i)
{
	BookmarkEditor ed(this);

	if (ed.exec() == QDialog::Accepted)
	{
		String title = ed.title();

		if (title.empty())
		{
			QMessageBox msg(QMessageBox::Warning, tr("Cannot create bookmark"), tr("This bookmark needs a title!"));
			return;
		}

		String notes = ed.notes();
		auto &match = m_data->get_match(i+1);
		Project::instance()->add_bookmark(match->to_bookmark(title, notes));
	}
}

} // namespace phonometrica