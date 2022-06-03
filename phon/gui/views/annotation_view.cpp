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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/msgdlg.h>
#include <phon/gui/views/annotation_view.hpp>
#include <phon/gui/new_layer_dialog.hpp>
#include <phon/gui/cmd/add_layer_command.hpp>
#include <phon/gui/cmd/remove_layer_command.hpp>
#include <phon/include/icons.hpp>

namespace phonometrica {

AnnotationView::AnnotationView(wxWindow *parent, const Handle<Annotation> &annot) :
	SpeechView(parent, annot->sound()), m_annot(annot)
{

}

wxString AnnotationView::GetLabel() const
{
	return m_annot->label();
}

String AnnotationView::GetPath() const
{
	return m_annot->path();
}

void AnnotationView::AddAnnotationMenu(ToolBar *toolbar)
{
#define ICN(x) wxBITMAP_PNG_FROM_DATA(x)
	m_layer_tool = m_toolbar->AddMenuButton(ICN(layers), _("Manage layers"));
	m_sharing_tool = m_toolbar->AddToggleButton(ICN(link), _("Share/unshare anchors"));
	m_add_tool = m_toolbar->AddToggleButton(ICN(anchor), _("Add anchor"));
	m_remove_tool = m_toolbar->AddToggleButton(ICN(delete), _("Remove anchor"));
	toolbar->AddSeparator();
#undef ICN

	m_sharing_tool->Check(false);
	m_add_tool->Check(false);
	m_remove_tool->Check(false);

	m_layer_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AnnotationView::OnLayerMenu, this);
	m_sharing_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AnnotationView::OnAnchorSharing, this);
	m_add_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AnnotationView::OnAddAnchor, this);
	m_remove_tool->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AnnotationView::OnRemoveAnchor, this);
}

void AnnotationView::AddAnnotationLayers(wxSizer *sizer)
{
	intptr_t count = m_annot->size();

	for (intptr_t i = 1; i <= count; i++)
    {
		auto layer = CreateLayerTrack(i);
		InsertAnnotationLayer(sizer, layer, sizer->GetItemCount());
    }
}

void AnnotationView::InsertAnnotationLayer(wxSizer *sizer, LayerTrack *layer, size_t pos)
{
	sizer->Insert(pos, layer, 0, wxEXPAND|wxRIGHT, 10);
	auto i = layer->GetIndex() - 1;
	m_layers.insert(m_layers.begin() + i, layer);
	m_speech_widgets.insert(m_speech_widgets.begin() + i, layer);
	auto hline = new HLine(this);
	sizer->Insert(pos+1, hline);
	m_layer_lines.insert(m_layer_lines.begin() + i, hline);

	layer->update_status.connect(&MessageCtrl::SetLayerInfo, m_msg_ctrl);
	layer->update_selected_event.connect(&AnnotationView::OnSelectEvent, this);
	//
//	for (auto plot : plots)
//	{
//		connect(layer, &LayerWidget::current_time, plot, &Waveform::setCurrentTime);
//		connect(layer, &LayerWidget::event_selected, plot, &Waveform::setSelection);
//		connect(layer, &LayerWidget::window_moved, plot, &Waveform::setWindow);
//	}
//
//	connect(layer, &LayerWidget::got_focus, this, &AnnotationView::focusLayer);
//	connect(layer, &LayerWidget::focus_event, this, &AnnotationView::focusEvent);
//	connect(layer, &LayerWidget::modified, this, &AnnotationView::modified);
//	connect(layer, &LayerWidget::anchor_moving, this, &AnnotationView::setMovingAnchor);
//	connect(layer, &LayerWidget::anchor_has_moved, this, &AnnotationView::resetAnchorMovement);
//	// When a layer triggers a window shift, we need to update the scrollbar and the plots
//	connect(layer, &LayerWidget::window_moved, wavebar, &WaveBar::setTimeSelection);
//	connect(waveform, &Waveform::windowHasChanged, layer, &LayerWidget::setWindow);
//	connect(link_button, &QToolButton::clicked, layer, &LayerWidget::setAnchorSharing);
//	connect(layer, &LayerWidget::anchor_added, this, &AnnotationView::notifyAnchorAdded);
//	connect(layer, &LayerWidget::anchor_removed, this, &AnnotationView::notifyAnchorRemoved);
//	connect(layer, &LayerWidget::anchor_moved, this, &AnnotationView::notifyAnchorMoved);
//	connect(layer, &LayerWidget::anchor_selected, this, &AnnotationView::onAnchorSelected);
//	connect(layer, &LayerWidget::editing_shared_anchor, this, &AnnotationView::onEditAnchor);
//	connect(layer, &LayerWidget::editing_shared_anchor, this, &AnnotationView::setMovingAnchor);
//	connect(layer, &LayerWidget::temporary_anchor, this, &AnnotationView::setTemporaryAnchor);
//	connect(layer, &LayerWidget::inform_selection, this, &AnnotationView::informSelectedEvent);
}

LayerTrack * AnnotationView::CreateLayerTrack(intptr_t i)
{
	auto duration = m_annot->sound()->duration();
	auto layer = new LayerTrack(this, m_annot, duration, i);
	layer->SetTimeWindow(GetTimeWindow());

	return layer;
}

void AnnotationView::OnAnchorSharing(wxCommandEvent &e)
{
	m_sharing_tool->Toggle();

	if (m_sharing_tool->IsChecked())
	{
		m_sharing_tool->SetBitmap(wxBITMAP_PNG_FROM_DATA(broken_link));
	}
	else
	{
		m_sharing_tool->SetBitmap(wxBITMAP_PNG_FROM_DATA(link));
	}
}

void AnnotationView::OnAddAnchor(wxCommandEvent &e)
{
	m_add_tool->Toggle();
	if (m_add_tool->IsChecked() && m_remove_tool->IsChecked()) {
		m_remove_tool->Toggle();
	}
}

void AnnotationView::OnRemoveAnchor(wxCommandEvent &e)
{
	m_remove_tool->Toggle();
	if (m_remove_tool->IsChecked() && m_add_tool->IsChecked()) {
		m_add_tool->Toggle();
	}
}

void AnnotationView::OnLayerMenu(wxCommandEvent &e)
{
	auto menu = new wxMenu;

	auto add_layer_action = menu->Append(wxID_ANY, _("Add new layer..."));
	auto remove_layer_action = menu->Append(wxID_ANY, _("Remove selected layer"));
	menu->AppendSeparator();
	auto dup_layer_action = menu->Append(wxID_ANY, _("Duplicate selected layer"));
	auto rename_layer_action = menu->Append(wxID_ANY, _("Rename selected layer"));
	auto clear_layer_action = menu->Append(wxID_ANY, _("Clear selected layer"));
	menu->AppendSeparator();
	auto show_layer_action = menu->Append(wxID_ANY, _("Select visible layers"));

	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &AnnotationView::OnAddLayer, this, add_layer_action->GetId());
	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &AnnotationView::OnRemoveLayer, this, remove_layer_action->GetId());
//	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SpeechView::OnShowWaveforms, this, show_tool->GetId());
//	menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &SpeechView::OnWaveformSettings, this, settings_tool->GetId());


	m_toolbar->ShowMenu(m_layer_tool, menu);
}

void AnnotationView::OnSelectEvent(int nlayer, const AutoEvent &event)
{
	OnUpdateSelection(TimeSelection{event->start_time(), event->end_time()});

	for (auto layer : m_layers)
	{
		if (layer->GetIndex() != nlayer) {
			layer->ClearSelectedEvent();
		}
	}
	Refresh();
}

void AnnotationView::UpdateLayersWindow(TimeWindow win)
{
	for (auto layer : m_layers) {
		layer->SetTimeWindow(win);
	}
}

Handle<Annotation> AnnotationView::GetAnnotation() const
{
	return m_annot;
}

void AnnotationView::OpenEvent(intptr_t nlayer, const AutoEvent &event)
{
	if (!event)
	{
		wxString msg;
		if (event->is_instant()) {
			msg = wxString::Format(_("Cannot open event at %f on layer %d"), event->start_time(), (int)nlayer);
		}
		else {
			msg = wxString::Format(_("Cannot open event from %f to %f on layer %d"), event->start_time(), event->end_time(), (int)nlayer);
		}
		wxMessageBox(msg, _("Invalid event"), wxICON_ERROR);
		return;
	}

	for (auto layer : m_layers)
	{
		if (layer->GetIndex() == nlayer)
		{
			layer->SetSelectedEvent(event);
		}
		else
		{
			layer->ClearSelectedEvent();
		}
	}

	double start = event->start_time();
	double end = event->end_time();

	if (event->is_instant())
	{
		// View two seconds
		start -= 1.0;
		end += 1.0;
	}

	SetTimeWindow(start, end);
}

void AnnotationView::AddLayer(intptr_t index, const String &label, bool has_instants)
{
	m_annot->create_layer(index, label, has_instants);
	size_t pos = GetLayerTrackPosition(index);
	auto layer = CreateLayerTrack(index);
	InsertAnnotationLayer(m_inner_sizer, layer, pos);
	m_y_axis->AddWindow(layer);
	Layout();
	modified();
}

void AnnotationView::RemoveLayer(intptr_t index)
{
	m_annot->remove_layer(index--);
	auto layer = m_layers[index];
	auto hline = m_layer_lines[index];
	m_layers.erase(m_layers.begin() + index);
	m_layer_lines.erase(m_layer_lines.begin() + index);
	m_inner_sizer->Detach(layer);
	m_inner_sizer->Detach(hline);
	m_y_axis->RemoveWindow(layer);
	delete layer;
	delete hline;
	Layout();
	modified();
}

size_t AnnotationView::GetLayerTrackPosition(intptr_t i) const
{
	assert(i > 0);
	// For each plot and layer before this layer, also take into account hlines.
	return size_t((m_plots.size() + i - 1) * 2) + 1;
}

void AnnotationView::OnAddLayer(wxCommandEvent &)
{
	NewLayerDialog dlg(this, m_annot->size());

	if (dlg.ShowModal() == wxID_OK)
	{
		String label = dlg.GetLayerLabel();
		int index = dlg.GetIndex();
		bool has_instants = dlg.HasInstants();
		auto cmd = std::make_unique<AddLayerCommand>(this, index, label, has_instants);

		if (!command_processor.submit(std::move(cmd))) {
			wxMessageBox(_("Could not create layer"), _("Error"), wxICON_ERROR);
		}
	}
}

void AnnotationView::OnRemoveLayer(wxCommandEvent &)
{
	auto index = GetSelectedLayer();

	if (index > 0)
	{
		auto cmd = std::make_unique<RemoveLayerCommand>(this, index);

		if (!command_processor.submit(std::move(cmd))) {
			wxMessageBox(_("Could not remove layer"), _("Error"), wxICON_ERROR);
		}
		modified();
	}
	else
	{
		wxMessageBox(_("No selected layer!"), _("Error"), wxICON_ERROR);
	}
}

String AnnotationView::GetLayerLabel(intptr_t index) const
{
	return m_annot->get_layer_label(index);
}

bool AnnotationView::LayerHasInstants(intptr_t index) const
{
	return m_annot->layer_has_instants(index);
}

intptr_t AnnotationView::GetSelectedLayer() const
{
	for (size_t i = 0; i < m_layers.size(); i++)
	{
		if (m_layers[i]->IsSelected()) {
			return intptr_t(i + 1);
		}
	}

	return -1;
}

} // namespace phonometrica
