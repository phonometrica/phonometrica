
#ifndef PHONOMETRICA_BUTTON_STYLESHEET_HPP
#define PHONOMETRICA_BUTTON_STYLESHEET_HPP

namespace phonometrica {

static const char *flat_button_stylesheet = R"__(
	QToolButton { background-color: transparent; }
	QToolButton:hover { background-color: qlineargradient(
            x1: 0, y1: 0, x2: 0, y2: 1,
            stop: 0 #dadbde, stop: 1 #f6f7fa;

			border-top: 3px transparent;
            border-bottom: 3px transparent;
            border-right: 3px transparent;
            border-left: 3px transparent;
        ); }
)__";

}

#endif // PHONOMETRICA_BUTTON_STYLESHEET_HPP
