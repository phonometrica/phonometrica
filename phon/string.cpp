/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
 *                                                                                                                     *
 * Created: 02/07/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cstring>
#include <cstdarg>
#include <cmath>
#include <phon/string.hpp>
#include <phon/error.hpp>
#include <phon/regex.hpp>
#include <phon/utils/alloc.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/third_party/utf8/utf8.h>
#include <phon/third_party/murmur_hash2.hpp>
#include <phon/third_party/utf8proc/utf8proc.h>
#include <phon/third_party/sol/unicode.hpp>
#include <phon/third_party/utf8.hpp>

#define MAX_FORMAT_BUFFER 256

#define USE_WINDOWS_NATIVE_UTF 1

#if PHON_WINDOWS
#   if USE_WINDOWS_NATIVE_UTF
#       include <Windows.h>
#   endif
#endif

namespace phonometrica {

Handle<String::Data> String::empty_string()
{
	static Data e;
	return Handle<Data>(&e, Handle<Data>::Retain());
}

String::String(const std::wstring &other) :
		String(from_wide(other))
{

}

String::String(const wchar_t *s, intptr_t len) :
		String(from_wide(s, len))
{

}

String::String(const std::u16string &other) :
		String(from_utf16(other))
{

}

String::String(const std::u32string &other) :
		String(from_utf32(other))
{

}

String::String(char32_t codepoint, intptr_t count) :
		String(count)
{
	for (intptr_t i = 0; i < count; ++i) {
		append(codepoint);
	}
}

void String::reserve(intptr_t requested)
{
	if (this->shared() || !check_capacity(requested))
	{
		auto new_capacity = utils::find_capacity(requested, this->capacity());
		String tmp(new_capacity);
		char_traits::copy(tmp.chars(), this->data(), this->size());
		tmp.adjust(this->size());
		swap(tmp);
	}
}

bool String::check_capacity(intptr_t requested) const
{
	return impl->data + requested < impl->limit;
}

void String::adjust(intptr_t new_size)
{
	impl->end = impl->data + new_size;
	*impl->end = 0;
	impl->reset();
}

bool String::operator==(Substring other) const
{
	return equals(other.data(), other.size());
}

bool String::operator!=(Substring other) const
{
	return !(*this == other);
}

bool String::operator<(const String &other) const
{
	return compare(other) < 0;
}

bool String::operator>(const String &other) const
{
	return compare(other) > 0;
}

bool String::operator<=(const String &other) const
{
	return compare(other) <= 0;
}

bool String::operator>=(const String &other) const
{
	return compare(other) >= 0;
}

bool String::operator==(const String &other) const
{
	return impl == other.impl || equals(other.data(), other.size());
}

bool String::operator!=(const String &other) const
{
	return !equals(other.data(), other.size());
}

int String::compare(Substring other) const
{
	return compare(other.data());
}

int String::compare(const String &other) const
{
	return compare(other.data());
}

int String::compare(const char *other) const
{
	return strcmp(this->data(), other);
}

String &String::operator=(const String &other)
{
	if (this != &other)
	{
		String tmp(other);
		this->swap(tmp);
	}

	return *this;
}

String &String::operator=(String &&other) noexcept
{
	this->swap(other);
	return *this;
}

String &String::append(Substring suffix)
{
	auto new_size = intptr_t(this->size() + suffix.size());
	this->reserve(new_size + 1);
	std::copy(suffix.begin(), suffix.end(), this->end());
	this->adjust(new_size);

	return *this;
}

String &String::insert(String::iterator pos, Substring infix)
{
	if (infix.empty()) return *this;
	size_t count = pos - begin();

	if (pos > this->end())
	{
		throw error("[Index error] Cannot insert string at byte index % in string containing % code units",
				count+1, this->size());
	}

	auto new_size = this->size() + intptr_t(infix.size());

	// Make room for the infix. We don't call reserve() or unshare().
	if (this->shared() || !check_capacity(new_size + 1))
	{
		auto capacity = utils::find_capacity(new_size + 1, this->capacity());

		String tmp(capacity);
		// Copy head up to the insertion point.
		iterator it = tmp.begin();
		std::copy(this->begin(), pos, it);

		// Copy infix.
		it += count;
		std::copy(infix.begin(), infix.end(), it);

		// Copy tail.
		it += infix.size();
		std::copy(pos, this->end(), it);

		this->swap(tmp);
	}
	// We can modify the string in place by shifting the tail and copying the infix.
	else
	{
		size_t tail_size = this->end() - pos;
		char_traits::move(pos + infix.size(), pos, tail_size);
		std::copy(infix.begin(), infix.end(), pos);
	}
	this->adjust(new_size);

	return *this;
}

void String::unshare()
{
	String tmp(this->data(), this->size());
	this->swap(tmp);
}

String &String::prepend(Substring prefix)
{
	return insert(begin(), prefix);
}

size_t String::hash() const
{
	auto h = impl->hash;

	if (h == 0 && !this->empty())
	{
		h = hash_chars<sizeof(size_t)>(this->data(), size_t(this->size()), utils::random_seed());
		impl->hash = h;
	}

    return h;
}

bool String::equals(const char *str, size_t len) const
{
	return size_t(size()) == len && char_traits::compare(cbegin(), str, len) == 0;
}

bool String::operator==(const char *str) const
{
	return equals(str, char_traits::length(str));
}

bool String::operator!=(const char *str) const
{
	return !equals(str, char_traits::length(str));
}

String &String::append(char32_t c)
{
	if (c < 128)
	{
		push_back(char(c));
		return *this;
	}

	return append(encode(c));
}

Codepoint String::encode(char32_t c)
{
	Codepoint result;
	auto count = (uint8_t) utf8proc_encode_char((utf8proc_int32_t) c,
	                                            reinterpret_cast<utf8proc_uint8_t *>(result.data));
	result.data[count] = '\0';
	result.size = count;

	return result;
}

void String::push_back(char c)
{
	reserve(size() + 2); // char + null terminator
	*impl->end++ = c;
	impl->reset();
}

String::Data::Data(const char *str, intptr_t len, intptr_t capacity) :
	end(data + len), limit(data + capacity)
{
	if (str)
	{
		char_traits::copy(data, str, len);
	}

	*end = 0;
}

void String::Data::operator delete(void* ptr, size_t)
{
	utils::free(ptr);
}

Handle<String::Data> String::Data::create(intptr_t capacity)
{
	// [capacity] must be at least the size of a pointer, otherwise we might corrupt the heap when the Data
	// is constructed in-place because the whole array might be 0-initizialized.
	auto c = std::max<intptr_t>(capacity, meta::pointer_size);
	return create(nullptr, 0, c);
}

Handle<String::Data> String::Data::create(const char *str, intptr_t len)
{
	auto capacity = utils::find_capacity(len + 1);
	return create(str, len, capacity);
}

Handle<String::Data> String::Data::create(const char *str, intptr_t len, intptr_t capacity)
{
	constexpr intptr_t base_size = sizeof(Data) - meta::pointer_size;
	auto self = utils::alloc(base_size + capacity);
	auto data = new (self) Data(str, len, capacity);

	return Handle<Data>(data);
}

void String::Data::reset()
{
	hash = 0;
	length = 0;
	*end = 0;
}


Substring String::next_grapheme(String::const_iterator &it) const
{
	intptr_t len;
	auto start = it;
	next_grapheme(it, len);

	return Substring(start, size_t(len));
}

void String::next_grapheme(String::const_iterator &it, intptr_t &len) const
{
	if (it == this->cend())
	{
		len = 0;
		return;
	}
	auto start = it;
	char32_t current_codepoint = 0;
	char32_t next_codepoint = impl->next_codepoint(it);
	//int32_t state = 0;
	auto pos = it;

	while (it < this->cend())
	{
		current_codepoint = next_codepoint;
		next_codepoint = impl->next_codepoint(pos);

		if (grapheme_break(current_codepoint, next_codepoint))
		{
			len = it - start;
			return;
		}
		it = pos;
	}

	len = this->cend() - start;
}

void String::previous_grapheme(String::const_iterator &it, intptr_t &len) const
{
	if (it == this->cbegin())
	{
		len = 0;
		return;
	}

	auto end = it;
	char32_t current_codepoint = 0;
	char32_t previous_codepoint = impl->previous_codepoint(it);
	auto pos = it;

	while (it > this->cbegin())
	{
		current_codepoint = previous_codepoint;
		previous_codepoint = impl->previous_codepoint(pos);

		if (grapheme_break(previous_codepoint, current_codepoint))
		{
			len = end - it;
			return;
		}
		it = pos;
	}

	len = end - this->cbegin();
}

Substring String::previous_grapheme(String::const_iterator &it) const
{
	intptr_t len;
	previous_grapheme(it, len);

	return Substring(it, size_t(len));
}

void String::advance(String::const_iterator &it, intptr_t count) const
{
	intptr_t len;

	if (count >= 0)
	{
		while (count-- != 0 && it != this->cend())
		{
			next_grapheme(it, len);
		}
	}
	else
	{
		while (count++ < 0 && it != this->cbegin())
		{
			previous_grapheme(it, len);
		}
	}
}

intptr_t String::distance(String::const_iterator from, String::const_iterator to) const
{
	assert(from <= to);
	intptr_t len, dist = 0;

	while (from != to)
	{
		next_grapheme(from, len);
		dist++;
	}

	return dist;
}

char32_t String::next_codepoint(String::const_iterator &it) const
{
	return impl->next_codepoint(it);
}

bool String::grapheme_break(char32_t c1, char32_t c2)
{
	return bool(utf8proc_grapheme_break(utf8proc_int32_t(c1), utf8proc_int32_t(c2)));
}

bool String::grapheme_break(char32_t c1, char32_t c2, int32_t *state)
{
	return bool(utf8proc_grapheme_break_stateful(utf8proc_int32_t(c1), utf8proc_int32_t(c2), state));
}

char32_t String::Data::next_codepoint(String::const_iterator &it) const
{
	auto result = sol::unicode::utf8_to_code_point(it, (const char*)this->end);

	if (result.error != sol::unicode::error_code::ok)
	{
		throw error("[Unicode error] Invalid UTF-8 string: ", sol::unicode::to_string(result.error));
	}
	it = result.next;

	return result.codepoint;
}

char32_t String::Data::previous_codepoint(String::const_iterator &it) const
{
	// See https://stackoverflow.com/a/22257843
	do
	{
		if (it <= this->data)
		{
			throw error("[Unicode error] Cannot access code point before the beginning of the string");
		}
		it--;
	}
	while ((*it & 0xC0) == 0x80);

	auto result = sol::unicode::utf8_to_code_point(it, (const char*)this->end);

	if (result.error != sol::unicode::error_code::ok)
	{
		throw error("[Unicode error] Invalid UTF-8 string: ", sol::unicode::to_string(result.error));
	}

	return result.codepoint;
}

intptr_t String::grapheme_count() const
{
	if (impl->length == 0 && !this->empty())
	{
		auto it = begin();
		uint32_t count = 1;
		char32_t current_codepoint = 0;
		char32_t next_codepoint = impl->next_codepoint(it);

		while (it < this->end())
		{
			current_codepoint = next_codepoint;
			next_codepoint = impl->next_codepoint(it);

			if (grapheme_break(current_codepoint, next_codepoint))
			{
				++count;
			}
		}

		impl->length = count;
	}

	return intptr_t(impl->length);
}

String wide_to_utf8(const wchar_t *s, intptr_t len);

std::u16string String::to_utf16(std::string_view s)
{
	std::u16string result;
	utf8::utf8to16(s.begin(), s.end(), std::back_inserter(result));

	return result;
}

String String::from_utf16(const std::u16string &s)
{
	String result(intptr_t(s.size()));
	utf8::utf16to8(s.begin(), s.end(), std::back_inserter(result));

	return result;
}

std::u32string String::to_utf32(std::string_view s)
{
	std::u32string result;
	utf8::utf8to32(s.begin(), s.end(), std::back_inserter(result));

	return result;
}

String String::from_utf32(const std::u32string &s)
{
	String result(intptr_t(s.size()));
	utf8::utf32to8(s.begin(), s.end(), std::back_inserter(result));

	return result;
}

std::wstring String::to_wide(std::string_view s)
{

#if PHON_WINDOWS
#	if USE_WINDOWS_NATIVE_UTF
	int count = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), NULL, 0);
	std::wstring result(count + 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), count);
	result.resize(count);
#	else
	std::wstring result;
	utf8::utf8to16(s.begin(), s.end(), std::back_inserter(result));
#	endif
#else
	static_assert(sizeof(wchar_t) == 4, "wide string <-> UTF-8 conversion is not supported on this platform");
	std::wstring result;
	utf8::utf8to32(s.begin(), s.end(), std::back_inserter(result));
#endif

	return result;
}

String String::from_wide(const std::wstring &s)
{
	return from_wide(s.data(), intptr_t(s.size()));
}

String String::from_wide(const wchar_t *s, intptr_t len)
{


#if PHON_WINDOWS
#	if USE_WINDOWS_NATIVE_UTF
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, s, len, NULL, 0, NULL, NULL);
	String result(utf8_size + 1);
	WideCharToMultiByte(CP_UTF8, 0, s, len, result.impl->data, utf8_size, NULL, NULL);
	result.impl->data[utf8_size] = '\0';
	result.adjust(utf8_size);
#	else
	String result(len + 1);
	auto end = s + len;
	utf8::utf16to8(s, end, std::back_inserter(result));
#	endif
#else
	String result(len + 1);
	auto end = s + len;
	utf8::utf32to8(s, end, std::back_inserter(result));
#endif
	
	return result;
}

bool String::contains(char32_t codepoint) const
{
	if (this->empty()) return false;

	if (codepoint < 128)
	{
		auto ch = char(codepoint);

		for (auto c : *this) {
			if (c == ch) return true;
		}

		return false;
	}

	return contains(encode(codepoint));
}

bool String::contains(Substring haystack, Substring needle)
{
#ifdef __APPLE__
	auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
#else
	auto it = std::search(haystack.begin(), haystack.end(), std::boyer_moore_searcher(needle.begin(), needle.end()));
#endif

	return it != haystack.end();
}

bool String::starts_with(Substring prefix) const
{
	return prefix.size() <= size_t(this->size()) &&
	       char_traits::compare(this->data(), prefix.data(), prefix.size()) == 0;
}

bool String::starts_with(char32_t codepoint) const
{
	if (codepoint < 128) {
		return !empty() && *begin() == char(codepoint);
	}

	return starts_with(encode(codepoint));
}

bool String::ends_with(Substring suffix) const
{
	if (suffix.size() > size_t(this->size()))
	{
		return false;
	}
	const char *start = this->end() - suffix.size();

	return char_traits::compare(start, suffix.data(), suffix.size()) == 0;
}

bool String::ends_with(char32_t codepoint) const
{
	if (codepoint < 128) {
		return !this->empty() && *(end() - 1) == char(codepoint);
	}

	return ends_with(encode(codepoint));
}

bool String::starts_with(Substring self, Substring prefix)
{
	if (prefix.size() > self.size()) {
		return false;
	}

	return char_traits::compare(self.data(), prefix.data(), prefix.size()) == 0;
}

bool String::ends_with(Substring self, Substring suffix)
{
	if (suffix.size() > self.size()) {
		return false;
	}
	const char *start = self.data() + self.size() - suffix.size();

	return char_traits::compare(start, suffix.data(), suffix.size()) == 0;
}

bool String::contains(Substring substring) const
{
	return contains(*this, substring);
}

String &String::trim()
{
	return trim(Option::Both);
}

String &String::ltrim()
{
	return trim(Option::Left);
}

String &String::rtrim()
{
	return trim(Option::Right);
}

String &String::trim(String::Option flag)
{
	intptr_t first = 0;
	intptr_t len = this->size();
	intptr_t last = len;

	if (static_cast<int>(flag) & static_cast<int>(Option::Left))
	{
		for (; first < len; first++)
		{
			if (!isspace(impl->data[first])) {
				break;
			}
		}
	}

	if (static_cast<int>(flag) & static_cast<int>(Option::Right))
	{
		for (; last > first; --last)
		{
			if (!isspace(impl->data[last - 1])) {
				break;
			}
		}
	}

	intptr_t new_size = last - first;

	if (new_size != len)
	{
		// Unshare manually to avoid copying and moving.
		if (this->shared())
		{
			String tmp(this->data() + first, new_size);
			this->swap(tmp);
		}
		else
		{
			char_traits::move(begin(), begin() + first, new_size);
			adjust(new_size);
		}
	}

	return *this;
}

void String::clear(bool flush)
{
	if (!flush && this->unique())
	{
		adjust(0);
	}
	else
	{
		String tmp;
		this->swap(tmp);
	}
}

intptr_t String::count(Substring substring) const
{
	return count(*this, substring);
}

intptr_t String::count(Substring self, Substring substring)
{
	intptr_t total = 0;
	size_t at = 0;

	while (at < self.size() && (at = self.find(substring, at)) != Substring::npos)
	{
		total++;
		at += substring.size();
	}

	return total;
}

String String::repeat(intptr_t count) const
{
	String result(this->size() * count);

	for (intptr_t i = 0; i < count; ++i)
	{
		result.append(*this);
	}

	return result;
}

std::string_view String::mid(intptr_t from, intptr_t count) const
{
	return mid(index_to_iter(from), count);
}

std::string_view String::mid(String::const_iterator from, String::const_iterator to) const
{
    assert(to >= from);

	if (from < begin() || to > end()) {
		throw error("Cannot extract % bytes at byte index %", to-from, from-begin()+1);
	}

    return std::string_view(from, size_t(to - from));
}

std::string_view String::mid(String::const_iterator from, intptr_t count) const
{
	auto to = from;

	// Go until the end
	if (count < 0)
	{
		to = end();
	}
	// Get `count` graphemes.
	else
	{
		advance(to, count);
	}

	return mid(from, to);
}

Substring String::rmid(String::const_iterator to, intptr_t count) const
{
	auto from = to;

	if (count < 0)
	{
		from = begin();
	}
	else
	{
		advance(from, -count);
	}

	return mid(from, to);
}

String::const_iterator String::index_to_iter(intptr_t i) const
{
	auto len = this->grapheme_count();

	if (i > 0 && i <= len)
	{
		auto it = begin();
		advance(it, --i);

		return it;
	}

	if (i >= -len && i < 0)
	{
		auto it = end();
		advance(it, i);

		return it;
	}

	throw error("[Index error] String index % out of range", i);
}

String::iterator String::index_to_iter(intptr_t i)
{
	auto len = this->grapheme_count();

	if (i > 0 && i <= len)
	{
		auto it = cbegin();
		advance(it, --i);

		return const_cast<iterator>(it);
	}

	if (i >= -len && i < 0)
	{
		auto it = cend();
		advance(it, i);

		return const_cast<iterator>(it);
	}

	throw error("[Index error] String index % out of range", i);
}

String &String::insert(intptr_t pos, Substring infix)
{
	auto it = index_to_iter(pos);
	return insert(it, infix);
}


intptr_t String::find(Substring substring, intptr_t from) const
{
	auto it = index_to_iter(from);
	auto result = find(substring, it);

	if (result == this->end()) {
		return 0; // not found!
	}

	// Return positive 1-based index.
	return distance(begin(), result) + 1;
}

String::const_iterator String::find(Substring substring, const_iterator from) const
{
#ifdef __APPLE__
	return std::search(from, this->end(), substring.begin(), substring.end());
#else
	return std::search(from, this->end(), std::boyer_moore_searcher(substring.begin(), substring.end()));
#endif
}

intptr_t String::find(char32_t c, intptr_t from) const
{
	auto result = find(c, index_to_iter(from));
	return (result == end()) ? 0 : distance(begin(), result) + 1;
}

String::const_iterator String::find(char32_t c, String::const_iterator from) const
{
	if (c < 128)
	{
		for (auto it = from; it != end(); it++)
		{
			if (*it == char(c)) {
				return it;
			}
		}

		return end(); // not found
	}

	return find(encode(c), from);
}

intptr_t String::rfind(Substring infix, intptr_t pos) const
{
	auto to = index_to_iter(pos);
	auto result = rfind(infix, to);

	return (result == end()) ? 0 : distance(begin(), result) + 1;
}

String::const_iterator String::rfind(Substring infix, String::const_iterator pos) const
{
	Substring s = this->view();
	auto result = s.rfind(infix, pos - this->begin());

	return result == Substring::npos ? end() : begin() + result;
}

String String::to_upper() const
{
	String result(this->size());
	auto it = begin();

	while (it != end())
	{
		char32_t c = impl->next_codepoint(it);
		result.append(utf8proc_toupper(c));
	}

	return result;
}

String String::to_lower() const
{
	String result(this->size());
	auto it = begin();

	while (it != end())
	{
		char32_t c = impl->next_codepoint(it);
		result.append(utf8proc_tolower(c));
	}

	return result;
}

String String::format(const char *fmt, ...)
{
	char buffer[MAX_FORMAT_BUFFER];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, MAX_FORMAT_BUFFER, fmt, args);
	va_end(args);

	return String(buffer);
}

String String::convert(bool b)
{
	return b ? String("true") : String("false");
}

String String::convert(intptr_t n)
{
	char buffer[MAX_FORMAT_BUFFER];
	snprintf(buffer, MAX_FORMAT_BUFFER, "%td", n);

	return String(buffer);
}

String String::convert(double n)
{
	char buffer[MAX_FORMAT_BUFFER];
	snprintf(buffer, MAX_FORMAT_BUFFER, "%.10f", n);

	// Manually convert to C locale.
	for (char *c = buffer; *c != '\0'; c++)
	{
		if (*c == ',')
		{
			*c = '.';
			break;
		}
	}

	return String(buffer);
}

String String::reverse() const
{
	String result(this->size());
	auto it = end();

	do {
		auto grapheme = previous_grapheme(it);
		result.append(grapheme);
	}
	while (it != begin());

	return result;
}

String &String::replace(intptr_t i, intptr_t count, Substring after)
{
	auto start = index_to_iter(i);
	const_iterator bound = start;
	advance(bound, count);

	return replace(start, bound, after);
}

String &String::replace(const_iterator from, const_iterator to, Substring after)
{
	if (from == to) return *this;
	bool ok = (begin() <= from && from <= end()) && (begin() <= to && to <= end()) && (to > from);
	intptr_t offset = from - begin();
	intptr_t count = to - from;
	intptr_t new_size = this->size() - count + after.size();

	if (!ok) {
		throw error("Cannot replace % code units at offset %", count, offset + 1);
	}

	if (this->shared() || !check_capacity(new_size + 1))
	{
		intptr_t capacity = utils::find_capacity(new_size + 1);
		String tmp(capacity);

		auto it = tmp.begin();
		std::copy(this->cbegin(), from, it);
		it += offset;
		std::copy(after.cbegin(), after.cend(), it);
		it += after.size();
		std::copy(to, this->cend(), it);
		tmp.adjust(new_size);
		this->swap(tmp);
	}
	else
	{
		auto dest = iterator(from) + after.size();
		char_traits::move(dest, to, this->end() - to);
		std::copy(after.cbegin(), after.cend(), iterator(from));
		this->adjust(new_size);
	}

	return *this;
}

String &String::replace(Substring before, Substring after, intptr_t ntimes)
{
	auto pos = begin();
	intptr_t counter = 0;

	if (ntimes < 0) ntimes = (std::numeric_limits<intptr_t>::max)();

	while (pos < end() && counter < ntimes)
	{
		pos = const_cast<iterator>(find(before, pos));
		intptr_t offset = pos - begin();

		if (pos != end())
		{
			this->replace(pos, pos + before.size(), after);
			// The string might have been reallocated because it was shared or because its capacity was too limited,
			// so we recompute the position.
			pos = begin() + offset + after.size();
			++counter;
		}
	}

	return *this;
}

String &String::replace(Regex &pattern, String after, intptr_t ntimes)
{
	if (pattern.match(*this))
	{
		auto start = pattern.capture_start_iter(0);
		auto end = pattern.capture_end_iter(0);

		// Replace whole match (same as Perl's $&)
		after.replace("%%", pattern.capture(0), ntimes);

		// At most 9 captures can be replaced
		auto count = std::min<intptr_t>(pattern.count(), 9);

		for (intptr_t i = 1; i <= count; ++i)
		{
			String num = String::format("%%%td", i);
			after.replace(num, pattern.capture(i), ntimes);
		}

		replace(start, end, after);
	}

	return *this;
}

String &String::replace(char before, char after)
{
	unshare();

	for (intptr_t i = 0; i < this->size(); i++)
	{
		if (impl->data[i] == before)
			impl->data[i] = after;
	}

	return *this;
}

char32_t String::first() const
{
	auto it = begin();
	return impl->next_codepoint(it);
}

char32_t String::last() const
{
	auto it = end();
	return impl->previous_codepoint(it);
}

String String::left(intptr_t count) const
{
	auto it = begin();
	advance(it, count);

	return mid(begin(), it);
}

String String::right(intptr_t count) const
{
	auto it = end();
	advance(it, -count);

	return mid(it, end());
}

String &String::replace_first(Substring before, Substring after)
{
	return replace(before, after, 1);
}

String &String::replace_last(Substring before, Substring after)
{
	auto pos = const_cast<iterator>(rfind(before, end()));

	if (pos != end()) {
		replace(pos, pos + before.size(), after);
	}

	return *this;
}

String &String::remove(intptr_t i, intptr_t count)
{
	return replace(i, count, "");
}

String &String::remove(Substring substring, intptr_t ntimes)
{
	return replace(substring, "", ntimes);
}

String &String::remove_first(Substring substring)
{
	return replace_first(substring, "");
}

String &String::remove_last(Substring substring)
{
	return replace_last(substring, "");
}

String &String::fill(char32_t c, intptr_t count)
{
	for (intptr_t i = 0; i < count; ++i)
	{
		this->append(c);
	}

	return *this;
}

String String::join(const Array<String> &strings, Substring separator)
{
	String result;
	intptr_t size = 0;

	for (auto &s : strings) {
		size += s.size() + separator.size();
	}

	result.reserve(size);
	auto count = strings.size();

	for (intptr_t i = 1; i <= count; i++)
	{
		result.append(strings[i]);

		if (i < count) {
			result.append({separator.data(), separator.size()});
		}
	}

	return result;
}

Array<String> String::split(Substring separator) const
{
	Array<String> strings;
	intptr_t substring_start;
	intptr_t string_last; /* end position accounting for the size of the splitter */
	intptr_t len;
	intptr_t string_size = this->size();
	const char *substring, *current_chunk;

	/* The splitter needs to be longer than the string to be split */
	if (intptr_t(separator.size()) >= string_size)
	{
		strings.push_back(*this);
		return strings;
	}

	substring_start = 0;
	string_last = string_size - separator.size();

	for (intptr_t i = 0; i <= string_last; ++i)
	{
		current_chunk = data() + i;

		if (char_traits::compare(current_chunk, separator.data(), separator.size()) == 0)
		{
			/* Get the length of the current unmatched text */
			len = i - substring_start;

			/* Get pointer to current substring */
			substring = data() + substring_start;

			/* Add string to string list */
			strings.append(String(substring, len));

			/* Skip the replaced chunk */
			substring_start = i + separator.size();
			i = substring_start - 1;
		}
	}

	/* Get right substring */
	if (substring_start == string_size) { /* The end of the string matched the separator */
		strings.append(String());
	}
	else
	{
		len = string_size - substring_start;
		substring = data() + substring_start;
		strings.append(String(substring, len));
	}

	return strings;
}

void String::chop(intptr_t new_size)
{
	if (new_size >= this->size()) {
		return;
	}

	if (this->shared())
	{
		String tmp(this->data(), new_size);
		this->swap(tmp);
	}
	else
	{
		adjust(new_size);
	}
}

String &String::arg(Substring arg1)
{
	replace("%1", arg1);
	return *this;
}

String &String::arg(Substring arg1, Substring arg2)
{
	replace("%1", arg1);
	replace("%2", arg2);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);
	replace("%5", arg5);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);
	replace("%5", arg5);
	replace("%6", arg6);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);
	replace("%5", arg5);
	replace("%6", arg6);
	replace("%7", arg7);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7, Substring arg8)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);
	replace("%5", arg5);
	replace("%6", arg6);
	replace("%7", arg7);
	replace("%8", arg8);

	return *this;
}

String &String::arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7, Substring arg8, Substring arg9)
{
	replace("%1", arg1);
	replace("%2", arg2);
	replace("%3", arg3);
	replace("%4", arg4);
	replace("%5", arg5);
	replace("%6", arg6);
	replace("%7", arg7);
	replace("%8", arg8);
	replace("%9", arg9);

	return *this;
}

bool String::is_letter(char32_t codepoint)
{
	utf8proc_propval_t cat = utf8proc_get_property((utf8proc_int32_t) codepoint)->category;

	return cat >= UTF8PROC_CATEGORY_LU && cat <= UTF8PROC_CATEGORY_LO;
}

Substring String::next_grapheme(intptr_t i) const
{
	auto it = index_to_iter(i);
	return next_grapheme(it);
}

double String::to_float(Substring str, bool *ok)
{
	auto ch = str.begin();
	bool numeric = true;
	double value = std::nan("");

	// Recognize floating point numbers in the C locale.
	while (ch < str.end())
	{
		if (isdigit(*ch))
		{
			ch++;
		}
		else if (*ch == '.')
		{
			do {
				ch++;
			} while (ch != str.end());

			if (ch != str.end())
			{
				numeric = false;
				break;
			}
		}
		else
		{
			numeric = false;
			break;
		}
	}

	if (numeric) {
		sscanf(str.data(), "%lf", &value);
	}

	// Only throw an error if ok is null
	if (ok)
	{
		*ok = numeric;
	}
	else if (!numeric)
	{
		throw error("Could not convert string to number");
	}

	return value;
}

intptr_t String::to_int(Substring str, bool *ok)
{
	// MSVC doesn't let us convert std::string_view::iterator to char*.
	auto end = const_cast<char*>(str.data() + str.size());
	auto e = const_cast<char*>(str.data());
	auto result = strtol(str.data(), &e, 10);
	bool success = (e == end) || str.empty();

	if (ok) {
		*ok = success;
	}
	else if (!success) {
		throw error("Could not convert string to integer");
	}

	return intptr_t(result);
}

bool String::to_bool(Substring str, bool strict)
{
	if (strict)
	{
		if (str == "true" || str == "TRUE" || str == "1")
			return true;
		if (str == "false" || str == "FALSE" || str == "0")
			return false;

		throw error("Could not convert string to Boolean");
	}
	else
	{
		return !((str == "false") || (str == "FALSE"));
	}
}

bool String::iequals(Substring self, Substring other)
{
	using namespace sol::unicode;

	if (utf8_length(self) != utf8_length(other)) {
		return false;
	}

	auto it1 = self.begin();
	auto it2 = other.begin();

	while (it1 != self.end())
	{
		auto r1 = utf8_to_code_point(it1, self.end());
		auto r2 = utf8_to_code_point(it2, other.end());
		auto c1 = utf8proc_tolower(r1.codepoint);
		auto c2 = utf8proc_tolower(r2.codepoint);

		if (c1 != c2) return false;
		it1 = r1.next;
		it2 = r2.next;
	}

	return true;
}

intptr_t String::utf8_length(Substring str)
{
	intptr_t len;
	utf8_strlen(str.data(), str.size(), &len);

	return len;
}

String::const_iterator String::ifind(Substring substring, String::const_iterator from) const
{
	auto this_length = utf8_length(*this);
	auto sub_length = utf8_length(substring);

	if (this_length < sub_length || substring.empty()) {
		return this->cend();
	}

	auto it = from;

	while (it != this->cend())
	{
		auto part = mid(it, sub_length);
		if (iequals(part, substring))
		{
			return reinterpret_cast<const_iterator>(it);
		}
		advance(it, 1);
	}

	return this->cend();
}

} // namespace phonometrica