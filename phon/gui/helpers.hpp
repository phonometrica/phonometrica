#ifndef PHONOMETRICA_GUI_HELPERS_HPP
#define PHONOMETRICA_GUI_HELPERS_HPP

#include <utility>

namespace phonometrica {

// Colours in sound and annotation views.

#define CURSOR_COLOUR wxColour(199, 179, 0)
#define ANCHOR_COLOUR wxColour(200, 0, 0)
#define WAVEBAR_SEL_COLOUR wxColour(96, 96, 96, 75)
#define PLOT_SEL_COLOUR wxColour(209, 116, 23, 50)
#define PLOT_SEL_TEXT_COLOUR wxColour(155, 101, 31)
#define LINE_COLOUR wxColour(0, 0, 0, 50)

// Structures to represent time or screen selections and locations.

// <start time, end time>
using TimeSpan = std::pair<double, double>;

// <start position, end position>
using PixelSelection = std::pair<double, double>;

// <position, time>
using TimeAnchor = std::pair<int, double>;

}

#endif // PHONOMETRICA_GUI_HELPERS_HPP
