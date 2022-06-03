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
 * Created: 14/01/2021                                                                                                 *
 *                                                                                                                     *
 * purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VIEW_HPP
#define PHONOMETRICA_VIEW_HPP

#include <wx/panel.h>
#include <wx/cmdproc.h>
#include <phon/utils/signal.hpp>
#include <phon/gui/cmd/command_processor.hpp>
#include <phon/application/vfs.hpp>

namespace phonometrica {

class View : public wxPanel
{
public:

	explicit View(wxWindow *parent);

	// This method is called before the view is destroyed. It returns true
	// if the view can be closed, false if it must be kept open.
	virtual bool Finalize(bool autosave);

	// These methods can be overriden to respond to accelerators such as ctrl+s (save) or ctrl+r (run)
	virtual void Save() { }

	virtual void Execute() { }

	virtual void Find();

	virtual void Replace();

	virtual void Escape() { }

	virtual void AdjustFontSize() { }

	virtual bool IsStartView() const { return false; }

	virtual String GetPath() const { return String(); }

	virtual bool IsModified() const = 0;

	virtual void DiscardChanges() = 0;

	virtual wxString GetLabel() const = 0;

	void UpdateTitle();

	virtual void Undo();

	virtual void Redo();

	void Submit(AutoCommand cmd);

	static Signal<> modified;

	static Signal<> request_console;

	static Signal<const String&> send_code;

	static Signal<const Handle<Document>&> file_created;

protected:

	void SetTitle(const wxString &title);

	bool AskImportFile(const String &path);

	// Update content when an edit has been done (or undone)
	virtual void UpdateView();

	CommandProcessor command_processor;
};

} // namespace phonometrica

#endif // PHONOMETRICA_VIEW_HPP
