#ifndef PHONOMETRICA_GUI_HELPERS_HPP
#define PHONOMETRICA_GUI_HELPERS_HPP

#include <utility>

namespace phonometrica {

// Colours in sound and annotation views.

#define CURSOR_COLOUR wxColour(199, 179, 0)
#define ANCHOR_COLOUR wxColour(200, 0, 0)
#define WAVEBAR_SEL_COLOUR wxColour(0, 0, 204, 60)
#define PLOT_SEL_COLOUR wxColour(209, 116, 23, 50)
#define PLOT_SEL_TEXT_COLOUR wxColour(155, 101, 31)
#define LINE_COLOUR wxColour(0, 0, 0, 50)

// Structures to represent time or screen selections and locations.

// <start time, end time>
using TimeWindow = std::pair<double, double>;

// <start position, end position> (selection in the wavebar and zoom)
using PixelSelection = std::pair<double, double>;

// Selection in a sound plot
struct TimeSelection
{
	double t1, t2;	// times in seconds

	bool is_valid() const { return t1 >= 0.0; }

	bool is_point() const { return t1 == t2; }

	bool is_span() const { return !is_point(); }

	double duration() const { return t2 - t1; }

	void invalidate() { t1 = t2 = -1.0; }
};

}

#endif // PHONOMETRICA_GUI_HELPERS_HPP
