/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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


EventEditor::EventEditor(wxWindow *parent, wxSize size) :
		wxDialog(parent, wxID_ANY, _("Edit event..."), wxDefaultPosition, size, wxCAPTION|wxCLOSE_BOX|wxSTAY_ON_TOP|wxRESIZE_BORDER)
{
	auto sizer = new VBoxSizer;
	m_ctrl = new wxRichTextCtrl(this, wxID_ANY, wxString(), wxDefaultPosition, wxDefaultSize, wxRE_MULTILINE);
	sizer->Add(m_ctrl, 1, wxEXPAND|wxALL, 0);
	auto font = m_ctrl->GetFont();
	font.MakeLarger();
	m_ctrl->SetFont(font);
	m_default_font = font;
	m_bold_font = font.MakeBold();
	SetSizer(sizer);
	m_default_colour = m_ctrl->GetForegroundColour();

	m_ctrl->Bind(wxEVT_KEY_DOWN, &EventEditor::OnKeyPressed, this);
	this->Bind(wxEVT_CLOSE_WINDOW, &EventEditor::OnClose, this);
}

void EventEditor::OnKeyPressed(wxKeyEvent &e)
{
	switch (e.GetKeyCode())
	{
		case WXK_RETURN:
		case WXK_NUMPAD_ENTER:
		{
			Hide();
			process();

		} break;
		case WXK_ESCAPE:
		{
			Hide();
		}
		break;
		default:
			e.Skip();
	}
}

void EventEditor::Prepare(const Handle<Annotation> &annot, const AutoEvent &event)
{
	m_annot = annot;
	m_event = event;
	m_ctrl->SetValue(event->text());
}

void EventEditor::Prepare(const Handle<Annotation> &annot, const AutoEvent &event, intptr_t sel_start, intptr_t len)
{
	m_annot = annot;
	m_event = event;
	auto txt = event->text();
	String left = String(txt.begin(), sel_start); // we need this one later
	wxString selection = String(txt.begin() + sel_start, len);
	wxString right = String(txt.begin() + sel_start + len);

	m_ctrl->Clear();
	m_ctrl->BeginTextColour(m_default_colour);
	m_ctrl->BeginFont(m_default_font);
	m_ctrl->WriteText(left);
	m_ctrl->BeginTextColour(*wxRED);
	m_ctrl->BeginFont(m_bold_font);
	m_ctrl->WriteText(selection);
	m_ctrl->EndFont();
	m_ctrl->EndTextColour();
	m_ctrl->WriteText(right);
	m_ctrl->EndFont();
	m_ctrl->EndTextColour();
	m_ctrl->SetFocus();
}

AutoCommand EventEditor::GetCommand() const
{
	String text = m_ctrl->GetValue();
	return std::make_unique<EditEventCommand>(m_annot, m_event, text);
}

void EventEditor::OnClose(wxCloseEvent &)
{
	Hide();
}

} // namespace phonometrica
