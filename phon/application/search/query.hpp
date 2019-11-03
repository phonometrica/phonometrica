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
 * Purpose: Query object. A query contains metadata nodes, which allow the user to filter annotations, and search      *
 * nodes, which allow them to extract concordances from the selected set of annotations.                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_HPP
#define PHONOMETRICA_QUERY_HPP

#include <memory>
#include <QObject>
#include <phon/application/protocol.hpp>
#include <phon/application/dataset.hpp>
#include <phon/application/search/meta_node.hpp>
#include <phon/application/search/search_node.hpp>

namespace phonometrica {

class Query final : public QObject
{
	Q_OBJECT

public:

	using Type = SearchNode::Type;
	using Settings = SearchNode::Settings;

	Query(AutoProtocol p, const String &label, AnnotationSet annotations, Array <AutoMetaNode> metadata,
	      AutoSearchNode tree, std::shared_ptr<Settings> settings);

	Query(const Query &) = delete;

	~Query() = default;

	AutoDataset execute();

	intptr_t id() const { return m_id; }

	static int current_id();

	String label() const { return m_label; }

	Type type() const { return m_settings->type; }

signals:

	void debug(const String &);

	void done();

private:

	void filter_metadata();

	QueryMatchList filter_data();

	static int the_id;

	AutoProtocol m_protocol; // may be null

	std::shared_ptr<Settings> m_settings;

	String m_label;

	AnnotationSet annotations;

	Array<AutoMetaNode> metadata;

	AutoSearchNode search_tree;

	intptr_t m_id;
};

// We shouldn't need the query to be shared, but we can't use a std::unique_ptr with Qt's signals and slots, since a
// signal may be connected to several slots, in which case the value needs to be copied.
using AutoQuery = std::shared_ptr<Query>;

using AutoQuerySettings = std::shared_ptr<Query::Settings>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_HPP
