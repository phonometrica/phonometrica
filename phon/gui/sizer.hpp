//
// Created by julien on 04/02/2021.
//

#ifndef PHONOMETRICA_SIZER_HPP
#define PHONOMETRICA_SIZER_HPP

#include <wx/sizer.h>

namespace phonometrica {

class VBoxSizer : public wxBoxSizer
{
public:
	VBoxSizer() : wxBoxSizer(wxVERTICAL) { }
};

class HBoxSizer : public wxBoxSizer
{
public:
	HBoxSizer() : wxBoxSizer(wxHORIZONTAL) { }
};


}

#endif //PHONOMETRICA_SIZER_HPP
