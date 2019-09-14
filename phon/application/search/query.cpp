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
 * Created: 07/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <QProgressDialog>
#include <phon/application/search/query.hpp>

namespace phonometrica {

int Query::the_id = 1;

Query::Query(const String &label, AnnotationSet annotations, Array<AutoMetaNode> metadata, AutoSearchNode tree) :
		m_label(label), annotations(std::move(annotations)), metadata(std::move(metadata)), search_tree(std::move(tree))
{
	m_id = the_id++;
}

AutoDataset Query::execute()
{
	filter_metadata();
	return std::make_shared<QueryDataset>(filter_data(), m_label);
}

void Query::filter_metadata()
{
	for (auto &node : metadata)
	{
		annotations = node->filter(annotations);
	}

#if PHON_DEBUG_CONSOLE
	emit debug("METADATA:");

	for (auto &annot : annotations)
	{
		emit debug(annot->path());
	}
#endif
}

QueryMatchList Query::filter_data()
{
#if PHON_DEBUG_CONSOLE
	emit debug("CONSTRAINTS:");
	emit debug(search_tree->to_string());
#endif

	QueryMatchList results;
	int count = annotations.size();

	QProgressDialog progress("Executing query", "Cancel query", 0, count, nullptr);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(0);

	int i = 0;
	for (auto &annot : annotations)
	{
		progress.setValue(i++);
		if (progress.wasCanceled()) return QueryMatchList();

		annot->open();
		auto matches = search_tree->filter(annot, QueryMatchSet());
		auto new_size = results.size() + matches.size();
		results.reserve(new_size);

		for (auto it = matches.begin(); it != matches.end(); /* nothing */)
		{
			results.append(std::move(matches.extract(it++).value()));
		}
	}
	progress.setValue(count);

#if PHON_DEBUG_CONSOLE
	emit done();
#endif

	return results;
}

int Query::current_id()
{
	return the_id;
}

} // namespace phonometrica