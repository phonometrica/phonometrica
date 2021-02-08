/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Abstract base class for tabular datasets, where each column represents a variable and each row represents  *
 * an observation. Derived classes are Spreadsheet, which represents a CSV file, and Concordance, which is the base    *
 * for all the types of concordances available in Phonometrica.                                                        *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DATASET_HPP
#define PHONOMETRICA_DATASET_HPP

#include <phon/application/vfs.hpp>

namespace phonometrica {

class Runtime;

class Dataset : public VFile
{
public:

	explicit Dataset(VFolder *parent, String path = String());

	bool is_dataset() const override;

	void from_xml(xml_node root, const String &project_dir);

	virtual String get_header(intptr_t j) const = 0;

	virtual String get_cell(intptr_t i, intptr_t j) const = 0;

	virtual intptr_t row_count() const = 0;

	virtual intptr_t column_count() const = 0;

	virtual bool empty() const = 0;

	virtual bool is_concordance() const { return false; }

	virtual bool is_spreadsheet() const { return false; }

	virtual void to_csv(const String &path, const String &sep);

	static void initialize(Runtime &rt);

private:

	void save_metadata() override;

	bool uses_external_metadata() const override;

};

using AutoDataset = std::shared_ptr<Dataset>;

namespace traits {
template<> struct maybe_cyclic<AutoDataset> : std::false_type { };
}

} // namespace phonometrica

#endif // PHONOMETRICA_DATASET_HPP
