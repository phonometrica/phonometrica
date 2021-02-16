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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/sizer.hpp>
#include <phon/gui/event_editor.hpp>
#include <phon/application/project.hpp>

namespace phonometrica {


EventEditor::EventEditor(wxWindow *parent, wxPoint position, wxSize size) :
		wxWindow(parent, wxID_ANY, position, size)
{
	auto sizer = new VBoxSizer;
	m_ctrl = new wxRichTextCtrl(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize, wxRE_MULTILINE);
	sizer->Add(m_ctrl, 1, wxEXPAND|wxALL, 5);
	SetBackgroundColour(wxColor(126, 66, 245));
	auto font = m_ctrl->GetFont();
	font.MakeLarger();
	m_ctrl->SetFont(font);
	SetSizer(sizer);

	m_ctrl->Bind(wxEVT_KEY_DOWN, &EventEditor::OnKeyPressed, this);
}


EventEditor::EventEditor(wxWindow *parent, const Handle<Annotation> &annot, const AutoEvent &event, wxPoint position,
                         wxSize size) : EventEditor(parent, position, size)
{
	m_annot = annot;
	m_event = event;
	m_ctrl->SetValue(event->text());
}


EventEditor::EventEditor(wxWindow *parent, const Handle<Annotation> &annot, const AutoEvent &event, intptr_t sel_start,
                         intptr_t len, wxPoint position, wxSize size) :
		EventEditor(parent, position, size)
{
	m_annot = annot;
	m_event = event;
	auto txt = event->text();
	String left = String(txt.begin(), sel_start); // we need this one later
	wxString selection = String(txt.begin() + sel_start, len);
	wxString right = String(txt.begin() + sel_start + len);

	m_ctrl->WriteText(left);
	m_ctrl->BeginTextColour(*wxRED);
	m_ctrl->BeginBold();
	m_ctrl->WriteText(selection);
	m_ctrl->EndBold();
	m_ctrl->EndTextColour();
	m_ctrl->WriteText(right);
	m_ctrl->SetFocus();
}

void EventEditor::OnKeyPressed(wxKeyEvent &e)
{
	// Note: Escape is handled globally
	switch (e.GetKeyCode())
	{
		case WXK_RETURN:
		case WXK_NUMPAD_ENTER:
		{
			EditEvent();

		} break;
		default:
			e.Skip();
	}
}

void EventEditor::EditEvent()
{
	String text = m_ctrl->GetValue();
	m_annot->set_event_text(m_event, text);
	Project::updated();
	done();
}

} // namespace phonometrica
