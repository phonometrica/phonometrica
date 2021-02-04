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
 * Created: 03/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: display a property in a query editor.                                                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROPERTYCTRL_HPP
#define PHONOMETRICA_PROPERTYCTRL_HPP

#include <optional>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/checklst.h>
#include <phon/application/property.hpp>

namespace phonometrica {

enum class Trilean
{
    False,
    True,
    Both
};


class PropertyCtrl final : public wxPanel
{
public:
    PropertyCtrl(wxWindow *parent, const String &category, const std::type_info &type);

    const std::type_info &GetType() const;

    bool GetBoolean() const;

    std::pair<double, double> GetNumericValue() const;

    int GetOperator() const;

    Array<String> GetTextValues() const;

    bool HasSelection() const;

	const String &GetCategory() const;

private:

	void OnCheckAllItems(wxCommandEvent &);

    const std::type_info &type;

    union
    {
        wxCheckListBox *checklist;
        wxChoice *choicelist;
    };

	wxTextCtrl *entry1 = nullptr, *entry2 = nullptr;

	String category;
};


} // namespace phonometrica

#endif // PHONOMETRICA_PROPERTYCTRL_HPP
