/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 02/07/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cstdarg>
#include <phon/file.hpp>
#include <phon/error.hpp>
#include <phon/utils/helpers.hpp>
#include <phon/third_party/sol/unicode.hpp>

namespace phonometrica {

static const size_t MAX_FORMAT_LEN = 512;
static const unsigned char BOM_UTF8[] = {0xef, 0xbb, 0xbf};           // UTF-8
static const unsigned char BOM_UTF16_LE[] = {0xff, 0xfe};             // UTF-16, little-endian
static const unsigned char BOM_UTF16_BE[] = {0xfe, 0xff};             // UTF-16, big-endian
static const unsigned char BOM_UTF32_LE[] = {0xff, 0xfe, 0x00, 0x00}; // UTF-32, little-endian
static const unsigned char BOM_UTF32_BE[] = {0x00, 0x00, 0xfe, 0xff}; // UTF-32, big-endian


File::File(const String &path, File::Mode mode, Encoding enc) :
		m_path(path), m_mode(mode), m_owned(true)
{
	if (path.empty()) {
		throw error("Cannot create file with an empty path");
	}

    auto m = mode_to_string(mode);
    m_handle = utils::open_file(path.data(), m.data());

	if (m_handle == nullptr)
	{
		throw error("File not found: \"%\"", path);
	}

	// Handle byte-mark-order in UTF files
	if (m_mode & Write)
	{
		if (is_utf16())
		{
			throw error("UTF16 unsupported for output");
		}
		else if (is_utf32())
		{
			throw error("UTF32 unsupported for output");
		}
		else if (enc == Encoding::Undefined)
		{
			m_enc = Encoding::Utf8;
		}

		write_bom();
	}
	else if (mode & Read)
	{
		// Try to guess encoding based on BOM, defaulting to UTF8 if no BOM is present
		if (enc == Encoding::Undefined)
		{
			m_enc = get_bom();
		}
		skip_bom();

		if (m_enc == Encoding::Undefined)
		{
			m_enc = Encoding::Utf8;
		}
	}
}

File::File(FILE *handle, File::Mode mode) :
		m_handle(handle), m_mode(mode), m_owned(false)
{
	m_enc = Encoding::Undefined;
    // TODO: determine file encoding with handle?
}

File::File(const String &path, const String &mode) :
    File(path, mode, "utf-8")
{

}

File::File(const String &path, const String &mode, const String &encoding) :
    File(String(path), string_to_mode(mode), string_to_enc(encoding))
{

}

intptr_t File::size()
{
    check_handle();
	auto current_pos = ftell(m_handle);
	fseek(m_handle, 0, SEEK_END);
	auto end_pos = ftell(m_handle);

	/* restore prior position */
	fseek(m_handle, current_pos, SEEK_SET);

	return end_pos;
}

std::array<char, 4> File::mode_to_string(File::Mode mode) const
{
	std::array<char, 4> m;

	switch (mode)
	{
		case Read:
			m = {'r', 'b', '\0'};
			break;
		case Write:
			m = {'w', 'b', '\0'};
			break;
		case Append:
			m = {'a', 'b', '\0'};
			break;
		case ReadPlus:
			m = {'r', 'b', '+', '\0'};
			break;
		case WritePlus:
			m = {'w', 'b', '+', '\0'};
			break;
		case AppendPlus:
			m = {'a', 'b', '+', '\0'};
			break;
		default:
			throw error("Invalid file mode");
	}

    return m;
}

File::Mode File::string_to_mode(std::string_view mode) const
{
    if (mode == "r")
        return Read;
    if (mode == "w")
        return Write;
    if (mode == "a")
        return Append;
    if (mode == "r+")
        return ReadPlus;
    if (mode == "w+")
        return WritePlus;
    if (mode == "a+")
        return AppendPlus;

    throw error("Invalid file mode \"%\"", mode);
}

Encoding File::string_to_enc(std::string_view enc)
{
    // Default is UTF-8.
    if (enc == "utf-8" || enc == "ascii")
        return Encoding::Utf8;

    // We will try to guess the encoding.
    if (enc.empty())
        return Encoding::Undefined;

    // Assume platform's endianness.
    if (enc == "utf-16")
        return utils::is_big_endian() ? Encoding::Utf16be : Encoding::Utf16le;
    if (enc == "utf-32")
        return utils::is_big_endian() ? Encoding::Utf32be : Encoding::Utf32le;

    // Request a specific endianness.
    if (enc == "utf16-le")
        return Encoding::Utf16le;
    if (enc == "utf32-le")
        return Encoding::Utf32le;
    if (enc == "utf16-be")
        return Encoding::Utf16be;
    if (enc == "utf32-be")
        return Encoding::Utf32be;

    throw error("Invalid encoding \"%\"", enc);
}

void File::check_handle()
{
    if (m_handle == nullptr) {
        throw error("The file handle for \"%\" is not open", m_path);
    }
}

File::~File()
{
	this->close();
}

File::File(File &&other) noexcept
{
	m_handle = other.handle();
	m_mode = other.m_mode;
	m_enc = other.m_enc;
	m_owned = other.m_owned;

	other.m_handle = nullptr;
}

File &File::operator=(File &&other) noexcept
{
	this->close();
	m_handle = other.handle();
	m_mode = other.m_mode;
	m_enc = other.m_enc;
	m_owned = other.m_owned;

	other.m_handle = nullptr;

	return *this;
}

void File::skip_bom()
{
	// Assume we are the beginning of the file
	char tmp[4];

	switch (m_enc)
	{
		case Encoding::Utf8:
			// Skip 3 bytes
            if (fread(tmp, 3, 1, m_handle) != 1)
			{
				throw error("Could not read BOM marker");
			}
			break;
		case Encoding::Utf16be:
		case Encoding::Utf16le:
			// Skip 2 bytes
			if (fread(tmp, 2, 1, m_handle) != 1)
			{
				throw error("Could not read BOM marker");
			}
			break;
		case Encoding::Utf32be:
		case Encoding::Utf32le:
			// Skip 4 bytes
			if (fread(tmp, 4, 1, m_handle) != 1)
			{
				throw error("Could not read BOM marker");
			}
		default:
			break;
	}
}

Encoding File::get_bom()
{
	unsigned char ch[4] = {0, 0, 0, 0};
	auto r = fread(ch, 4, 1, m_handle);
	(void) r;

	rewind();

	if (ch[0] == BOM_UTF8[0] && ch[1] == BOM_UTF8[1] && ch[2] == BOM_UTF8[2])
	{
		return Encoding::Utf8;
	}
	if (ch[0] == BOM_UTF16_BE[0] && ch[1] == BOM_UTF16_BE[1])
	{
		return Encoding::Utf16be;
	}
	if (ch[0] == BOM_UTF16_LE[0] && ch[1] == BOM_UTF16_LE[1])
	{
		return Encoding::Utf16le;
	}
	if (ch[0] == BOM_UTF32_BE[0] && ch[1] == BOM_UTF32_BE[1] && ch[2] == BOM_UTF32_BE[2] && ch[3] == BOM_UTF32_BE[3])
	{
		return Encoding::Utf32be;
	}
	if (ch[0] == BOM_UTF32_LE[0] && ch[1] == BOM_UTF32_LE[1] && ch[2] == BOM_UTF32_LE[2] && ch[3] == BOM_UTF32_LE[3])
	{
		return Encoding::Utf32le;
	}

	return Encoding::Undefined;
}

bool File::needs_swap16() const
{
	static const bool big = utils::is_big_endian();
	return (big && m_enc == Encoding::Utf16le) || (!big && m_enc == Encoding::Utf16be);
}

bool File::needs_swap32() const
{
	static const bool big = utils::is_big_endian();
	return (big && m_enc == Encoding::Utf32le) || (!big && m_enc == Encoding::Utf32be);
}

char16_t File::get_char_utf16()
{
	char16_t c = 0;

	if (fread(&c, 2, 1, m_handle) != 1 || feof(m_handle))
	{
		return Eof;
	}

	return c;
}

char32_t File::get_char_utf32()
{
	char32_t c = 0;

	if (fread(&c, 4, 1, m_handle) != 1 || feof(m_handle))
	{
		return Eof;
	}

	return c;
}

void File::rewind()
{
    check_handle();
	std::rewind(m_handle);
}

String File::read_line_utf8()
{
    const size_t limit = 32;
    char buffer[limit];
	size_t pos = 0;
	char ch = 0;
	bool fetch_next = true;
	String line;

	while (fetch_next)
	{
		ch = (char) fgetc(m_handle);

		if (ch != EOF)
		{
			if (pos < limit)
			{
				buffer[pos++] = ch;
			}
			else
			{
				line.append({ buffer, limit });
                buffer[0] = ch;
                pos = 1;
			}
		}

		if ((ch == '\n') || (ch == EOF))
		{
			fetch_next = false;
            //buffer[pos] = '\0';
			line.append({ buffer, pos });
		}
	}

    return line;
}

String File::read_line_utf16()
{
	char16_t buffer[2];
	const bool needs_swap = needs_swap16();
	String str;

	do
	{
		char16_t* end = buffer + 1;
		if (needs_swap)
		{
			auto cu = get_char_utf16();
			buffer[0] = PHON_BYTESWAP16(cu);
		}
		else
		{
			buffer[0] = get_char_utf16();
		}
			
		if (is_high_surrogate(buffer[0]))
		{
			if (needs_swap)
			{
				auto cu = get_char_utf16();
				buffer[1] = PHON_BYTESWAP16(cu);
			}
			else
			{
				buffer[1] = get_char_utf16();
			}
			end++;
		}

		auto result = sol::unicode::utf16_to_code_point(buffer, end);

		if (result.error == sol::unicode::error_code::ok)
		{
			str.append(result.codepoint);
		}
		else
		{
			throw error("Invalid UTF-16 code point");
		}

		if (result.codepoint == Eol)
		{
			return str;
		}
	} while (buffer[0] != Eof);

	return str;
}

String File::read_line_utf32()
{
	char32_t ch;
	const bool needs_swap = needs_swap32();
	String str;

	do
	{
		if (needs_swap)
		{
			ch = get_char_utf32();
			ch = PHON_BYTESWAP32(ch);
		}
		else
		{
			ch = get_char_utf32();
		}
		str.append(ch);

		if (ch == Eol)
		{
			return str;
		}
	} while (ch != Eof);

	return str;
}

void File::close()
{
	if (m_owned && m_handle != nullptr)
	{
		fclose(m_handle);
		m_handle = nullptr;
	}
}

bool File::at_end()
{
    check_handle();
	return feof(m_handle) != 0;
}

void File::write(const char *text)
{
    check_handle();
    fputs(text, m_handle);
}

void File::write(char c)
{
	check_handle();
	fputc(c, m_handle);
}

void File::write(const String &text)
{
    write(text.data());
}

void File::write_line(const char *text)
{
    check_handle();
	fputs(text, m_handle);
#if PHON_WINDOWS
    fputc('\r', m_handle);
#endif
    fputc('\n', m_handle);
}

void File::write_line(const String &text)
{
    write_line(text.data());
}

String File::read_all(const String &path, Encoding enc)
{
	String line, text;
	File infile(path, Read, enc);

	while (!infile.at_end())
	{
		text.append(infile.read_line());
	}

	return text;
}

Array<String> File::read_lines()
{
    check_handle();
	Array<String> lines;

	while (!this->at_end())
	{
		lines.append(read_line());
	}

	return lines;
}

String File::read_line()
{
    check_handle();

	switch (m_enc)
	{
		case Encoding::Utf8:
			return read_line_utf8();

		case Encoding::Utf16be:
		case Encoding::Utf16le:
			return read_line_utf16();

		case Encoding::Utf32be:
		case Encoding::Utf32le:
			return read_line_utf32();

		default:
			throw error("No file encoding");
	}
}

int File::read_byte()
{
    check_handle();
	return fgetc(m_handle);
}

void File::write_byte(int c)
{
    check_handle();
	fputc((char) c, m_handle);
}

bool File::operator==(const File &other) const
{
	return this->handle() == other.handle() && this->handle() != nullptr;
}

void File::write_bom()
{
	if (is_utf8()) fputs("\xef\xbb\xbf", m_handle);
}

bool File::writable() const
{
    return m_mode & Write;
}

void File::format(const char *fmt, ...)
{
	char buffer[MAX_FORMAT_LEN];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	fputs(buffer, m_handle);
}

} // namespace phonometrica
