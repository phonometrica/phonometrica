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
 * Created: 27/05/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display and manage a layer in an annotation view.                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_LAYER_TRACK_HPP
#define PHONOMETRICA_LAYER_TRACK_HPP

#include <wx/stattext.h>
#include <phon/gui/plot/speech_widget.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

class LayerTrack final : public SpeechWidget
{
public:

	LayerTrack(wxWindow *parent, const Handle<Annotation> &annot, double duration, intptr_t layer_index);

	void DrawYAxis(PaintDC &dc, const wxRect &rect) override;

	void SetSelectedEvent(const AutoEvent &e);

	void ClearSelectedEvent();

	int GetIndex() const;

	bool IsSelected() const { return m_selected_event != nullptr; }

	Signal<int, const AutoEvent&> update_selected_event;

private:

	double GetSoundDuration() const override { return m_duration; }

	void UpdateCache() override;

	void OnPaint(wxPaintEvent &);

	void DrawAnchor(wxPaintDC &dc, double time, bool is_instant);

	void OnMotion(wxMouseEvent &e);

	void OnLeftClick(wxMouseEvent &e);

	AutoEvent XPosToEvent(int x) const;

	EventList FilterEvents(double t1, double t2) const { return m_graph.get_layer_events(m_layer->index, t1, t2); }

	// We need to store a reference to the annotation to avoid a subtle bug that leads to a crash. Due to the way
	// destructors work, the AutoLayer pointer owned by this widget will be released after the AutoAnnotation pointer
	// owned by the annotation view. If the annotation pointer is unique (because it's a temporary annotation which is
	// not registered in the project), the annotation graph will already have been destroyed when we try to finalize
	// the layer. As a result, the anchors that are referenced by the layer's events will be invalid. Therefore,
	// we keep an extra pointer to the annotation *before* the layer pointer to ensure that the anchors still exist.
	Handle<Annotation> unused;

	// Cached events in the current window.
	EventList m_cached_events;

	// Selected event on this layer, if any. There can be only one selected event across all the layers of an annotation.
	AutoEvent m_selected_event;

	AGraph &m_graph;

	AutoLayer m_layer;

	// Duration of the sound file.
	double m_duration;
};

} // namespace phonometrica

#endif // PHONOMETRICA_LAYER_TRACK_HPP
