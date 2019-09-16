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

#ifndef PHONOMETRICA_STRING_HPP
#define PHONOMETRICA_STRING_HPP

#include <array>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>

#ifdef PHON_GUI
#include <QString>
#endif


#include <phon/array.hpp>
#include <phon/utils/refcount.hpp>

namespace phonometrica {


class Regex;

using Substring = std::string_view;

struct Codepoint
{
	char data[7] = { '\0' };
	uint8_t size = 0;

	operator Substring() const {
		return { data, size};
	}
};


class String final
{
public:

	typedef char value_type;
	typedef char *iterator;
	typedef const char *const_iterator;

	using char_traits = std::char_traits<char>;

    // Trimming options.
    enum class Option
    {
        Left  = 1,
        Right = 2,
        Both  = 3
    };

    enum class NormalForm
    {
    	NFC,  // normalization form composition
    	NFD,  // normalization form decomposition
    	NFKC, // normalization form compatibility composition
    	NFKD  // normalization form compatibility decomposition
    };
	// Iterators work on code units.
	iterator begin() noexcept { return impl->data; }
	const_iterator begin() const noexcept { return impl->data; }
	const_iterator cbegin() const noexcept { return begin(); }
	iterator end() noexcept { return impl->end; }
	const_iterator end() const noexcept { return impl->end; }
	const_iterator cend() const noexcept { return end(); }

	String() : impl(empty_string()) { }

	String(const String &) = default;

	String(String &&) noexcept = default;

	explicit String(intptr_t capacity) : impl(Data::create(capacity)) { }

	String(const char *str) : String(str, intptr_t(char_traits::length(str))) { }

	String(const char *str, intptr_t len) : impl(Data::create(str, len)) { }

	String(const std::string &s) : String(s.data(), intptr_t(s.size())) { }

	String(Substring s) : String(s.data(), intptr_t(s.size())) { }

	explicit String(const std::wstring &other);

#ifdef PHON_GUI
    String(const QString &str) :
        String(str.toUtf8())
    { }
#endif

	// Provided mostly for Windows.
	String(const wchar_t *s, intptr_t len);

	explicit String(const std::u16string &other);

	explicit String(const std::u32string &other);

	String(char32_t codepoint, intptr_t count);


	String &operator=(const String &other);
	String &operator=(String &&other) noexcept;

	intptr_t size() const { return impl->end - impl->data; }

	intptr_t grapheme_count() const;

	intptr_t capacity() const { return impl->limit - impl->data; }

	bool empty() const { return impl->end == impl->data; }

	const char *data() const { return impl->data; }


	size_t use_count() const { return impl->use_count(); }

	bool shared() const { return impl->shared(); }

	bool unique() const { return impl->unique(); }

	void reserve(intptr_t requested);

	void unshare();

	void swap(String &other) noexcept { std::swap(impl, other.impl); }

	bool operator==(const String &other) const;
	bool operator!=(const String &other) const;
	bool operator==(Substring other) const;
	bool operator!=(Substring other) const;
	bool operator==(const char *str) const;
	bool operator!=(const char *str) const;

	bool operator<(const String &other) const;
	bool operator>(const String &other) const;
	bool operator<=(const String &other) const;
	bool operator>=(const String &other) const;

	int compare(Substring other) const;
	int compare(const String &other) const;
    int compare(const char *other) const;

	operator Substring() const { return view(); }

#ifdef PHON_GUI
    operator QString() const { return QString::fromUtf8(data(), int(size())); }
#endif

	String &insert(iterator pos, Substring infix);
	String &insert(intptr_t pos, Substring infix);
	String &prepend(Substring prefix);
	String &append(Substring suffix);
	String &append(char32_t c);

	void push_back(char c);

	size_t hash() const;

	static Codepoint encode(char32_t c);

	char32_t next_codepoint(const_iterator &it) const;

	// Read the length of the current grapheme and update the iterator to point at the first byte after the grapheme.
	void next_grapheme(const_iterator &it, intptr_t &len) const;

	// Convenience method for the scripting engine.
	Substring next_grapheme(intptr_t i) const;

	// Read the length of the previous grapheme and update the iterator to point at the grapheme's first byte.
	void previous_grapheme(const_iterator &it, intptr_t &len) const;

	Substring next_grapheme(const_iterator &it) const;
	Substring previous_grapheme(const_iterator &it) const;

	intptr_t distance(const_iterator from, const_iterator to) const;

	// Advance iterator by count graphemes. If count is negative, iterate backward.
	void advance(const_iterator &it, intptr_t count) const;

	// Unicode conversion.
	std::u16string to_utf16() const { return to_utf16(*this); }
	static std::u16string to_utf16(std::string_view s);
	static String from_utf16(const std::u16string &s);

	std::u32string to_utf32() const { return to_utf32(*this); };
	static std::u32string to_utf32(std::string_view s);
	static String from_utf32(const std::u32string &s);

	std::wstring to_wide() const { return to_wide(this->data()); }
	static std::wstring to_wide(std::string_view s);
	static String from_wide(const std::wstring &s);
	static String from_wide(const wchar_t *s, intptr_t len);


	bool starts_with(Substring prefix) const;
	bool starts_with(char32_t codepoint) const;
	static bool starts_with(Substring self, Substring prefix);

	bool ends_with(Substring suffix) const;
	bool ends_with(char32_t codepoint) const;
	static bool ends_with(Substring self, Substring suffix);

	bool contains(Substring substring) const;
	bool contains(char32_t codepoint) const;
    static bool contains(Substring haystack, Substring needle);

    String &trim();
	String &ltrim();
	String &rtrim();

	void clear(bool flush = true);

	intptr_t count(Substring substring) const;
    static intptr_t count(Substring self, Substring substring);

	String repeat(intptr_t count) const;

	Substring mid(intptr_t from, intptr_t count = -1) const;
	Substring mid(const_iterator from, const_iterator to) const;
	Substring mid(const_iterator from, intptr_t count = -1) const;

	Substring rmid(const_iterator to, intptr_t count = -1) const;

	const_iterator index_to_iter(intptr_t i) const;
	iterator index_to_iter(intptr_t i);

	intptr_t find(Substring substring, intptr_t from = 1) const;
	const_iterator find(Substring substring, const_iterator from) const;
	intptr_t find(char32_t c, intptr_t from = 1) const;
	const_iterator find(char32_t c, const_iterator from) const;

	// TODO: check rfind
	intptr_t rfind(Substring infix, intptr_t pos = -1) const;
	const_iterator rfind(Substring infix, const_iterator pos) const;

	Substring view() const { return Substring(data(), size_t(size())); }

	String to_upper() const;
	String to_lower() const;

	String reverse() const;

	static String format(const char *fmt, ...);

	static String convert(bool b);
	static String convert(intptr_t n);
	static String convert(double n);

	static double to_float(Substring str, bool *ok = nullptr);
	double to_float(bool *ok = nullptr) const { return to_float(view(), ok); }

	static intptr_t to_int(Substring str, bool *ok = nullptr);
	intptr_t to_int(bool *ok = nullptr) const { return to_int(view(), ok); }

	static bool to_bool(Substring str, bool strict = false);
	bool to_bool(bool strict = false) const { return to_bool(view(), strict); }

	String &replace(intptr_t i, intptr_t count, Substring after);
	String &replace(const_iterator from, const_iterator to, Substring after);
	String &replace(Substring before, Substring after, intptr_t ntimes = -1);
	String &replace(Regex &pattern, String after, intptr_t ntimes = -1);
	String &replace(char before, char after);

	String &replace_first(Substring before, Substring after);
	String &replace_last(Substring before, Substring after);

	String &remove(intptr_t i, intptr_t count);
	String &remove(Substring substring, intptr_t ntimes = -1);

	String &remove_first(Substring substring);
	String &remove_last(Substring substring);

	char32_t first() const;
	char32_t last() const;

	String left(intptr_t count) const;
	String right(intptr_t count) const;

	String &fill(char32_t c, intptr_t count);

	static String join(const Array<String> &strings, Substring separator);

	Array<String> split(Substring separator) const;

	void chop(intptr_t new_size);

		// Replace positional arguments %1 to %9. This is similar to Qt's QString::arg method.
	String &arg(Substring arg1);
	String &arg(Substring arg1, Substring arg2);
	String &arg(Substring arg1, Substring arg2, Substring arg3);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7, Substring arg8);
	String &arg(Substring arg1, Substring arg2, Substring arg3, Substring arg4, Substring arg5, Substring arg6, Substring arg7, Substring arg8, Substring arg9);

	static bool is_letter(char32_t codepoint);

	// Case-insensitive comparison
	static bool iequals(Substring self, Substring other);

	// Case-insensitive find
	const_iterator ifind(Substring substring, const_iterator from) const;

private:

	friend struct Variant;

	struct Data : public Countable<Data,uint32_t>
	{
		// Constructor for the empty string
		Data() : end(data), limit(data + meta::pointer_size) { }

		Data(const char *str, intptr_t len, intptr_t capacity);

		static void operator delete(void* ptr, size_t);

		static Handle<Data> create(intptr_t capacity);

		static Handle<Data> create(const char *str, intptr_t len);

		static Handle<Data> create(const char *str, intptr_t len, intptr_t capacity);

		char32_t next_codepoint(String::const_iterator &it) const;

		char32_t previous_codepoint(String::const_iterator &it) const;

		void reset();

		// Cached length (in user perceived characters).
		uint32_t length = 0;

		// Cached hash value.
		size_t hash = 0;

		// Points to the end of the string (i.e. the null terminator).
		char *end;

		// Points to the first byte past the end of the allocated buffer.
		char *limit;

		// Beginning of the data (more is allocated after that).
		char data[meta::pointer_size] = { '\0' };
	};

	// Pointer to implementation.
	Handle<Data> impl;

	Handle<Data> empty_string();
	
#ifdef PHON_GUI
    String(const QByteArray &utf8) :
        String(utf8.data(), utf8.size())
    { }
#endif

	char* chars() { return impl->data;  }
	
	bool check_capacity(intptr_t requested) const;

	void adjust(intptr_t new_size);

	bool equals(const char *str, size_t len) const;

	static bool grapheme_break(char32_t c1, char32_t c2);

	static bool grapheme_break(char32_t c1, char32_t c2, int32_t *state);

	String &trim(Option flag);

	static intptr_t utf8_length(Substring str);
};

inline
std::ostream &operator<<(std::ostream &stream, const String &s)
{
    stream.write(s.data(), s.size());
    return stream;
}

} // namespace phonometrica

//----------------------------------------------------------------------------------------------------------------------

//inline
//std::ostream &operator<<(std::ostream &out, const phonometrica::String &s)
//{
//	return out.write(s.data(), s.size());
//}



//----------------------------------------------------------------------------------------------------------------------

namespace std {

template<>
struct hash<phonometrica::String>
{
	size_t operator()(const phonometrica::String &s) const noexcept
	{
		return s.hash();
	}
};

} // namespace std

#endif // PHONOMETRICA_STRING_HPP
