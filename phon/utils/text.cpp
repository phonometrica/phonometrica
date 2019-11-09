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
 * Created: 20/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/utils/text.hpp>

namespace phonometrica { namespace utils {

Array<Array<String>> parse_csv(const String &path, std::string_view splitter, bool has_header)
{
	Array<Array<String>> csv;
	File infile(path);
	if (has_header) infile.read_line();

	while (!infile.at_end())
	{
		auto line = infile.read_line();
		line.trim_new_line();
		if (line.empty()) continue;
		auto fields = line.split(splitter);
		csv.append(std::move(fields));
	}
	infile.close();

	if (!csv.empty())
	{
		intptr_t size = csv.first().size();

		for (intptr_t i = 2; i <= csv.size(); i++)
		{
			if (csv[i].size() != size)
			{
				throw error("Inconsistent number of columns in CSV file on line % (expected %, got %)",
						i, size, csv[i].size());
			}
		}
	}

	return csv;
}

void write_csv(const String &path, const Array<Array<String>> &csv, std::string_view separator)
{
	File outfile(path, File::Write);

	for (auto &ln : csv)
	{
		outfile.write(String::join(ln, separator));
		outfile.write('\n');
	}
}

Array<double> read_matrix(const String &path, std::string_view splitter, bool has_header)
{
	auto csv = parse_csv(path, splitter, has_header);
	auto nrow = csv.size();
	if (nrow == 0) {
		throw error("Empty matrix");
	}
	auto ncol = csv.first().size();
	Array<double> M(nrow, ncol);

	for (intptr_t i = 1; i <= nrow; i++)
	{
		auto &row = csv[i];

		for (intptr_t j = 1; j <= ncol; j++)
		{
			try {
				M(i,j) = row[j].to_float();
			}
			catch (std::exception) {
				throw error("Invalid numeric value at row %, column %", i, j);
			}
		}
	}

	return M;
}

void write_matrix(const Array<double> &matrix, const String &path, std::string_view sep)
{
	File out(path, File::Write);
	out.write(matrix_to_csv(matrix, sep));
}


String matrix_to_csv(const Array<double> &matrix, std::string_view sep)
{
	assert(matrix.ndim() == 2);
	String result(matrix.size() * 2); // guesstimate capacity.

	for (intptr_t i = 1; i <= matrix.nrow(); i++)
	{
		for (intptr_t j = 1; j <= matrix.ncol(); j++)
		{
			result.append(String::convert(matrix(i,j)));

			if (j < matrix.ncol())
			{
				result.append(sep);
			}
			else
			{
				result.append("\n");
			}
		}
	}

	return result;
}

}} // namespace phonometrica::utils
