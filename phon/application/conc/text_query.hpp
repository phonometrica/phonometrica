/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Query for text concordances in annotations.                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_QUERY_HPP
#define PHONOMETRICA_TEXT_QUERY_HPP

#include <phon/application/conc/query.hpp>

namespace phonometrica {

class TextQuery final : public Query
{
public:

	TextQuery(VFolder *parent, const String &path);

	const char *class_name() const override { return "TextQuery"; }

	// Note: subclasses must override this method and return false
	bool is_text_query() const override { return true; }

private:

	void load() override;

	void write() override;

	void metaconstraints_from_xml(xml_node node);

	void constraints_from_xml(xml_node node);
};

using AutoTextQuery = std::shared_ptr<TextQuery>;

} // namespace phonometrica

#endif // PHONOMETRICA_TEXT_QUERY_HPP
