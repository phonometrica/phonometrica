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
 * Purpose: Query object. A query contains metadata nodes, which allow the user to filter annotations, and search     *
 * nodes, which allow them to extract concordances from the selected set of annotations.                              *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_HPP
#define PHONOMETRICA_QUERY_HPP

#include <memory>
#include <QObject>
#include <phon/application/search/meta_node.hpp>
#include <phon/application/search/search_node.hpp>

namespace phonometrica {

class Query final : public QObject
{
	Q_OBJECT

public:

	Query(AnnotationSet annotations, Array<AutoMetaNode> metadata, AutoSearchNode tree) :
		annotations(std::move(annotations)), metadata(std::move(metadata)), search_tree(std::move(tree))
	{ }

	Query(const Query &) = delete;

	~Query() = default;

	void execute();

signals:

	void debug(const String &);

	void done();

private:

	void filter_metadata();

	void filter_data();

	AnnotationSet annotations;
	Array<AutoMetaNode> metadata;
	AutoSearchNode search_tree;
};

// We shouldn't need the query to be shared, but we can't use a std::unique_ptr with Qt's signals and slots, since a
// signal may be connected to several slots, in which case the value needs to be copied.
using AutoQuery = std::shared_ptr<Query>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_HPP
