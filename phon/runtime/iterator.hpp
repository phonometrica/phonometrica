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
 * Created: 22/05/2020                                                                                                 *
 *                                                                                                                     *
 * Purpose: Iterator object, to iterate over sequences such as List and Table.                                         *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ITERATOR_HPP
#define PHONOMETRICA_ITERATOR_HPP

#include <phon/runtime/list.hpp>
#include <phon/runtime/table.hpp>

namespace phonometrica {

// We can't make this an abstract interface because we need to store iterators in handles, and Handle<T> would complain that it can't
// instantiate an abstract class.
class Iterator
{
public:

	Iterator(Variant v, bool ref_val) : object(std::move(v)), ref_val(ref_val) { }

	virtual ~Iterator() = default;

	virtual Variant get_key() { return Variant(); }

	virtual Variant get_value();

	virtual bool at_end() const { return true; };

protected:

	Variant object;
	bool ref_val;
};

//---------------------------------------------------------------------------------------------------------------------

class ListIterator : public Iterator
{
public:

	ListIterator(Variant v, bool ref_val);

	Variant get_key() override;

	Variant get_value() override;

	bool at_end() const override;

private:

	List::Storage *lst;
	intptr_t pos;
};

//---------------------------------------------------------------------------------------------------------------------

class TableIterator : public Iterator
{
public:

	TableIterator(Variant v, bool ref_val);

	Variant get_key() override;

	Variant get_value() override;

	bool at_end() const override;

private:

	Table::Storage *map;
	Table::iterator it;
};

//---------------------------------------------------------------------------------------------------------------------

class StringIterator : public Iterator
{
public:

	StringIterator(Variant v, bool ref_val);

	Variant get_key() override;

	Variant get_value() override;

	bool at_end() const override;

private:

	String *str;
	intptr_t pos = 1;
};


//---------------------------------------------------------------------------------------------------------------------

class RegexIterator : public Iterator
{
public:

	RegexIterator(Variant v, bool ref_val);

	Variant get_key() override;

	Variant get_value() override;

	bool at_end() const override;

private:

	Regex *re;
	intptr_t pos = 1;
};


//---------------------------------------------------------------------------------------------------------------------

class FileIterator : public Iterator
{
public:

	FileIterator(Variant v, bool ref_val);

	Variant get_key() override;

	Variant get_value() override;

	bool at_end() const override;

private:

	File *file;
	intptr_t pos = 1;
};

} // namespace phonometrica

#endif // PHONOMETRICA_ITERATOR_HPP
