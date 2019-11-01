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
 * Created: 31/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QLayout>
#include <QHeaderView>
#include <phon/application/audio_player.hpp>
#include <phon/application/praat.hpp>
#include <phon/application/settings.hpp>
#include <phon/gui/font.hpp>
#include <phon/gui/toolbar.hpp>
#include <phon/gui/views/acoustic_query_view.hpp>
#include <phon/application/search/query.hpp>

namespace phonometrica {

AcousticQueryView::AcousticQueryView(QWidget *parent, Runtime &rt, AutoQueryTable data, int t) :
	View(parent), runtime(rt), m_data(std::move(data))
{
	auto type = static_cast<Query::Type>(t);

	if (type == Query::Type::Formants)
		m_type = Measurement::Formants;
	else if (type == Query::Type::Pitch)
		m_type = Measurement::Pitch;
	else if (type == Query::Type::Intensity)
		m_type = Measurement::Intensity;
	else
		throw error("[Internal error] Invalid query type in acoustic query view constructor");

	setupUi();
}

void AcousticQueryView::setupUi()
{
	auto layout = new QVBoxLayout;
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

}

void AcousticQueryView::fillTable()
{
	QFont font = get_monospace_font();
	auto bold_font = font;
	bold_font.setBold(true);
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
			// TODO: make formants editable
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			item->setFont(font);
			item->setTextAlignment(Qt::AlignCenter);

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


} // namespace phonometrica
