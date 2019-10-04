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
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: interface for bookmarkable items.                                                                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_BOOKMARK_HPP
#define PHONOMETRICA_BOOKMARK_HPP

#include <phon/application/annotation.hpp>

namespace phonometrica {

class Bookmark : public VNode
{
public:

	Bookmark(VFolder *parent);

	Bookmark(VFolder *parent, String title);

	String label() const override;

	bool is_bookmark() const override;

	void set_notes(const String &value, bool mutate = true);

	virtual String tooltip() const { return String(); }

	virtual bool is_annotation_stamp() const { return false; }

protected:

	String m_title;

	String m_notes;
};

using AutoBookmark = std::shared_ptr<Bookmark>;

//----------------------------------------------------------------------------------------------------------------------

class AnnotationStamp final : public Bookmark
{
public:

	AnnotationStamp(VFolder *parent, String title, AutoAnnotation annot, size_t layer, double start,
					double end, String match, String left, String right);


	const char *class_name() const override;

	void to_xml(xml_node root) override;

	String tooltip() const override;

	bool is_annotation_stamp() const override { return true; }

	double start() const { return m_start; }

	double end() const { return m_end; }

	intptr_t layer() const { return m_layer; }

	AutoAnnotation annotation() const { return m_annot; }

private:

	AutoAnnotation m_annot;

	// Text of the match.
	String m_match;

	// KWIC context (empty for complex queries).
	String m_left, m_right;

	// Layer where the bookmark belongs (starting from 0).
	intptr_t m_layer;

	// Selection in the annotation layer.
	double m_start, m_end;
};

} // namespace phonometrica

#endif // PHONOMETRICA_BOOKMARK_HPP
