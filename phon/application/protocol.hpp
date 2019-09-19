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
 * Created: 17/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: a query protocol defines the semantics of a coding scheme. It is translated as a widget with clickable     *
 * buttons, which is presented to the user in a query editor. The user's input is translated back into a regular       *
 * expression which is passed as a search pattern to a query.                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PROTOCOL_HPP
#define PHONOMETRICA_PROTOCOL_HPP

#include <memory>
#include <phon/string.hpp>
#include <phon/runtime/runtime.hpp>

namespace phonometrica {

// Helper structures.

struct SearchChoice final
{
	String match, text;
};

struct SearchValue final
{
	String match, text, layer_name;
	Array<SearchChoice> choices;
};

struct SearchField final
{
	String name, match_all, layer_pattern;
	Array<SearchValue> values;
};

//----------------------------------------------------------------------------------------------------------------------

class Protocol final
{
public:

	Protocol(Runtime &rt, const String &path);

	String version() const { return m_version; }

	String layer_pattern() const { return m_layer_pattern; }

	int layer_index() const { return m_layer_index; }

	bool case_sensitive() const { return m_case_sensitive; }

	String name() const { return m_name; }

	String field_separator() const { return m_separator; }

	intptr_t field_count() const { return m_fields.size(); }

	int fields_per_row() const { return m_fields_per_row; }

	const Array<SearchField> &fields() const { return m_fields; }

	String get_field_name(intptr_t i) const;

private:

	Runtime &runtime;

	Array<SearchField> m_fields;

	String m_path;

	String m_name;

	String m_version;

	String m_layer_pattern;

	String m_separator;

	int m_layer_index = 0; // search everywhere by default

	int m_fields_per_row = 3;

	int m_layer_selecting_field = -1;

	int m_layer_field = 0; // if this is positive, indicates which field is used to select the tier.

	bool m_case_sensitive = false;
};

using AutoProtocol = std::shared_ptr<Protocol>;

} // namespace phonometrica

#endif // PHONOMETRICA_PROTOCOL_HPP
