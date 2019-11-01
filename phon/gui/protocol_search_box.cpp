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
 * Created: 18/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/protocol_search_box.hpp>
#include <phon/gui/field_box.hpp>

namespace phonometrica {

ProtocolSearchBox::ProtocolSearchBox(const AutoProtocol &protocol, QWidget *parent, int context_length) :
		SearchBox(parent, protocol->name(), context_length), protocol(protocol)
{

}

AutoSearchNode ProtocolSearchBox::buildSearchTree()
{
	auto p = protocol.get();
	auto op = SearchConstraint::Opcode::Matches;
	auto rel = SearchConstraint::Relation::None;
	String value = this->get_pattern();

	return std::make_shared<SearchConstraint>(protocol, context_length, 1, p->layer_index(), p->layer_pattern(),
			p->case_sensitive(), op, rel, std::move(value));
}

void ProtocolSearchBox::setupUi(Runtime &rt)
{
	auto layout = new QGridLayout;
	layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    auto &fields = protocol->fields();

	int col = 0;
	int row = 0;
	QString label("All values");

	for (int i = 1; i <= fields.size(); i++)
	{
		auto &field = fields[i];
		QStringList values;
		for (auto &v: field.values) {
			values << v.text;
		}

		auto field_box = new FieldBox(field, label);
		layout->addWidget(field_box, row, col);
		m_fields << field_box;

		if (i % protocol->fields_per_row() == 0)
		{
			row++;
			col = 0;
		}
		else
			col++;
	}

	setLayout(layout);
}

String ProtocolSearchBox::get_pattern()
{
	QStringList values;

	for (auto fb : m_fields) {
		values << fb->get_pattern();
	}

	return values.join(protocol->field_separator());
}

AutoQuerySettings ProtocolSearchBox::getSettings() const
{
	return std::make_unique<Query::Settings>(Query::Type::CodingProtocol);
}

} // namespace phonometrica