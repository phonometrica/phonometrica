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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: nodes used to filter metadata in a query.                                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_META_NODE_HPP
#define PHONOMETRICA_META_NODE_HPP

#include <set>
#include <functional>
#include <unordered_set>
#include <phon/string.hpp>
#include <phon/regex.hpp>
#include <phon/application/annotation.hpp>

namespace phonometrica {

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
