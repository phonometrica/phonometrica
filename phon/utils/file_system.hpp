/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                      *
 *                                                                                                                    *
 * The contents of this file are subject to the Mozilla Public License Version 2.0 (the "License"); you may not use   *
 * this file except in compliance with the License. You may obtain a copy of the License at                           *
 * http://www.mozilla.org/MPL/.                                                                                       *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: path manipulation routines.                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_FILESYSTEM_HPP
#define PHONOMETRICA_FILESYSTEM_HPP

#include <phon/string.hpp>

namespace phonometrica { namespace filesystem {

String full_path(const String &relative_path);

String user_directory();

String current_directory();

void set_current_directory(const String &path);

String separator();

inline String join() { return String(); }

String join(std::string_view s1, std::string_view s2);

template<typename... Args>
String join(std::string_view s1, std::string_view s2, Args... args)
{
	return join(join(s1, s2), args...);
}

void append(String &s1, std::string_view s2);

String application_directory();

String temp_directory();

String temp_filename();

String temp_file(std::string_view name);

String base_name(std::string_view path);

String directory_name(const String &path);

bool create_directory(const String &path);

bool remove_directory(const String &dir, bool recursive);

bool remove_file(const String &path);

bool remove(const String &path);

Array<String> list_directory(const String &path, bool include_hidden = false);

bool exists(const String &path);

bool is_directory(const String &path);

bool is_file(const String &path);

bool clear_directory(const String &path);

void rename(std::string_view old_name, std::string_view new_name);


// Return a pair containing the path name without the extension as the first
// element, and the extension as the second element. If no extension was found,
// the extension is an empty String
std::pair<String, String> split_ext(const String &path);

String ext(String path, bool lower = false, bool strip_dot = false);

String strip_ext(const String &path);

// Convert to native separators, modifying the String in-place
String &nativize(String &path);

// Convert to generic (UNIX-like) separator (if needed), modifying the String in-place
String &genericize(String &path);


}} // namespace phonometrica::filesystem

#endif // PHONOMETRICA_FILESYSTEM_HPP
