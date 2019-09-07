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
 * Purpose: see header.                                                                                               *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include <phon/application/search/meta_node.hpp>

namespace phonometrica {

AnnotationSet DescriptionNode::filter(const AnnotationSet &files)
{
	AnnotationSet new_files;

	for (auto &f : files)
	{
		auto &desc = f->description();
		bool result;

		switch (op)
		{
			case DescOperator::Equals:
			{
				result = (desc == value);
				break;
			}
			case DescOperator::Contains:
			{
				result = desc.contains(value);
				break;
			}
			case DescOperator::Matches:
			{
				Regex re(value);
				result = re.match(desc);
				break;
			}
		}

		if (result == truth)
		{
			new_files.insert(f);
		}
	}

	return new_files;
}

AnnotationSet TextPropertyNode::filter(const AnnotationSet &files)
{
	AnnotationSet new_files;

	for (auto &f : files)
	{
		for (auto &property : f->properties())
		{
			if (property.category() == this->category)
			{
				auto it = values.find(property.value());
				if (it != values.end()) new_files.insert(f);
				break;
			}
		}
	}

	return new_files;
}

AnnotationSet NumericPropertyNode::filter(const AnnotationSet &files)
{
	AnnotationSet new_files;

	for (auto &f : files)
	{
		for (auto &property : f->properties())
		{
			if (property.category() == this->category)
			{
				bool result = callback(property.numeric_value());
				if (result) new_files.insert(f);
				break;
			}
		}
	}

	return new_files;
}

AnnotationSet BooleanPropertyNode::filter(const AnnotationSet &files)
{
	AnnotationSet new_files;

	for (auto &f : files)
	{
		for (auto &property : f->properties())
		{
			if (property.category() == this->category)
			{
				bool result = (property.boolean_value() == this->value);
				if (result) new_files.insert(f);
				break;
			}
		}
	}

	return new_files;
}
} // namespace phonometrica