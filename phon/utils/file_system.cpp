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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <functional>

#include <phon/utils/file_system.hpp>
#include <phon/utils/helpers.hpp>

#if PHON_WINDOWS
	#include <Shlwapi.h>
	#include <ShlObj.h>
#else

	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <libgen.h>
	#include <dirent.h>

#endif

#include <phon/utils/os.hpp>

namespace phonometrica {
namespace filesystem {


#if PHON_WINDOWS
	#define PHON_PATH_SEPARATOR "\\"
	#define PHON_PATH_SEPARATOR_CHAR '\\'
	#define PHON_PATH_ENV_DELIMITER ";"
	#define PHON_USER_DIRECTORY "USERPROFILE"
#else
	#define PHON_PATH_SEPARATOR "/"
	#define PHON_PATH_SEPARATOR_CHAR '/'
	#define PHON_PATH_ENV_DELIMITER ":"
	#define PHON_USER_DIRECTORY "HOME"
#endif

// application variables
static const std::string_view VAR_APPDIR("$PHON_APPDIR");
static const std::string_view VAR_HOME("$HOME");
static const std::string_view VAR_PROJECT("$PHON_PROJECT");

#define MAX_SIZE 1024

static inline void check_end(String &path)
{
	if (!path.ends_with(PHON_PATH_SEPARATOR_CHAR))
	{
		path.push_back(PHON_PATH_SEPARATOR_CHAR);
	}
}


String full_path(const String &relative_path)
{
	if (relative_path.starts_with('~'))
	{
		String path = relative_path;
		String user_dir(getenv(PHON_USER_DIRECTORY));

		return path.replace(1, 1, user_dir);
	}
	else
	{
		String result;

#if PHON_WINDOWS
		std::wstring out(MAX_SIZE, 0);
		String path("\\\\?\\"); /* Necessary with the Unicode version. */
		path.append(relative_path);
		auto in = path.to_wide();
		int size = GetFullPathName(in.data(), MAX_SIZE, (LPTSTR)out.data(), nullptr);

		if (size > 0) {
			out.resize(size);
			result = String(out);
		}
#elif defined(PHON_MACOS) || defined(_DEFAULT_SOURCE) || defined(_BSD_SOURCE)
		char buffer[MAX_SIZE];

		bool err = (realpath(relative_path.data(), buffer) == nullptr);

		if (not err)
		{
			result = buffer;
		}
#else
	#error full_path() unavailable on this platform
#endif
		if (result.empty())
		{
			throw error("Cannot get absolute path of \"%\": %", relative_path, utils::error_message());
		}

		return result;
	}
}

String user_directory()
{
#if PHON_WINDOWS
	WCHAR path[256];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) 
	{
		return String(path);
	}
	else
	{
		return String();
	}
#else
	char *dir = getenv(PHON_USER_DIRECTORY);
	return dir ? String(dir) : String();
#endif
}

String current_directory()
{
#if PHON_WINDOWS
	int size = GetCurrentDirectoryW(0, NULL);
	std::wstring dir(size, 0);

	if (GetCurrentDirectoryW(size, (LPTSTR)dir.data()) != 0) {
		return String(dir);
	}
#else
	char dir[MAX_SIZE];

	if (getcwd(dir, MAX_SIZE) != nullptr)
	{
		return String(dir);
	}
#endif

	throw error(utils::error_message());
}

void set_current_directory(const String &path)
{
	bool err;

#if PHON_WINDOWS
	std::wstring p = path.to_wide();
	err = (SetCurrentDirectoryW(p.data()) == 0);
#else
	err = (chdir(path.data()) != 0);
#endif

	if (err)
	{
		throw error("Cannot move to directory \"%\": %", path, utils::error_message());
	}
}

String separator()
{
	static String sep(PHON_PATH_SEPARATOR);
	return sep;
}

String join(std::string_view s1, std::string_view s2)
{
	if (s1.empty()) return s2;
	String path(s1.data(), s1.size());

	check_end(path);
	path.append(s2);

	return path;
}

void append(String &s1, std::string_view s2)
{
	check_end(s1);
	s1.append(s2);
}

String application_directory()
{
#if PHON_WINDOWS
	return filesystem::join(user_directory(), "AppData", "Roaming");

#elif PHON_MACOS
	return filesystem::join(user_directory(), "Applications");;

#else
	return filesystem::join(user_directory(), ".config");;
#endif
}

String temp_directory()
{
#if PHON_WINDOWS
	int length;
	wchar_t path[256];

	length = GetTempPathW(256, path);
	return String(path, intptr_t(length));

#elif defined(P_tmpdir)
	return String(P_tmpdir);
#else
	return String("/tmp");
#endif
}

String temp_file(std::string_view name)
{
	String f(temp_directory());
	append(f, name);

	return f;
}

String base_name(std::string_view path)
{
	auto pos = path.rfind(PHON_PATH_SEPARATOR_CHAR);

	if (pos == std::string_view::npos)
	{
		return path;
	}
	pos++; // skip separator.
	auto len = intptr_t(path.size() - pos);

	return String(path.data() + pos, len);
}

String directory_name(const String &path)
{
	auto size = path.size();
	auto str = path.data();
	intptr_t i;

	for (i = size; i-- > 0;)
	{
		if (str[i] == PHON_PATH_SEPARATOR_CHAR)
		{
			return String(path.data(), i);
		}
	}

	return String();
}

bool create_directory(const String &path)
{
	bool has_error;

#if PHON_WINDOWS
	auto p = path.to_wide();
	has_error = CreateDirectoryW((LPCWSTR)p.data(), nullptr) == 0;
#else
	has_error = mkdir(path.data(), S_IXUSR | S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP) != 0;
#endif

	if (has_error)
	{
		throw error("Cannot create directory \"%\": %\n", path, utils::error_message());
	}

	return !has_error;
}

bool remove_directory(const String &dir)
{
	if (!is_directory(dir))
	{
		throw error("Trying to remove \"%\" which is not a directory", dir);
	}

	bool has_error;

#if PHON_WINDOWS
	auto p = dir.to_wide();
	has_error = RemoveDirectoryW((LPCWSTR)p.data()) == 0;
#else

    clear_directory(dir);

    auto dirs = list_directory(dir, true);
    auto size = dirs.size();

    for (intptr_t i = 0; i < size; ++i)
    {
        String filename, path;

        filename = dirs[i];

        if (filename == "." || filename == "..")
        {
            continue;
        }

        path = join(dir, filename);

        if (is_directory(path))
        {
            remove_directory(path);
        }
        else
        {
            remove_file(path);
        }
    }

	has_error = rmdir(dir.data()) != 0;
#endif

	if (has_error)
	{
		throw error("Cannot remove directory \"%\": %\n", dir, utils::error_message());
	}

	return !has_error;
}

bool remove_file(const String &path)
{
	bool has_error = false;

	if (exists(path))
	{
#if PHON_POSIX
		has_error = std::remove(path.data()) != 0;

#else
		auto p = path.to_wide();
		has_error = DeleteFile(p.data()) == 0;
#endif

		if (has_error)
		{
			throw error("Cannot remove file \"%\": %", path, utils::error_message());
		}
	}
	else
	{
		throw error("Cannot remove \"%\": file does not exist", path);
	}

	return !has_error;
}

bool remove(const String &path)
{
	if (is_directory(path))
	{
		return remove_directory(path);
	}
	else
	{
		return remove_file(path);
	}
}

static void read_dir_entries(const String &path, std::function<void(String)> func, bool include_hidden)
{
#if PHON_WINDOWS
	WIN32_FIND_DATA ffd;
	//TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;


	// Prepare String for use with FindFile functions.  First, copy the
	// String to a buffer, then append '\*' to the directory name.
	auto utf16 = path.to_wide();
	utf16.append(L"\\*");
//	wcscpy(szDir, utf16.data());
//	wcscat(szDir, L"\\*");
//    stringCchCopy(szDir, MAX_PATH, (LPCWSTR)utf16.data());
//    stringCchCat(szDir, MAX_PATH, TEXT("\\*"));
	hFind = FindFirstFile(utf16.data(), &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		throw error("problem with FindFirstFile()");
		return;
	}

	do  {
		std::wstring utf16(ffd.cFileName, (size_t) wcslen(ffd.cFileName));
		if (utf16 != L"." && utf16 != L"..")
		{
			func(String(utf16));
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);

	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES)
	{
		throw error("problem with FindFirstFile()");
		return;
	}

	FindClose(hFind);

#elif PHON_POSIX
	DIR *dir;
	dirent *entry;
	dir = opendir(path.data());

	if (dir != nullptr)
	{
		while ((entry = readdir(dir)) != nullptr)
		{
			if (include_hidden || entry->d_name[0] != '.')
			{
				func(String(entry->d_name));
			}
		}
		closedir(dir);
	}
	else
	{
		throw error("Couldn't open directory \"%\"", path);
	}
#else
#error undefined function
#endif
}

Array<String> list_directory(const String &path, bool include_hidden)
{
    Array<String> entries;

	auto lambda = [&](const String &str) {
		entries.push_back(str);
	};
	read_dir_entries(path, lambda, include_hidden);
	std::sort(entries.begin(), entries.end());

	return entries;
}

bool exists(const String &path)
{
	if (path.empty())
	{
		return false;
	}
	else
	{
#if PHON_WINDOWS
		bool exists;
		auto p = path.to_wide();
		exists = PathFileExists((LPCWSTR)p.data()) != 0;

		return exists;
#else
		struct stat st;
		return (stat(path.data(), &st) == 0);
#endif
	}
}

bool is_directory(const String &path)
{
	if (path.empty())
	{
		return false;
	}
	else
	{
#if PHON_WINDOWS
		bool result;
		auto p = path.to_wide();
		result = PathIsDirectory((LPCWSTR)p.data()) != 0;

		return result;
#else
		struct stat st;
		bool status = (stat(path.data(), &st) == 0);
		if (not status)
		{ return false; }

		return S_ISDIR(st.st_mode);
#endif
	}
}

bool is_file(const String &path)
{
	if (path.empty())
	{
		return false;
	}
	else
	{
#if PHON_WINDOWS
		bool result;
		auto p = path.to_wide(); 
		result = PathFileExists((LPCWSTR)p.data()) && !PathIsDirectory((LPCWSTR)p.data());

		return result;
#else
		struct stat st;
		bool status = (stat(path.data(), &st) == 0);

		if (!status)
		{ return false; }
		return S_ISREG(st.st_mode);
#endif
	}
}

void clear_directory(const String &path)
{
	if (is_directory(path))
	{
#if PHON_WINDOWS
		bool result;
		auto p = path.to_wide();
		result = PathIsDirectoryEmpty((LPCWSTR)p.data()) != 0;
#elif defined(PHON_POSIX)
		DIR *dir;
		struct dirent *entry;
		bool result = true;
		dir = opendir(path.data());

		if (dir != nullptr)
		{
			while ((entry = readdir(dir)))
			{
				const char *name = entry->d_name;

				if (name[0] != '.')
				{
					if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0)
					{
						result = false;
						break;
					}
				}
			}
			closedir(dir);

			return;
		}
		else
		{
			throw error("Couldn't open directory \"%\"", path);
		}
#endif
	}
	else
	{
		throw error("\"%\" is not a directory", path);
	}
}

std::pair<String, String> split_ext(const String &path)
{
	auto data = path.data();
	intptr_t i;

	for (i = path.size(); i-- > 0;)
	{
		if (data[i] == '.')
		{
			break;
		}
	}

	if (i < 0)
	{
		return {{path.data(), path.size()}, String()};
	}
	else
	{
		String base(data, i);
		String ext(data + i, path.size() - i);

		return {base, ext};
	}
}

String strip_ext(const String &path)
{
	return split_ext(path).first;
}

String ext(String path, bool lower, bool strip_dot)
{
	auto data = path.data();
	intptr_t i;

	for (i = path.size(); i-- > 0;)
	{
		if (data[i] == '.')
		{
			if (strip_dot) i++;
			break;
		}
	}

	if (i < 0)
	{
		return String(); // empty path
	}

	String result(data + i, path.size() - i);

	if (lower)
	{
		return result.to_lower();
	}

	return result;
}

String &nativize(String &path)
{
#if PHON_WINDOWS
	path.replace('/', '\\');
#endif

	return path;
}

String &genericize(String &path)
{
#if PHON_WINDOWS
	path.replace('\\', '/');
#endif

    return path;
}

String temp_filename()
{
    return join(temp_directory(), utils::new_uuid());
}

void rename(std::string_view old_name, std::string_view new_name)
{
    int result;

#if PHON_WINDOWS
    auto wold = String::to_wide(old_name);
    auto wnew = String::to_wide(new_name);

    result = _wrename(wold.data(), wnew.data());
#else
    result = std::rename(old_name.data(), new_name.data());
#endif

    if (result != 0)
    {
        throw error(utils::error_message());
    }
}

void interpolate(String &path, std::string_view project_dir)
{
	if (path.starts_with(VAR_PROJECT)) {
		path.replace(1, VAR_PROJECT.size(), project_dir);
	}
	else if (path.starts_with(VAR_APPDIR)) {
		path.replace(1, VAR_APPDIR.size(), application_directory());
	}
	else if (path.starts_with(VAR_HOME)) {
		path.replace(1, VAR_HOME.size(), user_directory());
	}

	nativize(path);
}

void compress(String &path, std::string_view project_dir)
{
	if (path.starts_with(project_dir)) {
		path.replace(1, project_dir.size(), VAR_PROJECT);
	}
	else {
		auto app_dir = application_directory();

		if (path.starts_with(app_dir)) {
			path.replace(1, app_dir.size(), VAR_APPDIR);
		}
		else {
			auto user_dir = user_directory();

			if (path.starts_with(user_dir)) {
				path.replace(1, user_dir.size(), VAR_HOME);
			}
		}
	}

	genericize(path);
}

}
} // namespace phonometrica::filesystem

