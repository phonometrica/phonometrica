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
#include <phon/gui/script_ctrl.hpp>
#include <phon/application/settings.hpp>

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
	InitializeFont();
	InitializeCallTips();
    StyleClearAll();
	SetTabWidth(TAB_WIDTH);
	Bind(wxEVT_STC_CHARADDED, &ScriptControl::OnCharAdded, this);
	Bind(wxEVT_STC_CHANGE, &ScriptControl::OnChange, this);
	Bind(wxEVT_STC_CALLTIP_CLICK, &ScriptControl::OnCallTipClicked, this);
}

void ScriptControl::InitializeFont()
{
    wxFont font = Settings::get_mono_font();
    SetFont(font);
    StyleSetFont(wxSTC_STYLE_DEFAULT, font);
}


void ScriptControl::SetSyntaxHighlighting()
{
	error_indicator = 8;
	IndicatorSetStyle(error_indicator, wxSTC_INDIC_SQUIGGLE);
	IndicatorSetForeground(error_indicator, *wxRED);

    wxColour comment_color(96, 96, 96);

    SetLexer(wxSTC_LEX_PYTHON);

    // Keywords from llex.c


    SetKeyWords(0, "and as assert break class continue debug do downto else elsif end false field for foreach function "
				   "if in inherits local method nan not null option or pass print ref repeat return step super then this throw "
	                "to true until while");

//    SetKeyWords(1, "false nan null this true");

    StyleSetForeground(wxSTC_P_STRING, wxColour(0, 102, 0));
    StyleSetForeground(wxSTC_P_NUMBER, wxColour(0, 0, 102));
    StyleSetForeground(wxSTC_P_COMMENTLINE, comment_color);
    StyleSetForeground(wxSTC_P_WORD, wxColour(153, 0, 76));
    StyleSetForeground(wxSTC_P_WORD2, wxColour(51, 0, 102));

    StyleSetBold(wxSTC_P_WORD, true);
    StyleSetBold(wxSTC_P_WORD2, true);
	AddTypeNames();
}

void ScriptControl::SetLineNumbering()
{
    SetMarginWidth(MARGIN_LINE_NUMBERS, 50);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour (75, 75, 75));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour (220, 220, 220));
    SetMarginType(MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
}

void ScriptControl::AddTypeNames()
{
    wxString keywords = "Array Boolean Float Function Integer json List Module Number Object phon Regex Set String Table";
    SetKeyWords(1, keywords);
}

void ScriptControl::HideMargin()
{
	// But default, margin 1 is displayed with a width of 16 pixels. We make it 0 pixel.
	// See documentation here: https://www.scintilla.org/ScintillaDoc.html#Margins
	this->SetMarginWidth(1, 0);
}

void ScriptControl::OnChange(wxStyledTextEvent &)
{
	notify_modification();

	for (int i = 0; i < GetLineCount(); ++i)
	{
		int start = PositionFromLine(i);
		int end = GetLineEndPosition(i);
		int len = end - start;
		SetIndicatorCurrent(error_indicator);
		IndicatorClearRange(start, len);
	}
}

void ScriptControl::OnCharAdded(wxStyledTextEvent &event)
{
	static const wxString completion_list("Array assert Boolean break class continue debug do downto else elsif false field Float foreach function Function "
			"inherits Integer json List local method Module null Number Object option pass phon print Regex repeat return "
			"Set step String super Table then this throw true until while");
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
			WriteNewLine();
			for (int i = 0; i < indent; i++) {
				WriteText(tab);
			}
			WriteText("end");
			WriteNewLine();
		}
		else if (line.EndsWith(else_) || line.EndsWith(elsif))
		{
			WriteText(tab);
			pos++;
		}
		//SetInsertionPoint(pos);
		SetEmptySelection(pos);
	}
	else if (has_hints)
	{
		if (event.GetKey() == '(')
		{
			int current_pos =  GetCurrentPos();
			int end_pos = current_pos - 1; // ignore '('
			int start_pos = WordStartPosition(end_pos, true);
			auto txt = GetSubstring(start_pos, end_pos);
			SetEmptySelection(current_pos);
			auto it = calltips.find(txt);
			if (it != calltips.end())
			{
				// Point to the first tip
				calltip_position = current_pos;
				calltip_list = &it->second;
				current_calltip = calltip_list->begin();
				CallTipShow(current_pos, *current_calltip);
			}
		}
		else
		{
			// Find the word start
			int current_pos = GetCurrentPos();
			int start_pos = WordStartPosition(current_pos, true);
			// Display the autocompletion list
			int word_len = current_pos - start_pos;
			if (word_len > 1) {
				AutoCompShow(word_len, completion_list);
			}
		}
	}
}

std::pair<int, int> ScriptControl::GetSelectedLines() const
{
	int start = LineFromPosition(GetSelectionStart());
	int end = LineFromPosition(GetSelectionEnd());

	return {start, end};
}

void ScriptControl::ShowError(intptr_t line_no)
{
	line_no--; // to base 0
	int start = PositionFromLine(line_no);
	int end = GetLineEndPosition(line_no);
	int len = end - start;
	SetIndicatorCurrent(error_indicator);
	IndicatorFillRange(start, len);
}

void ScriptControl::WriteNewLine()
{
#ifdef __WXMSW__
	WriteText("\r\n");
#else
	WriteText("\n");
#endif
}

wxString ScriptControl::GetSubstring(int from, int to)
{
	SetSelectionStart(from);
	SetSelectionEnd(to);
	auto txt = GetSelectedText();
	ClearSelections();

	return txt;
}

void ScriptControl::InitializeCallTips()
{
	std::vector<std::pair<const char*, std::vector<wxString>>> function_declarations = {
			{ "split",  {
				"split(ref str as String, delim as String)\nReturns a List of strings which have been split at each occurrence of the substring delim."
			}},
			{ "append", {
				"append(ref str as String, suffix as String)\nInserts suffix as the end of str.\002",
				"append(ref list as List, item as Object)\nInserts item at the end of list.\001"
			}}
	};

	for (auto &def : function_declarations) {
		calltips[def.first] = std::move(def.second);
	}
}

void ScriptControl::ActivateHints(bool value)
{
	has_hints = value;
	if (!has_hints)
	{
		AutoCompCancel();
		CallTipCancel();
	}
}

void ScriptControl::OnCallTipClicked(wxStyledTextEvent &e)
{
	int pos = e.GetPosition();

	if (pos == 0)
	{
		return;
	}
	else if (pos == 1)
	{
		--current_calltip;
	}
	else // pos == 2
	{
		++current_calltip;
	}

	CallTipShow(calltip_position, *current_calltip);
}

} // namespace phonometrica