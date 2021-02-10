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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/ctrl/concordance_controller.hpp>

namespace phonometrica {

ConcordanceController::ConcordanceController(AutoConcordance conc) :
	DataController(), m_conc(std::move(conc))
{

}

int ConcordanceController::GetNumberRows()
{
	return (int) m_conc->row_count();
}

int ConcordanceController::GetNumberCols()
{
	return (int) m_conc->column_count();
}

wxString ConcordanceController::GetValue(int row, int col)
{
	return m_conc->get_cell(row, col);
}

void ConcordanceController::SetValue(int row, int col, const wxString &value)
{
	m_conc->set_cell(row, col, value);
}
} // namespace phonometrica
