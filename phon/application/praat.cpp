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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/praat.hpp>
#include <phon/regex.hpp>
#include <phon/error.hpp>
#include <phon/file.hpp>
#include <phon/utils/file_system.hpp>

extern "C" char *sendpraat (void *display, const char *programName, long timeOut, const char *text);
extern "C" wchar_t *sendpraatW (void *display, const wchar_t *programName, long timeOut, const wchar_t *text);

namespace phonometrica { namespace praat {

static Regex pattern_interval_tier("class\\s+=\\s+\"IntervalTier\"");
static Regex pattern_point_tier("class\\s+=\\s+\"TextTier\"");
static Regex pattern_name("name\\s+=\\s+\"(.*)\"");
static Regex pattern_size("size\\s+=\\s+(\\d*)");

static Regex pattern_interval("intervals\\s+\\[\\d*\\]:");
static Regex pattern_xmin("xmin\\s+=\\s+-?(\\d+\\.?\\d*)"); // Some files have -0 in the Seoul Speech Corpus
static Regex pattern_xmax("xmax\\s+=\\s+(\\d+\\.?\\d*)");
static Regex pattern_text("text\\s+=\\s+\"(.*)\"", Regex::Multiline);

static Regex pattern_point("points\\s+\\[\\d*\\]:");
static Regex pattern_time("number\\s+=\\s+(\\d+\\.?\\d*)");
static Regex pattern_mark("mark\\s+=\\s+\"(.*)\"", Regex::Multiline);

static
bool search(File &infile, Regex &pattern, String &result)
{
	String line;

	while (!infile.at_end())
	{
		line.append(infile.read_line());
		line.rtrim();

		if (pattern.match(line))
		{
			result = pattern.capture(1);
			return true;
		}
	}

	return false;
}

static
double validate(const String &value)
{
	bool ok = false;
	double result = value.to_float(&ok);

	if (!ok) {
		throw error("Invalid TextGrid file: conversion to float failed");
	}

	return result;
}

bool parse_tier_header(File &infile, const String &line, TierHeader &header)
{
	bool has_intervals = pattern_interval_tier.match(line);
	bool has_points = false;

	if (!has_intervals) {
		has_points = pattern_point_tier.match(line);
	}

	if (has_intervals || has_points)
	{
		search(infile, pattern_name, header.label);
		header.has_points = has_points;
		String tmp;
		search(infile, pattern_xmin, tmp);
		header.xmin = validate(tmp);
		search(infile, pattern_xmax, tmp);
		header.xmax = validate(tmp);
		search(infile, pattern_size, tmp); // discard

		return true;
	}

	return false;
}

bool parse_interval(File &infile, const String &line, Interval &interval)
{
	if (pattern_interval.match(line))
	{
		String result;

		search(infile, pattern_xmin, result);
		interval.xmin = validate(result);
		search(infile, pattern_xmax, result);
		interval.xmax = validate(result);
		bool ok = search(infile, pattern_text, interval.text);

		if (! ok) {
			throw error("Invalid TextGrid File: could not parse interval");
		}
		interval.text.replace("\"\"", "\"");

		return true;
	}

	return false;
}

bool parse_point(File &infile, const String &line, Point &point)
{
	if (pattern_point.match(line))
	{
		String result;

		search(infile, pattern_time, result);
		point.time = validate(result);
		bool ok = search(infile, pattern_mark, point.text);

		if (! ok) {
			throw error("Invalid TextGrid File: could not parse point");
		}
		point.text.replace("\"\"", "\"");

		return true;
	}

	return false;
}

static void run_temp_script(const String &script)
{
	using namespace filesystem;
	auto path = join(temp_directory(), "temp.praat");
	
	File outfile(path, File::Write);
	outfile.write(script);
	outfile.close();
	auto args = String("execute %1").arg(path);

	// sendpraatW doesn't seem to work. For now, we try to use sendpraat.
#if PHON_WINDOWS
	if (!path.is_ascii()) {
		throw error("Path \"%\" contains non-ASCII characters: this is not supported by sendpraat");
	}
#endif
#if 0
//#if PHON_WINDOWS
    auto wargs = args.to_wide();
    auto result = sendpraatW(nullptr, L"praat", 0, wargs.data());

    if (result != nullptr)
    {
		String msg(result);
		throw error(msg);
    }
#else
	auto result = sendpraat(nullptr, "praat", 0, args.data());

	if (result != nullptr)
		throw error(result);
#endif
	
	//filesystem::remove_file(path);
}


void open_textgrid(const String &tgd, const String &snd)
{
	String script;

	if (snd.empty())
	{
		script = String(
				"! Automatically generated by the Phonometrica\n"
				"Read from file: \"%1\"\n"
				"Edit\n"
		).arg(tgd);
	}
	else
	{
		script = String(
				"! Automatically generated by the Phonometrica\n"
				"Open long sound file: \"%1\"\n"
				"sound$ = selected$(\"LongSound\", -1)\n"
				"Read from file: \"%2\"\n"
				"textgrid$ = selected$(\"TextGrid\", -1)\n"
				"select LongSound 'sound$'\n"
				"plus TextGrid 'textgrid$'\n"
				"Edit\n"
		).arg(snd, tgd);
	}

	run_temp_script(script);
}

void open_sound(const String &path)
{
	String script = QString(
			"! Automatically generated by the Phonometrica\n"
			"Read from file: \"%1\"\n"
			"Edit\n"
	).arg(path);

	run_temp_script(script);
}

void open_interval(intptr_t tier, double xmin, const String &textgrid, const String &sound)
{
	String script;
	xmin += 0.0001; // make sure we're within the interval

	if (sound.empty())
	{
		script = utils::format(
				"! Automatically generated by the Dolmen\n"
				"uTier = %\n"
				"Read from file: \"%\"\n"
				"textgrid$ = selected$(\"TextGrid\", -1)\n"
				"interval = Get interval at time: uTier, %\n"
				"begin = Get starting point: uTier, interval\n"
				"begin = begin + 0.0001\n"
				"end = Get end point: uTier, interval\n\n"
				"select TextGrid 'textgrid$'\n"
				"Edit\n"
				"editor TextGrid 'textgrid$'\n"
				"	for z from 1 to uTier-1\n"
				"		Select next tier\n"
				"	endfor\n\n"
				"	Zoom: begin, end\n"
				"	Move cursor to: begin\n"
				"	Move cursor to begin of selection\n"
				"endeditor\n\n", tier, textgrid, xmin);
	}
	else
	{
		script = utils::format(
				"! Automatically generated by the Dolmen\n"
				"uTier = %\n"
				"Open long sound file: \"%\"\n"
				"sound$ = selected$(\"LongSound\", -1)\n"
				"Read from file: \"%\"\n"
				"textgrid$ = selected$(\"TextGrid\", -1)\n"
				"interval = Get interval at time: uTier %\n"
				"begin = Get starting point: uTier, interval\n"
				"begin = begin + 0.0001\n"
				"end = Get end point: uTier, interval\n\n"
				"select LongSound 'sound$'\n"
				"plus TextGrid 'textgrid$'\n"
				"Edit\n"
				"editor TextGrid 'textgrid$'\n"
				"	for z from 1 to uTier-1\n"
				"		Select next tier\n"
				"	endfor\n\n"
				"	Zoom: begin, end\n"
				"	Move cursor to: begin\n"
				"	Move cursor to begin of selection\n"
				"endeditor\n\n",
				tier, sound, textgrid, xmin);
	}

	run_temp_script(script);
}



}} // namespace phonometrica::praat