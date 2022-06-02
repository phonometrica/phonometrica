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
 * Created: 02/06/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/regex.hpp>
#include <phon/file.hpp>
#include <phon/runtime/variant.hpp>
#include <phon/runtime/iterator.hpp>

namespace phonometrica {

Variant Iterator::get_value()
{
	throw error("[Type error] Type % only supports iteration over keys", object.class_name());
}


//---------------------------------------------------------------------------------------------------------------------

ListIterator::ListIterator(Variant v, bool ref_val) : Iterator(std::move(v), ref_val), pos(1)
{
	lst = &raw_cast<List>(object.resolve()).items();
}

Variant ListIterator::get_key()
{
	return pos;
}

Variant ListIterator::get_value()
{
	auto &value = (*lst)[pos++];
	if (ref_val) value.make_alias();

	return value;
}

bool ListIterator::at_end() const
{
	return pos > lst->size();
}


//---------------------------------------------------------------------------------------------------------------------

TableIterator::TableIterator(Variant v, bool ref_val) :
	Iterator(v, ref_val), it(raw_cast<Table>(v.resolve()).map().begin())
{
	map = &raw_cast<Table>(object.resolve()).map();
}

Variant TableIterator::get_key()
{
	return it->first;
}

Variant TableIterator::get_value()
{
	auto &value = (it++)->second;
	if (ref_val) value.make_alias();

	return value;
}

bool TableIterator::at_end() const
{
	return it == raw_cast<Table>(object).map().end();
}


//---------------------------------------------------------------------------------------------------------------------

StringIterator::StringIterator(Variant v, bool ref_val) : Iterator(std::move(v), ref_val)
{
	str = &raw_cast<String>(object.resolve());
}

Variant StringIterator::get_key()
{
	return pos;
}

Variant StringIterator::get_value()
{
	if (ref_val) {
		throw error("[Reference error] Cannot take a reference to a character in a string.\nHint: take the second loop variable by value, not by reference");
	}
	return str->next_grapheme(pos++);
}

bool StringIterator::at_end() const
{
	return pos > str->grapheme_count();
}


//---------------------------------------------------------------------------------------------------------------------

RegexIterator::RegexIterator(Variant v, bool ref_val) : Iterator(std::move(v), ref_val)
{
	re = &raw_cast<Regex>(object.resolve());
}

Variant RegexIterator::get_key()
{
	return pos;
}

Variant RegexIterator::get_value()
{
	if (ref_val) {
		throw error("[Reference error] Cannot take a reference to a group in a regular expression.\nHint: take the second loop variable by value, not by reference");
	}
	return re->capture(pos++);
}

bool RegexIterator::at_end() const
{
	return pos > re->count();
}


//---------------------------------------------------------------------------------------------------------------------

FileIterator::FileIterator(Variant v, bool ref_val) : Iterator(std::move(v), ref_val)
{
	file = &raw_cast<File>(object.resolve());

	if (!file->readable()) {
		throw error("[Iterator error] Cannot iterate File object: the file is not readable");
	}
}

Variant FileIterator::get_key()
{
	return pos++;
}

Variant FileIterator::get_value()
{
	if (ref_val) {
		throw error("[Reference error] Cannot take a reference to a line in a file.\nHint: take the second loop variable by value, not by reference");
	}

	return file->read_line();
}

bool FileIterator::at_end() const
{
	return file->at_end();
}
} // namespace phonometrica