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
 * purpose: A bunch of macros related to the user interface.                                                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_MACROS_HPP
#define PHONOMETRICA_MACROS_HPP

#include <wx/settings.h>

// Monospace font
#if __WXMAC__
#define MONOSPACE_FONT wxFont(wxFontInfo(13).FaceName("Monaco"))
#elif defined(__WXMSW__)
#define MONOSPACE_FONT wxFont(wxFontInfo(10).FaceName("Consolas"))
#else
#define MONOSPACE_FONT wxFont(wxFontInfo(12).FaceName("Monospace"))
#endif

// Control key as displayed in tooltips.
#ifdef __WXMAC__
#define CTRL_KEY u8"⌘"
#else
#define CTRL_KEY "ctrl+"
#endif

// On Windows, the return key symbol is not displayed properly
#ifdef __WXMSW__
#define RETURN_KEY "Return"
#else
#define RETURN_KEY "↵"
#endif

// Default ratios for the main window
#define DEFAULT_PROJECT_RATIO 0.17
#define DEFAULT_INFO_RATIO 0.8
#define DEFAULT_CONSOLE_RATIO 0.8


// Extensions for Phonometrica files
#define PHON_EXT_PROJECT ".phon-project"
#define PHON_EXT_ANNOTATION ".phon-annot"
#define PHON_EXT_QUERY ".phon-query"
#define PHON_EXT_CONCORDANCE ".phon-conc"
#define PHON_EXT_SCRIPT ".phon"

#endif // PHONOMETRICA_MACROS_HPP
