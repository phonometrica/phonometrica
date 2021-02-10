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
 * Created: 08/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: a Concordance represents the result of a query.                                                            *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_HPP
#define PHONOMETRICA_CONCORDANCE_HPP

#include <phon/application/dataset.hpp>
#include <phon/application/conc/match.hpp>

namespace phonometrica {

class Concordance : public Dataset
{
public:

	enum class Context
	{
		None,   // no context
		Labels, // labels from surrounding events
		KWIC    // keyword in context
	};

	Concordance(intptr_t target_count, Context ctx, Array<AutoMatch> matches, VFolder *parent, const String &path = String());

	const char *class_name() const override;

	bool is_concordance() const override { return true; }

	intptr_t target_count() const { return m_target_count; }

	String get_header(intptr_t j) const override;

	String get_cell(intptr_t i, intptr_t j) const override;

	void set_cell(intptr_t i, intptr_t j, const String &value);

	intptr_t row_count() const override;

	intptr_t column_count() const override;

	bool empty() const override;

protected:



	void load() override;

	void write() override;

	Array<AutoMatch> m_matches;

	// Left and right context
	Array<std::pair<String,String>> m_context;

	intptr_t m_target_count;

	Context m_context_type;
};

using AutoConcordance = std::shared_ptr<Concordance>;

} // namespace phonometrica

#endif // PHONOMETRICA_CONCORDANCE_HPP
