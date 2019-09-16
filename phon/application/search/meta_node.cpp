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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

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