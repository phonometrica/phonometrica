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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <QProgressDialog>
#include <phon/application/search/query.hpp>

namespace phonometrica {

int Query::the_id = 1;

Query::Query(AutoProtocol p, const String &label, AnnotationSet annotations, Array<AutoMetaNode> metadata, AutoSearchNode tree) :
		m_protocol(std::move(p)), m_label(label), annotations(std::move(annotations)), metadata(std::move(metadata)),
		search_tree(std::move(tree))
{
	m_id = the_id++;
}

AutoDataset Query::execute()
{
	filter_metadata();
	return std::make_shared<QueryTable>(m_protocol, filter_data(), m_label);
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

#ifdef PHON_TIMING
	auto first_time = clock();
#endif

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