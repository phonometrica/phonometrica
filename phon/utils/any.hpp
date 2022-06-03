/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2022 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
 *                                                                                                                    *
 * Created: 23/05/2020                                                                                                *
 *                                                                                                                    *
 * Purpose: std::any.                                                                                                 *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_ANY_HPP
#define PHONOMETRICA_ANY_HPP

// Frack you, Apple.
#if PHON_MACOS
#include <phon/third_party/any.hpp>
namespace std {
    using namespace linb;
}
#else
#include <any>
#endif

#endif // PHONOMETRICA_ANY_HPP
