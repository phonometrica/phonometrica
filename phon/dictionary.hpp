#ifndef PHONOMETRICA_DICTIONARY_HPP
#define PHONOMETRICA_DICTIONARY_HPP

#include <phon/hashmap.hpp>
#include <phon/string.hpp>

namespace phonometrica {

template <typename Val>
using Dictionary = Hashmap<String, Val>;

}

#endif // PHONOMETRICA_DICTIONARY_HPP
