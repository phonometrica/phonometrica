/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2021 Julien Eychenne <jeychenne@gmail.com>                                                       *
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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: interface for bookmarkable items.                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_BOOKMARK_HPP
#define PHONOMETRICA_BOOKMARK_HPP

#include <phon/application/annotation.hpp>

namespace phonometrica {

class Bookmark : public Element
{
public:

	Bookmark(Class *klass, Directory *parent);

	Bookmark(Directory *parent, String title);

	String label() const override;

	bool is_bookmark() const override;

	void set_notes(const String &value, bool mutate = true);

	virtual String tooltip() const { return String(); }

	virtual bool is_annotation_stamp() const { return false; }

	bool quick_search(const String &text) const override;

protected:

	String m_title;

	String m_notes;
};


//----------------------------------------------------------------------------------------------------------------------

class TimeStamp final : public Bookmark
{
public:

	TimeStamp(Directory *parent, String title, Handle<Annotation> annot, size_t layer, double start,
			  double end, String match, String left, String right);


	static void initialize(Runtime &rt);

	void to_xml(xml_node root) override;

	String tooltip() const override;

	bool is_annotation_stamp() const override { return true; }

	double start() const { return m_start; }

	double end() const { return m_end; }

	intptr_t layer() const { return m_layer; }

	Handle<Annotation> annotation() const { return m_annot; }

private:

	Handle<Annotation> m_annot;

	// Text of the match.
	String m_match;

	// KWIC context (empty for complex queries).
	String m_left, m_right;

	// Layer where the bookmark belongs (starting from 0).
	intptr_t m_layer;

	// Selection in the annotation layer.
	double m_start, m_end;
};


namespace traits {
template<> struct maybe_cyclic<Bookmark> : std::false_type { };
template<> struct maybe_cyclic<TimeStamp> : std::false_type { };
}


} // namespace phonometrica

#endif // PHONOMETRICA_BOOKMARK_HPP
