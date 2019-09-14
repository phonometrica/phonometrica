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
 * Created: 28/02/2019                                                                                                *
 *                                                                                                                    *
 * Purpose: interface for bookmarkable items.                                                                         *
 *                                                                                                                    *
 **********************************************************************************************************************/

#ifndef PHONOMETRICA_BOOKMARK_HPP
#define PHONOMETRICA_BOOKMARK_HPP

#include <phon/application/vfs.hpp>

namespace phonometrica {

class Bookmark : public VNode
{
public:

	Bookmark(VFolder *parent);

	Bookmark(VFolder *parent, String title);

	String label() const override;

	bool is_bookmark() const override;

protected:

	String m_title;

	String m_notes;
};

using AutoBookmark = std::shared_ptr<Bookmark>;

//----------------------------------------------------------------------------------------------------------------------

class AnnotationStamp final : public Bookmark
{
public:

	AnnotationStamp(VFolder *parent, String title, std::shared_ptr<VFile> file, size_t layer, double start,
					double end, String match,
					String left = String(), String right = String());


	const char *class_name() const override;

	void to_xml(xml_node root) override;

private:

	std::shared_ptr<VFile> m_file;

	// Text of the match.
	String m_match;

	// KWIC context (empty for complex queries).
	String m_left, m_right;

	// Layer where the bookmark belongs (starting from 0).
	size_t m_layer;

	// Selection in the annotation layer.
	double m_start, m_end;
};

} // namespace phonometrica

#endif // PHONOMETRICA_BOOKMARK_HPP
