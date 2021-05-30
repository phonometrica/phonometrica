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
 * Created: 27/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: View for annotation files.                                                                                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ANNOTATION_VIEW_HPP
#define PHONOMETRICA_ANNOTATION_VIEW_HPP

#include <phon/gui/views/speech_view.hpp>
#include <phon/gui/plot/layer_track.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

class AnnotationView final : public SpeechView
{
public:

	AnnotationView(wxWindow *parent, const Handle<Annotation> &annot);

	wxString GetLabel() const override;

	String GetPath() const override;

	Handle<Annotation> GetAnnotation() const;

	void OpenEvent(intptr_t nlayer, const AutoEvent &event);

private:

	void AddAnnotationMenu(ToolBar *toolbar) override;

	void AddAnnotationLayers(wxSizer *sizer) override;

	LayerTrack * AddAnnotationLayer(intptr_t i);

	void OnLayerMenu(wxCommandEvent &e);

	void OnAnchorSharing(wxCommandEvent &e);

	void OnAddAnchor(wxCommandEvent &e);

	void OnRemoveAnchor(wxCommandEvent &e);

	void OnSelectEvent(int nlayer, const AutoEvent &event);

	void UpdateLayersWindow(TimeWindow win) override;

	// Annotation managed by this view.
	Handle<Annotation> m_annot;

	// Layer tracks, in the order they are stored and displayed
    std::vector<LayerTrack*> m_layers;

    // Separators for layers
    std::vector<HLine*> m_layer_lines;

    wxButton *m_layer_tool;

    ToggleButton *m_sharing_tool, *m_add_tool, *m_remove_tool;
};

} // namespace phonometrica



#endif // PHONOMETRICA_ANNOTATION_VIEW_HPP
