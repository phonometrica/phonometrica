// This file is a modified version of wxWidget's tab art provider, which came with the following copyright notice:

///////////////////////////////////////////////////////////////////////////////
// Name:        src/aui/tabart.cpp
// Purpose:     wxaui: wx advanced user interface - notebook-art
// Author:      Benjamin I. Williams
// Modified by: Jens Lody (moved from auibook.cpp in extra file)
// Created:     2012-03-21
// Copyright:   (C) Copyright 2006, Kirix Corporation, All Rights Reserved
// Licence:     wxWindows Library Licence, Version 3.1
///////////////////////////////////////////////////////////////////////////////

#ifndef WX_PRECOMP
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/settings.h"
#include "wx/bitmap.h"
#include "wx/menu.h"
#endif

#include "wx/renderer.h"
#include "wx/aui/auibook.h"
#include "wx/aui/framemanager.h"
#include "wx/aui/dockart.h"

#ifdef __WXMAC__
#include "wx/osx/private.h"
#endif

#include <wx/aui/auibook.h>
#include <wx/settings.h>
#include <phon/gui/tab_art_provider.hpp>
#include <phon/gui/macros.hpp>
#include <phon/include/icons.hpp>

// TODO: better match tab color to system color on mac
#ifdef __WXMAC__
#define BACKGROUND_COLOR wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)
//wxColor(237, 237, 237)
#endif

static void IndentPressedBitmap(const wxSize& offset, wxRect* rect, int button_state)
{
	if (button_state == wxAUI_BUTTON_STATE_PRESSED)
	{
		rect->x += offset.x;
		rect->y += offset.y;
	}
}

wxBitmap wxAuiBitmapFromBits(const unsigned char bits[], int w, int h,
							 const wxColour& color);

// This function is defined in dockart.cpp.
void wxAuiScaleBitmap(wxBitmap& bmp, double scale);
float wxAuiGetColourContrast(const wxColour& c1, const wxColour& c2);

wxString wxAuiChopText(wxDC& dc, const wxString& text, int max_size);

static bool wxAuiHasSufficientContrast(const wxColour& c1, const wxColour& c2)
{
	return wxAuiGetColourContrast(c1, c2) >= 4.5f;
}

// Pick a color that provides better contrast against the background
static wxColour wxAuiGetBetterContrastColour(const wxColour& back_color,
											 const wxColour& c1, const wxColour& c2)
{
	return wxAuiGetColourContrast(back_color, c1)
		   > wxAuiGetColourContrast(back_color, c2) ? c1 : c2;
}


TabArtProvider::TabArtProvider() : wxAuiGenericTabArt()
{
	m_disabledCloseBmp = wxBitmap(wxBITMAP_PNG(close_tab));
	m_activeCloseBmp = wxBitmap(wxBITMAP_PNG(close_tab2));
}

void TabArtProvider::DrawBackground(wxDC &dc, wxWindow *, const wxRect &rect)
{
	// draw background using arbitrary hard-coded, but at least adapted to dark
	// mode, gradient
	int topLightness, bottomLightness;
	if (wxSystemSettings::GetAppearance().IsUsingDarkBackground())
	{
		topLightness = 110;
		bottomLightness = 90;
	}
	else
	{
		topLightness = 90;
		bottomLightness = 170;
	}

	wxColor top_color    = m_baseColour.ChangeLightness(topLightness);
	wxColor bottom_color = m_baseColour.ChangeLightness(bottomLightness);
	wxRect r;

	if (m_flags &wxAUI_NB_BOTTOM)
		r = wxRect(rect.x, rect.y, rect.width+2, rect.height);
		// TODO: else if (m_flags &wxAUI_NB_LEFT) {}
		// TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
	else //for wxAUI_NB_TOP
		r = wxRect(rect.x, rect.y, rect.width+2, rect.height-3);

#ifdef __WXMAC__
	if (!wxSystemSettings::GetAppearance().IsUsingDarkBackground())
	{
		bottom_color = wxColor(237, 237, 237);
		top_color = bottom_color;
	}
#endif
	dc.GradientFillLinear(r, top_color, bottom_color, wxSOUTH);


	// draw base lines

	dc.SetPen(m_borderPen);
	int y = rect.GetHeight();
	int w = rect.GetWidth();

	if (m_flags &wxAUI_NB_BOTTOM)
	{
		dc.SetBrush(wxBrush(bottom_color));
		dc.DrawRectangle(-1, 0, w+2, 4);
	}
		// TODO: else if (m_flags &wxAUI_NB_LEFT) {}
		// TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
	else //for wxAUI_NB_TOP
	{
		dc.SetBrush(m_baseColourBrush);
		dc.DrawRectangle(-1, y-4, w+2, 4);
	}
}

void TabArtProvider::DrawTab(wxDC &dc, wxWindow *wnd, const wxAuiNotebookPage &page, const wxRect &in_rect,
							 int close_button_state, wxRect *out_tab_rect, wxRect *out_button_rect, int *x_extent)
{
	wxCoord normal_textx, normal_texty;
	wxCoord selected_textx, selected_texty;
	wxCoord texty;

	// if the caption is empty, measure some temporary text
	wxString caption = page.caption;
	if (caption.empty())
		caption = wxT("Xj");

	dc.SetFont(m_selectedFont);
	dc.GetTextExtent(caption, &selected_textx, &selected_texty);

	dc.SetFont(m_normalFont);
	dc.GetTextExtent(caption, &normal_textx, &normal_texty);

	// figure out the size of the tab
	wxSize tab_size = GetTabSize(dc,
								 wnd,
								 page.caption,
								 page.bitmap,
								 page.active,
								 close_button_state,
								 x_extent);

	wxCoord tab_height = m_tabCtrlHeight - 3;
	wxCoord tab_width = tab_size.x;
	wxCoord tab_x = in_rect.x;
	wxCoord tab_y = in_rect.y + in_rect.height - tab_height;


	caption = page.caption;


	// select pen, brush and font for the tab to be drawn

	if (page.active)
	{
		dc.SetFont(m_selectedFont);
		texty = selected_texty;
	}
	else
	{
		dc.SetFont(m_normalFont);
		texty = normal_texty;
	}


	// create points that will make the tab outline

	int clip_width = tab_width;
	if (tab_x + clip_width > in_rect.x + in_rect.width)
		clip_width = (in_rect.x + in_rect.width) - tab_x;

/*
    wxPoint clip_points[6];
    clip_points[0] = wxPoint(tab_x,              tab_y+tab_height-3);
    clip_points[1] = wxPoint(tab_x,              tab_y+2);
    clip_points[2] = wxPoint(tab_x+2,            tab_y);
    clip_points[3] = wxPoint(tab_x+clip_width-1, tab_y);
    clip_points[4] = wxPoint(tab_x+clip_width+1, tab_y+2);
    clip_points[5] = wxPoint(tab_x+clip_width+1, tab_y+tab_height-3);
    // FIXME: these ports don't provide wxRegion ctor from array of points
#if !defined(__WXDFB__)
    // set the clipping region for the tab --
    wxRegion clipping_region(WXSIZEOF(clip_points), clip_points);
    dc.SetClippingRegion(clipping_region);
#endif // !wxDFB && !wxCocoa
*/
	// since the above code above doesn't play well with WXDFB or WXCOCOA,
	// we'll just use a rectangle for the clipping region for now --
	dc.SetClippingRegion(tab_x, tab_y, clip_width+1, tab_height-3);


	wxPoint border_points[6];
	if (m_flags &wxAUI_NB_BOTTOM)
	{
		border_points[0] = wxPoint(tab_x,             tab_y);
		border_points[1] = wxPoint(tab_x,             tab_y+tab_height-6);
		border_points[2] = wxPoint(tab_x+2,           tab_y+tab_height-4);
		border_points[3] = wxPoint(tab_x+tab_width-2, tab_y+tab_height-4);
		border_points[4] = wxPoint(tab_x+tab_width,   tab_y+tab_height-6);
		border_points[5] = wxPoint(tab_x+tab_width,   tab_y);
	}
	else //if (m_flags & wxAUI_NB_TOP) {}
	{
		border_points[0] = wxPoint(tab_x,             tab_y+tab_height-4);
		border_points[1] = wxPoint(tab_x,             tab_y+2);
		border_points[2] = wxPoint(tab_x+2,           tab_y);
		border_points[3] = wxPoint(tab_x+tab_width-2, tab_y);
		border_points[4] = wxPoint(tab_x+tab_width,   tab_y+2);
		border_points[5] = wxPoint(tab_x+tab_width,   tab_y+tab_height-4);
	}
	// TODO: else if (m_flags &wxAUI_NB_LEFT) {}
	// TODO: else if (m_flags &wxAUI_NB_RIGHT) {}

	int drawn_tab_yoff = border_points[1].y;
	int drawn_tab_height = border_points[0].y - border_points[1].y;

	bool isdark = wxSystemSettings::GetAppearance().IsUsingDarkBackground();

	wxColor back_color = m_baseColour;
	if (page.active)
	{
		// draw active tab

		// draw base background color
		wxRect r(tab_x, tab_y, tab_width, tab_height);
		dc.SetPen(wxPen(m_activeColour));
		dc.SetBrush(wxBrush(m_activeColour));
		dc.DrawRectangle(r.x+1, r.y+1, r.width-1, r.height-4);

		// this white helps fill out the gradient at the top of the tab
		wxColor gradient = *wxWHITE;
		if (isdark)
		{
			//dark mode, we go darker
			gradient = m_activeColour.ChangeLightness(70);
		}
		back_color = gradient;

		dc.SetPen(wxPen(gradient));
		dc.SetBrush(wxBrush(gradient));
		dc.DrawRectangle(r.x+2, r.y+1, r.width-3, r.height-4);

		// these two points help the rounded corners appear more antialiased
		dc.SetPen(wxPen(m_activeColour));
		dc.DrawPoint(r.x+2, r.y+1);
		dc.DrawPoint(r.x+r.width-2, r.y+1);

		// set rectangle down a bit for gradient drawing
		r.SetHeight(r.GetHeight()/2);
		r.x += 2;
		r.width -= 3;
		r.y += r.height;
		r.y -= 2;

		// draw gradient background
		wxColor top_color = gradient;
		wxColor bottom_color = m_activeColour;
		// Give uniform color on Mac
#ifdef __WXMAC__
		if (!wxSystemSettings::GetAppearance().IsUsingDarkBackground())
		{
			bottom_color = wxColor(237, 237, 237);
			top_color = bottom_color;
		}
#endif
		dc.GradientFillLinear(r, bottom_color, top_color, wxSOUTH);
	}
	else
	{
		// draw inactive tab

		wxRect r(tab_x, tab_y+1, tab_width, tab_height-3);

		// start the gradient up a bit and leave the inside border inset
		// by a pixel for a 3D look.  Only the top half of the inactive
		// tab will have a slight gradient
		r.x += 3;
		r.y++;
		r.width -= 4;
		r.height /= 2;
		r.height--;

		// -- draw top gradient fill for glossy look
		wxColor top_color = m_baseColour;
		wxColor bottom_color = top_color.ChangeLightness(160);
		if (isdark)
		{
			//dark mode, we go darker
			top_color = m_activeColour.ChangeLightness(70);
			bottom_color = m_baseColour;
		}

#ifdef __WXMAC__
		if (!wxSystemSettings::GetAppearance().IsUsingDarkBackground())
		{
			bottom_color = wxColor(237, 237, 237);
			top_color = bottom_color;
		}
#endif
		dc.GradientFillLinear(r, bottom_color, top_color, wxNORTH);

		r.y += r.height;
		r.y--;

		// -- draw bottom fill for glossy look
		top_color = m_baseColour;
		bottom_color = m_baseColour;
#ifdef __WXMAC__
		if (!isdark)
		{
			bottom_color = wxColor(237, 237, 237);
			top_color = bottom_color;
		}
#endif
		dc.GradientFillLinear(r, top_color, bottom_color, wxSOUTH);
	}

	// draw tab outline
	dc.SetPen(m_borderPen);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawPolygon(WXSIZEOF(border_points), border_points);

	// there are two horizontal grey lines at the bottom of the tab control,
	// this gets rid of the top one of those lines in the tab control
	if (page.active)
	{
		if (m_flags &wxAUI_NB_BOTTOM)
			dc.SetPen(wxPen(m_baseColour.ChangeLightness(170)));
			// TODO: else if (m_flags &wxAUI_NB_LEFT) {}
			// TODO: else if (m_flags &wxAUI_NB_RIGHT) {}
		else //for wxAUI_NB_TOP
			dc.SetPen(m_baseColourPen);
		dc.DrawLine(border_points[0].x+1,
					border_points[0].y,
					border_points[5].x,
					border_points[5].y);
	}


	int text_offset;
	int bitmap_offset = 0;
	if (page.bitmap.IsOk())
	{
		bitmap_offset = tab_x + wnd->FromDIP(8);

		// draw bitmap
		dc.DrawBitmap(page.bitmap,
					  bitmap_offset,
					  drawn_tab_yoff + (drawn_tab_height/2) - (page.bitmap.GetScaledHeight()/2),
					  true);

		text_offset = bitmap_offset + page.bitmap.GetScaledWidth();
		text_offset += wnd->FromDIP(3); // bitmap padding
	}
	else
	{
		text_offset = tab_x + wnd->FromDIP(8);
	}

	// draw close button if necessary
	int close_button_width = 0;
	if (close_button_state != wxAUI_BUTTON_STATE_HIDDEN)
	{
		wxBitmap bmp = m_disabledCloseBmp;

		if (close_button_state == wxAUI_BUTTON_STATE_HOVER ||
			close_button_state == wxAUI_BUTTON_STATE_PRESSED)
		{
			bmp = m_activeCloseBmp;
		}

		wxAuiScaleBitmap(bmp, wnd->GetDPIScaleFactor());

		int offsetY = tab_y-1;
		if (m_flags & wxAUI_NB_BOTTOM)
			offsetY = 1;

		wxRect rect(tab_x + tab_width - bmp.GetScaledWidth() - wnd->FromDIP(1),
					offsetY + (tab_height/2) - (bmp.GetScaledHeight()/2),
					bmp.GetScaledWidth(),
					tab_height);

		IndentPressedBitmap(wnd->FromDIP(wxSize(1, 1)), &rect, close_button_state);
		dc.DrawBitmap(bmp, rect.x, rect.y, true);

		*out_button_rect = rect;
		close_button_width = bmp.GetScaledWidth();
	}

	wxString draw_text = wxAuiChopText(dc,
									   caption,
									   tab_width - (text_offset-tab_x) - close_button_width);

	// draw tab text
	wxColor sys_color = wxSystemSettings::GetColour(
			page.active ? wxSYS_COLOUR_CAPTIONTEXT : wxSYS_COLOUR_INACTIVECAPTIONTEXT);
#ifdef __WXMAC__
	auto better_color = wxSystemSettings::GetAppearance().IsUsingDarkBackground() ? *wxWHITE : wxColor(67,67,67);
#else
	auto better_color = *wxWHITE;
#endif
	wxColor font_color = wxAuiHasSufficientContrast(back_color, sys_color) ? sys_color
																		   : wxAuiGetBetterContrastColour(back_color, better_color, *wxBLACK);
	dc.SetTextForeground(font_color);
	dc.DrawText(draw_text,
				text_offset,
				drawn_tab_yoff + (drawn_tab_height)/2 - (texty/2) - 1);

	// draw focus rectangle
	if (page.active && (wnd->FindFocus() == wnd))
	{
		wxRect focusRectText(text_offset, (drawn_tab_yoff + (drawn_tab_height)/2 - (texty/2) - 1),
							 selected_textx, selected_texty);

		wxRect focusRect;
		wxRect focusRectBitmap;

		if (page.bitmap.IsOk())
			focusRectBitmap = wxRect(bitmap_offset, drawn_tab_yoff + (drawn_tab_height/2) - (page.bitmap.GetScaledHeight()/2),
									 page.bitmap.GetScaledWidth(), page.bitmap.GetScaledHeight());

		if (page.bitmap.IsOk() && draw_text.IsEmpty())
			focusRect = focusRectBitmap;
		else if (!page.bitmap.IsOk() && !draw_text.IsEmpty())
			focusRect = focusRectText;
		else if (page.bitmap.IsOk() && !draw_text.IsEmpty())
			focusRect = focusRectText.Union(focusRectBitmap);

		focusRect.Inflate(2, 2);

		wxRendererNative::Get().DrawFocusRect(wnd, dc, focusRect, 0);
	}

	*out_tab_rect = wxRect(tab_x, tab_y, tab_width, tab_height);

	dc.DestroyClippingRegion();
}

wxAuiTabArt *TabArtProvider::Clone()
{
	return new TabArtProvider();
}

