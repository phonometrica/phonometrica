//
// Created by Julien Eychenne on 16/01/2021.
//

// Custom art provider for wxAuiNotebook

#ifndef PHONOMETRICA_TAB_ART_PROVIDER_HPP
#define PHONOMETRICA_TAB_ART_PROVIDER_HPP

#include <wx/aui/tabart.h>

class TabArtProvider final : public wxAuiGenericTabArt
{
public:

	TabArtProvider();

	wxAuiTabArt* Clone() override;
	void DrawBackground(wxDC &dc, wxWindow * WXUNUSED(wnd), const wxRect &rect) override;
	void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page, const wxRect& in_rect, int close_button_state, wxRect* out_tab_rect, wxRect* out_button_rect, int* x_extent) override;



private:

};

#endif //PHONOMETRICA_TAB_ART_PROVIDER_HPP
