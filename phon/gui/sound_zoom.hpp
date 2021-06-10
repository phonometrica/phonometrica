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
 * Created: 20/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Connects the wavebar to the sound plots in a sound view or the annotation layers in an annotation view.    *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_SOUND_ZOOM_HPP
#define PHONOMETRICA_SOUND_ZOOM_HPP

#include <wx/window.h>
#include <wx/dcclient.h>
#include <phon/gui/helpers.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class SoundZoom final : public wxWindow
{
public:

	SoundZoom(wxWindow *parent);

	void OnSetSelection(PixelSelection sel);

private:

	bool HasSelection() const { return m_sel.first >= 0; }

	void OnPaint(wxPaintEvent &);

    void OnEraseBackground(wxEraseEvent &);

	PixelSelection m_sel;
};

} // namespace phonometrica



#endif // PHONOMETRICA_SOUND_ZOOM_HPP
