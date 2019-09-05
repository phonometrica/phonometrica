/**********************************************************************************************************************
 *                                                                                                                    *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                           *
 *                                                                                                                    *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public  *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any     *
 * later version.                                                                                                     *
 *                                                                                                                    *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more      *
 * details.                                                                                                           *
 *                                                                                                                    *
 * You should have received a copy of the GNU General Public License along with this program. If not, see             *
 * <http://www.gnu.org/licenses/>.                                                                                    *
 *                                                                                                                    *
 * Created: 20/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/file.hpp>
#include <phon/utils/text.hpp>

namespace phonometrica { namespace utils {

Array<Array<String>> parse_csv(const String &path, std::string_view splitter)
{
	Array<Array<String>> csv;
	File infile(path);

	while (!infile.at_end())
	{
		auto line = infile.read_line();
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
