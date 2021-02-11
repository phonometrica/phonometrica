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
 * Created: 10/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Controller for a concordance.                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_CONTROLLER_HPP
#define PHONOMETRICA_CONCORDANCE_CONTROLLER_HPP

#include <phon/gui/ctrl/data_controller.hpp>
#include <phon/application/conc/concordance.hpp>

namespace phonometrica {

class ConcCellAttrProvider: public wxGridCellAttrProvider
{
public:
	ConcCellAttrProvider(const wxFont &match_font);

	~ConcCellAttrProvider() override
	{
		m_match_attr->DecRef();
		m_normal_attr->DecRef();
	}

	wxGridCellAttr *GetAttr(int row, int col, wxGridCellAttr::wxAttrKind  kind) const override;

private:
	wxGridCellAttr *m_match_attr, *m_normal_attr;
};


//---------------------------------------------------------------------------------------------------------------------

class ConcordanceController : public DataController
{
public:

	explicit ConcordanceController(AutoConcordance conc);

	int GetNumberRows() override;

	int GetNumberCols() override;

	wxString GetValue(int row, int col) override;

	void SetValue(int row, int col, const wxString &value) override;

	wxString GetColLabelValue(int col) override;

protected:

	AutoConcordance m_conc;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_CONTROLLER_HPP
