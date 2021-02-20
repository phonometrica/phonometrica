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
 * Created: 19/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: scrollbar that displays the whole sound file in a sound view or annotation view.                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVE_BAR_HPP
#define PHONOMETRICA_WAVE_BAR_HPP

#include <wx/dcclient.h>
#include <wx/window.h>
#include <wx/graphics.h>
#include <phon/application/sound.hpp>

namespace phonometrica {

class WaveBar final : public wxWindow
{
public:

	WaveBar(wxWindow *parent, const Handle <Sound> &snd);


private:

	void OnPaint(wxPaintEvent &);

	void Render(wxPaintDC &dc);

	void MakePath(wxGraphicsPath &path);

	double SampleToYPos(double s) const;;

	Handle<Sound> m_sound;

	double raw_magnitude = 0.0;

	Array<std::pair<double,double>> m_cache;
};

} // namespace phonometrica



#endif // PHONOMETRICA_WAVE_BAR_HPP
