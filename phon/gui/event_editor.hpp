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
 * Created: 14/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Edit a label in a concordance or annotation.                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_EVENT_EDITOR_HPP
#define PHONOMETRICA_EVENT_EDITOR_HPP

#include <wx/dialog.h>
#include <wx/richtext/richtextctrl.h>
#include <phon/application/annotation.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

class EventEditor final : public wxDialog
{
public:

	EventEditor(wxWindow *parent, const Handle<Annotation> &annot, const AutoEvent &event, wxSize size);

	EventEditor(wxWindow *parent, const Handle<Annotation> &annot, const AutoEvent &event, intptr_t sel_start, intptr_t len, wxSize size);

	Signal<bool> done;

private:

	EventEditor(wxWindow *parent, wxSize size);

	void OnKeyPressed(wxKeyEvent &e);

	void EditEvent();

	wxRichTextCtrl *m_ctrl;

	Handle<Annotation> m_annot;

	AutoEvent m_event;
};

} // namespace phonometrica



#endif // PHONOMETRICA_EVENT_EDITOR_HPP
