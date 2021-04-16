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
 * Created: 15/04/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Dialog used to select visible channels in sound and annotation views.                                      *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_CHANNEL_DIALOG_HPP
#define PHONOMETRICA_CHANNEL_DIALOG_HPP

#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/checkbox.h>

namespace phonometrica {

class ChannelDialog final : public wxDialog
{
public:

	ChannelDialog(wxWindow *parent, int nchannel, const std::vector<int> &visible_channels);

	// Returns an empty vector for averaging.
	std::vector<int> GetSelectedChannels() const;

private:

	void OnChoiceChanged(wxCommandEvent &);

	wxChoice *selector;

	std::vector<wxCheckBox*> channels;
};

} // namespace phonometrica



#endif // PHONOMETRICA_CHANNEL_DIALOG_HPP
