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
 * Created: 07/09/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: nodes used to filter metadata in a query.                                                                 *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_META_NODE_HPP
#define PHONOMETRICA_META_NODE_HPP

#include <set>
#include <functional>
#include <unordered_set>
#include <phon/string.hpp>
#include <phon/regex.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

using AnnotationSet = std::set<AutoAnnotation, AnnotationLessComparator>;


enum class DescOperator
{
	Equals,
	Contains,
	Matches
};

struct MetaNode
{
	MetaNode() = default;
	virtual ~MetaNode() = default;

	virtual AnnotationSet filter(const AnnotationSet &files) = 0;
};

using AutoMetaNode = std::unique_ptr<MetaNode>;

struct DescriptionNode final : public MetaNode
{
	DescriptionNode(const String &value, DescOperator op, bool truth) :
		value(value)
	{
		this->op = op;
		this->truth = truth;
	}

	AnnotationSet filter(const AnnotationSet &files) override;

	String value;
	DescOperator op;
	bool truth;
};

struct PropertyNode : public MetaNode
{
	PropertyNode(const String &cat) : category(cat) { }

	String category;
};

struct TextPropertyNode final : public PropertyNode
{
	TextPropertyNode(const String &category, std::unordered_set<String> values) :
		PropertyNode(category), values(std::move(values)) { }

	AnnotationSet filter(const AnnotationSet &files) override;

	std::unordered_set<String> values;
};

struct NumericPropertyNode final : public PropertyNode
{
	NumericPropertyNode(const String &category, std::function<bool(double)> cb) :
		PropertyNode(category), callback(std::move(cb)) { }

	AnnotationSet filter(const AnnotationSet &files) override;

	std::function<bool(double)> callback;
};

struct BooleanPropertyNode final : public PropertyNode
{
	BooleanPropertyNode(const String &category, bool value) :
		PropertyNode(category) { this->value = value; }

	AnnotationSet filter(const AnnotationSet &files) override;

	bool value;
};


} // namespace phonometrica

#endif // PHONOMETRICA_META_NODE_HPP
