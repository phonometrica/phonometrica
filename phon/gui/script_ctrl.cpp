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
 * Created: 13/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/settings.h>
#include <phon/gui/macros.hpp>
#include <phon/gui/script_ctrl.hpp>

namespace phonometrica {

static const int TAB_WIDTH = 4;

enum
{
    MARGIN_LINE_NUMBERS,
    MARGIN_FOLD
};

ScriptControl::ScriptControl(wxWindow *parent) :
        wxStyledTextCtrl(parent, wxNewId(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_RICH2|wxTE_PROCESS_ENTER)
{
    InitFont();
    StyleClearAll();
	Bind(wxEVT_STC_CHARADDED, &ScriptControl::OnChange, this);
}

void ScriptControl::InitFont()
{
    wxFont font = MONOSPACE_FONT;
    SetFont(font);
    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
    SetTabWidth(TAB_WIDTH);
}


void ScriptControl::SetSyntaxHighlighting()
{
    wxColour comment_color(96, 96, 96);

    SetLexer(wxSTC_LEX_PYTHON);

    // Keywords from llex.c


    SetKeyWords(0, "and as assert break class continue debug do downto else elsif end explicit field for foreach function "
				   "if in inherits local method not option or pass print ref repeat return step super then throw "
	                "to until var while");

    SetKeyWords(1, "false nan null this true");
    StyleSetForeground(wxSTC_P_STRING, wxColour(0, 102, 0));
    StyleSetForeground(wxSTC_P_NUMBER, wxColour(0, 0, 102));
    StyleSetForeground(wxSTC_P_COMMENTLINE, comment_color);
    StyleSetForeground(wxSTC_P_WORD, wxColour(153, 0, 76));
    StyleSetForeground(wxSTC_P_WORD2, wxColour(51, 0, 102));

    StyleSetBold(wxSTC_P_WORD, true);
    StyleSetBold(wxSTC_P_WORD2, true);
}

void ScriptControl::SetLineNumbering()
{
    SetMarginWidth(MARGIN_LINE_NUMBERS, 50);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour (75, 75, 75));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour (220, 220, 220));
    SetMarginType(MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
}

void ScriptControl::AddTypeName(const wxString &kw)
{
    wxString keywords = "List String Regex";
    SetKeyWords(1, keywords.Append(kw));
}

void ScriptControl::HideMargin()
{
	// But default, margin 1 is displayed with a width of 16 pixels. We make it 0 pixel.
	// See documentation here: https://www.scintilla.org/ScintillaDoc.html#Margins
	this->SetMarginWidth(1, 0);
}

void ScriptControl::OnChange(wxStyledTextEvent &event)
{
	constexpr int NEW_LINE = 10;
	static wxString then(" then"), do_(" do"), func("function "), func2("local function "), else_("else"), elsif("elsif"), tab("\t");

	// Add the same level of indentation as the previous line.
	if (event.GetKey() == NEW_LINE)
	{
		int pos = GetCurrentPos();
		int no = GetCurrentLine() - 1;
		auto line = this->GetLine(no);
		int indent = 0;

		for (auto c : line)
		{
			if (c == '\t')
			{
				indent++;
				pos++;
			}
			else
			{
				break;
			}
		}
		for (int i = 0; i < indent; i++) {
			WriteText(tab);
		}
		line.Trim();
		line.Trim(false);
		if (line.EndsWith(then) || line.EndsWith(do_) || line.StartsWith(func) || line.StartsWith(func2) || line.EndsWith(else_) || line.EndsWith(elsif))
		{
			WriteText(tab);
			pos++;
		}
		SetInsertionPoint(pos);
	}

//	wxStyledTextCtrl* stc = (wxStyledTextCtrl*)event.GetEventObject();
//
//	// Find the word start
//	int currentPos = stc->GetCurrentPos();
//	int wordStartPos = stc->WordStartPosition( currentPos, true );
//
//	// Display the autocompletion list
//	int lenEntered = currentPos - wordStartPos;
//	if (lenEntered > 0)
//	{
//		stc->AutoCompShow(lenEntered, "and break do else elsif end false for function if in local nil not or repeat return then true until while");
//	}
}

std::pair<int, int> ScriptControl::GetSelectedLines() const
{
	int start = LineFromPosition(GetSelectionStart());
	int end = LineFromPosition(GetSelectionEnd());

	return {start, end};
}

} // namespace phonometrica