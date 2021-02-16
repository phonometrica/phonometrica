/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: Search constraint on metadata.                                                                             *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_METACONSTRAINT_HPP
#define PHONOMETRICA_METACONSTRAINT_HPP

#include <phon/string.hpp>
#include <phon/regex.hpp>
#include <phon/application/vfs.hpp>
#include <phon/utils/xml.hpp>

namespace phonometrica {

struct MetaConstraint
{
	MetaConstraint() = default;

	virtual ~MetaConstraint() = default;

	virtual bool filter(const Document * file) const = 0;

	virtual void to_xml(xml_node node) = 0;
};

using AutoMetaConstraint = std::shared_ptr<MetaConstraint>;

//----------------------------------------------------------------------------------------------------------------------

struct DescMetaConstraint final : public MetaConstraint
{
	enum class Operator
	{
		Equal = 0, // this must be 0 to match the selector in the GUI
		NotEqual,
		Contains,
		NotContains,
		Match,
		NotMatch
	};

	DescMetaConstraint(Operator op, const String &value);

	static const char * op_to_name(Operator op);

	static Operator name_to_op(std::string_view name);

	bool filter(const Document * file) const override;

	void to_xml(xml_node node) override;

	Operator op;

	String value;

	// If we use a match on the regex, compile the regex once
	std::unique_ptr<Regex> regex;

};

//----------------------------------------------------------------------------------------------------------------------

struct PropertyMetaConstraint : public MetaConstraint
{
	PropertyMetaConstraint(const String &category) :
		MetaConstraint(), category(category)
	{ }

	String category;
};

struct TextMetaConstraint : public PropertyMetaConstraint
{
	TextMetaConstraint(const String &category, Array<String> values) :
		PropertyMetaConstraint(category), values(std::move(values))
	{ }

	bool filter(const Document * file) const override;

	void to_xml(xml_node node) override;

	Array<String> values;
};

struct NumericMetaConstraint : public PropertyMetaConstraint
{
	enum class Operator
	{
		None = 0, // this must be 0 to match the selector in the GUI
		Equal,
		NotEqual,
		Less,
		LessEqual,
		Greater,
		GreaterEqual,
		InclusiveRange,
		ExclusiveRange
	};

	NumericMetaConstraint(const String &category, Operator op, const std::pair<double,double> &value) :
		PropertyMetaConstraint(category), op(op), value(value)
	{ }

	bool filter(const Document * file) const override;

	static const char * op_to_name(Operator op);

	static Operator name_to_op(std::string_view name);

	void to_xml(xml_node node) override;

	bool check_value(double num) const;

	Operator op;

	// The second member is only used by InclusiveRange.
	std::pair<double,double> value;
};

struct BooleanMetaConstraint : public PropertyMetaConstraint
{
	BooleanMetaConstraint(const String &category, bool value) :
		PropertyMetaConstraint(category), value(value)
	{ }

	bool filter(const Document * file) const override;

	void to_xml(xml_node node) override;

	bool value;
};

} // namespace phonometrica

#endif // PHONOMETRICA_METACONSTRAINT_HPP
