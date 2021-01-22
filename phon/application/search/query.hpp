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
 * Purpose: Query object. A query contains metadata nodes, which allow the user to filter annotations, and search      *
 * nodes, which allow them to extract concordances from the selected set of annotations.                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_HPP
#define PHONOMETRICA_QUERY_HPP

#include <memory>
#include <phon/application/protocol.hpp>
#include <phon/application/dataset.hpp>
#include <phon/application/search/meta_node.hpp>
#include <phon/application/search/search_node.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class Query final
{
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


	Signal<const String &> debug;

	Signal<> done;

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
