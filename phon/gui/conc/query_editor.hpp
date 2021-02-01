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
 * Purpose: Base class for all query editors.                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_EDITOR_HPP
#define PHONOMETRICA_QUERY_EDITOR_HPP

#include <wx/dialog.h>
#include <phon/application/conc/query.hpp>

namespace phonometrica {

class QueryEditor : public wxDialog
{
public:

	QueryEditor(wxWindow *parent, const wxString &title);

	QueryEditor(wxWindow *parent, const wxString &title, const AutoQuery &query);

	void Prepare();

	intptr_t GenerateId() const { return id++; }

	void Execute();

protected:

	void SetHeader();

	virtual void SetSearchPanel() = 0;

	void SetMetadata();

	wxBoxSizer *MakeButtons(wxWindow *parent);

	AutoQuery m_query;

	bool prepared = false;

	static intptr_t id;
};

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_EDITOR_HPP
