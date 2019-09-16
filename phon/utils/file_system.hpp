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
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: path manipulation routines.                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PATH_HPP
#define PHONOMETRICA_PATH_HPP

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

bool remove_directory(const String &dir);

bool remove_file(const String &path);

bool remove(const String &path);

Array<String> list_directory(const String &path, bool include_hidden = false);

bool exists(const String &path);

bool is_directory(const String &path);

bool is_file(const String &path);

void clear_directory(const String &path);

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

void interpolate(String &path, std::string_view project_dir);

void compress(String &path, std::string_view project_dir);

}} // namespace phonometrica::filesystem

#endif // PHONOMETRICA_PATH_HPP
