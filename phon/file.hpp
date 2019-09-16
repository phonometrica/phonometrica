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
 * Purpose: File object.                                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FILE_HPP
#define PHONOMETRICA_FILE_HPP

#include <cstdio>
#include <array>
#include <phon/string.hpp>

namespace phonometrica {

enum class Encoding : uint8_t
{
	Undefined, // No encoding
	Utf8,      // Default encoding
	Utf16le,   // Read-only
	Utf16be,   // Read-only
	Utf32le,   // Read-only
	Utf32be    // Read-only
};


class File final
{
public:

	// UTF end-of-line character
	const char32_t Eol = 0x0000000A;

	// UTF end-of-file character
	const char32_t Eof = 0xFFFF;

	// UTF end-of-text character
	const char32_t Eot = 0x00000003;

	enum Mode : uint8_t
	{
		Read = 1,
		Write = 2,
		Append = 8,
		Plus = 16,
		ReadPlus = Read | Plus,
		WritePlus = Write | Plus,
		AppendPlus = Append | Plus
	};

	explicit File(const String &path, Mode mode = Read, Encoding enc = Encoding::Undefined);

	explicit File(FILE *handle, Mode = Read);

	// Constructors the scripting engine.
	File(const String &path, const String &mode);

	File(const String &path, const String &mode, const String &encoding);

	File(const File &other) = delete;

	File(File &&other) noexcept;

	~File();

	bool writable() const;

	File &operator=(File &&other) noexcept;

	bool is_utf8() const;

	bool is_utf16() const;

	bool is_utf16le() const;

	bool is_utf16be() const;

	bool is_utf32() const;

	bool is_utf32le() const;

	bool is_utf32be() const;

	FILE *handle() const;

	String path() const;

	void rewind();

	intptr_t size();

	void close();

	// Check whether the end of the file has been reached
	bool at_end();

	void write(char c);

	// Write a string to this file
	void write(const char *text);

	void write(const String &text);

	// Write a string to this file and add a line ending
	void write_line(const char *text);

	void write_line(const String &text);

	void format(const char *fmt, ...);

	// Read a file at once into one large string
	static String read_all(const String &path, Encoding enc = Encoding::Undefined);

	// Get the lines in a file
	Array<String> read_lines();

	// Read one line from the file, converting it to UTF8 on the fly
	String read_line();

	int read_byte();

	void write_byte(int c);

	bool operator==(const File &other) const;

	static Encoding string_to_enc(std::string_view enc);

private:

	void check_handle();

	char16_t get_char_utf16();

	char32_t get_char_utf32();

	String read_line_utf8();

	String read_line_utf16();

	String read_line_utf32();

	// Skip byte order marker.
	void skip_bom();

	// Write byte order marker.
	void write_bom();

	// Get byte order marker.
	Encoding get_bom();

	// Needs to swap bytes for a UTF-16 file
	bool needs_swap16() const;

	// Needs to swap bytes for a UTF-32 file
	bool needs_swap32() const;

	std::array<char, 4> mode_to_string(Mode mode) const;

	Mode string_to_mode(std::string_view mode) const;

	String m_path;

	FILE *m_handle = nullptr;

	Mode m_mode = Read;

	Encoding m_enc = Encoding::Undefined;

	bool m_owned = true; // whether we own the file handle.

	static bool is_low_surrogate(char16_t c)
	{
		return c >= 0xDC00 && c <= 0xDFFF;
	}

	static bool is_high_surrogate(char16_t c)
	{
		assert(!is_low_surrogate(c));
		return c >= 0xD800 && c <= 0xDBFF;
	}

};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////                                      INLINE PUBLIC METHODS                                               /////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


inline
FILE *File::handle() const
{ return m_handle; }

inline
bool File::is_utf8() const
{ return m_enc == Encoding::Utf8; }

inline
bool File::is_utf16() const
{ return is_utf16be() || is_utf16le(); }

inline
bool File::is_utf16le() const
{ return m_enc == Encoding::Utf16le; }

inline
bool File::is_utf16be() const
{ return m_enc == Encoding::Utf16be; }

inline
bool File::is_utf32() const
{ return is_utf32be() || is_utf32le(); }

inline
bool File::is_utf32le() const
{ return m_enc == Encoding::Utf32le; }

inline
bool File::is_utf32be() const
{ return m_enc == Encoding::Utf32be; }

inline
String File::path() const
{
	return m_path;
}

} // namespace phonometrica

#endif // PHONOMETRICA_FILE_HPP
