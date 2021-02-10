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
 * Purpose: Display a concordance (i.e. the result of a query).                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CONCORDANCE_VIEW_HPP
#define PHONOMETRICA_CONCORDANCE_VIEW_HPP

#include <phon/gui/views/view.hpp>
#include <phon/gui/ctrl/concordance_controller.hpp>

namespace phonometrica {

class ConcordanceView : public View
{
public:

	ConcordanceView(wxWindow *parent, AutoConcordance conc);

	virtual bool IsModified() const override;

	virtual void DiscardChanges() override;

	virtual wxString GetLabel() const override;

	virtual String GetPath() const override;

protected:

	wxGrid *m_grid;

	AutoConcordance m_conc;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CONCORDANCE_VIEW_HPP
