#ifndef PHONOMETRICA_GUI_HELPERS_HPP
#define PHONOMETRICA_GUI_HELPERS_HPP

namespace phonometrica {

// Structures to represent time or screen selections.

struct TimeSelection
{
	double from, to;
};

struct PixelSelection
{
	double from, to;
};

}

#define WAVEBAR_SEL_COLOUR wxColour(68, 18, 232, 60)

#endif // PHONOMETRICA_GUI_HELPERS_HPP
