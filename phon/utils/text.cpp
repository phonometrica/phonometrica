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
