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
