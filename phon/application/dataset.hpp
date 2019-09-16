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
 * Purpose: tabular dataset, where each column represents a variable and each row represents an observation.           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_DATASET_HPP
#define PHONOMETRICA_DATASET_HPP

#include <phon/application/vfs.hpp>


namespace phonometrica {

class Dataset : public VFile
{
public:

	enum Type {
		Undefined,
		Native,
		Csv
	};

	explicit Dataset(VFolder *parent, String path = String());

	const char *class_name() const override;

	bool is_dataset() const override;

	void from_xml(xml_node root, const String &project_dir);

	virtual String get_header(intptr_t j) const = 0;

	virtual String get_cell(intptr_t i, intptr_t j) const = 0;

	virtual intptr_t row_count() const = 0;

	virtual intptr_t column_count() const = 0;

	virtual bool empty() const = 0;

	virtual bool is_query_dataset() const { return false; }

	virtual bool is_spreadsheet() const { return false; }

	virtual void to_csv(const String &path, const String &sep = "\t");

private:

	void load() override;

	void write() override;

	void save_metadata() override;

	Type guess_type() const;

	bool uses_external_metadata() const override;


	Type m_type = Undefined;

};

using AutoDataset = std::shared_ptr<Dataset>;

} // namespace phonometrica

#endif // PHONOMETRICA_DATASET_HPP
