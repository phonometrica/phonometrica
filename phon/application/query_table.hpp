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
 * Created: 12/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: result of a query.                                                                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_QUERY_TABLE_HPP
#define PHONOMETRICA_QUERY_TABLE_HPP

#include <phon/application/dataset.hpp>
#include <phon/application/search/query.hpp>
#include <phon/application/search/query_match.hpp>
#include <phon/application/protocol.hpp>

namespace phonometrica {

class QueryTable final : public Dataset
{
public:

	enum Flags
	{
		ShowNothing      = 0,
		ShowFields       = 1,
		ShowFileInfo     = 2,
		ShowMatchContext = 4,
		ShowMetadata     = 8,
		ShowAcoustics    = 16
	};

	QueryTable(AutoProtocol p, QueryMatchList matches, String label, const AutoQuerySettings &settings);

	String get_header(intptr_t j) const override;

	String get_cell(intptr_t i, intptr_t j) const override;

	intptr_t row_count() const override;

	intptr_t column_count() const override;

	String label() const override;

	bool empty() const override;

	bool is_query_table() const override { return true; }

	const AutoQueryMatch & get_match(intptr_t i) { return m_matches[i]; }

	void set_flags(int flags) { m_flags = flags; }

	intptr_t adjust_column(intptr_t j) const;

	const char *class_name() const override;

	bool has_textgrid() const;

	bool has_protocol() const { return m_protocol != nullptr; }

	bool has_split_fields() const;

	int field_count() const;

	bool is_acoustic_table() const;

	bool is_text_table() const;

	bool is_formant_table() const;

	bool is_pitch_table() const;

	bool is_intensity_table() const;

private:

	void load() override;

	void write() override;

	intptr_t category_count() const;

	int get_acoustic_field_count() const;

	String get_annotation_name(intptr_t i) const;

	String get_layer_index(intptr_t i) const;

	String get_start_time(intptr_t i) const;

	String get_end_time(intptr_t i) const;

	String get_left_context(intptr_t i) const;

	String get_matched_text(intptr_t i, intptr_t submatch_index = 1) const;

	String get_right_context(intptr_t i) const;

	String get_property(intptr_t i, const String &category) const;

	String get_acoustic_cell(intptr_t i, intptr_t j) const;

	String get_acoustic_header(intptr_t j) const;

	AutoProtocol m_protocol; // may be null

	QueryMatchList m_matches;

	std::set<String> m_categories;

	String m_label;

	AutoQuerySettings m_settings;

	int m_flags = 0;
};

using AutoQueryTable = std::shared_ptr<QueryTable>;

} // namespace phonometrica

#endif // PHONOMETRICA_QUERY_TABLE_HPP
