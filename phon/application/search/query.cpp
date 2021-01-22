/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/progdlg.h>
#include <phon/application/search/query.hpp>
#include <phon/application/query_table.hpp>

namespace phonometrica {

int Query::the_id = 1;

Query::Query(AutoProtocol p, const String &label, AnnotationSet annotations, Array <AutoMetaNode> metadata,
             AutoSearchNode tree, std::shared_ptr<Query::Settings> settings) :
		m_protocol(std::move(p)), m_label(label), annotations(std::move(annotations)), metadata(std::move(metadata)),
		search_tree(std::move(tree)), m_settings(std::move(settings))
{
	m_id = the_id++;
}

AutoDataset Query::execute()
{
	filter_metadata();
	return std::make_shared<QueryTable>(m_protocol, filter_data(), m_label, m_settings);
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

	wxProgressDialog progress(_("Executing query"), _("Processing files"), count);

	int i = 0;

#ifdef PHON_TIMING
	auto first_time = clock();
#endif

	for (auto &annot : annotations)
	{
		// Cancelled by user?
		if (!progress.Update(i++)) {
			 return QueryMatchList();
		}
		annot->open();
		auto matches = search_tree->filter(m_settings.get(), annot, QueryMatchSet());
		auto new_size = results.size() + matches.size();
		results.reserve(new_size);

		for (auto it = matches.begin(); it != matches.end(); /* nothing */)
		{
#if PHON_MACOS
            results.append(*it); it++;
#else
		    results.append(std::move(matches.extract(it++).value()));
#endif
		}
	}

#ifdef PHON_TIMING
	auto last_time = clock();
	auto total = double(last_time-first_time) * 1000 / CLOCKS_PER_SEC;
	std::cerr << "Total loading time for " << annotations.size() << " annotations: " << total << " ms\n";
	std::cerr << "Average per annotation: " << (total/annotations.size()) << " ms\n";
#endif

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