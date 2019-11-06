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
	return std::make_shared<Query::Settings>(getType());
}

Query::Type ProtocolSearchBox::getType() const
{
	return Query::Type::CodingProtocol;
}

} // namespace phonometrica