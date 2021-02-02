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
 * Purpose: Editor for text concordances in annotations.                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_TEXT_QUERY_EDITOR_HPP
#define PHONOMETRICA_TEXT_QUERY_EDITOR_HPP

#include <phon/gui/conc/query_editor.hpp>
#include <phon/application/conc/text_query.hpp>

namespace phonometrica {

class TextQueryEditor final : public QueryEditor
{
public:

	explicit TextQueryEditor(wxWindow *parent);

	AutoQuery GetQuery() const override;

private:

	wxWindow *MakeSearchPanel(wxWindow *parent) override;

	void ParseQuery() override;

	AutoTextQuery query;
};

} // namespace phonometrica



#endif // PHONOMETRICA_TEXT_QUERY_EDITOR_HPP
