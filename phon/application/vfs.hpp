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
 * Purpose: Element (formerly VNode) is the base class for all the elements in a project's virtual file system. All    *
 * files displayed in the file manager and stored in a project are subclasses of this class.                           *
 * Note that some operations can be mutating or non-mutating, depending on where they are triggered. For instance,     *
 * when a project is loaded, files are appended to a folder silently, but if a user explicitly adds a file to a folder *
 * then the folder must register the mutation. This behavior is controlled by a Boolean flag.                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VFS_HPP
#define PHONOMETRICA_VFS_HPP

#include <vector>
#include <phon/runtime/typed_object.hpp>
#include <phon/runtime/class.hpp>
#include <phon/application/property.hpp>
#include <phon/utils/xml.hpp>
#include <phon/utils/signal.hpp>

namespace phonometrica {

enum class FileType
{
	Annotation = 1,
	Sound = 2,
	Query = 4,
	Script = 8,
	Dataset = 16,
	CorpusFile = Annotation|Sound,
	Any = Annotation|Sound|Query|Script|Dataset
};


class Directory;


class Element : public Atomic
{
public:

	Element(Class *klass, Directory *parent);

	virtual ~Element() = default;

	void detach(bool mutate = true);

	void set_parent(Directory *parent, bool mutate = true);

	virtual String label() const = 0;

	virtual bool modified() const;

	template<typename T>
	bool is() const { return dynamic_cast<const T*>(this) != nullptr; }

	virtual void to_xml(xml_node root) = 0;

	void move_to(Directory *parent, intptr_t pos);

	Directory *parent() const { return m_parent; }

	virtual const Directory *toplevel() const;

	virtual void discard_changes();

	virtual bool contains(const Element *node) const { return false; }

	virtual bool quick_search(const String &text) const { return true; }

	static Signal<const String &, const String &, int> request_progress;

	static Signal<int> update_progress;

protected:

	// Non-owning reference to the parent.
	Directory *m_parent = nullptr;

	bool m_content_modified = false;

};

using ElementList = Array<Handle<Element>>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Directory final : public Element
{
public:

	typedef ElementList::iterator iterator;
	typedef ElementList::const_iterator const_iterator;
	typedef ElementList::reverse_iterator reverse_iterator;
	typedef ElementList::const_reverse_iterator const_reverse_iterator;

	Directory(Directory *parent, String label);

	iterator begin() noexcept;
	const_iterator begin() const noexcept;

	iterator end() noexcept;
	const_iterator end() const noexcept;

	reverse_iterator rbegin() noexcept;
	const_reverse_iterator rbegin() const noexcept;

	reverse_iterator rend() noexcept;
	const_reverse_iterator rend() const noexcept;

	String label() const override;

	void set_label(String value);

	bool empty() const;

	Handle<Element> &get(intptr_t i);
	const Handle<Element> &get(intptr_t i) const;

	void append(Handle<Element> node, bool mutate = true);

	void insert(intptr_t pos, Handle<Element> node);

	void remove(const Handle<Element> &node, bool mutate = true);

	bool modified() const override;

	void discard_changes() override;

	void to_xml(xml_node root) override;

	intptr_t size() const;

	void save_content();

	bool expanded() const;

	void set_expanded(bool value);

	void clear(bool mutate = true);

    const Directory *toplevel() const override;

    void add_subfolder(const String &name);

	bool contains(const Element *node) const override;

	bool quick_search(const String &text) const override;

	void sort();

protected:

    void set_modified(bool value);

	String m_label;

	ElementList m_content;

	bool m_expanded = false;

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Document : public Element
{
public:

	Document(Class *klass, Directory *parent, String path);

	String label() const override;

	bool has_path() const;

	const String &path() const;

	virtual void set_path(String path, bool mutate);

	void open();

	void save();

	void reload();

	bool loaded() const;

	bool modified() const override;

	void add_property(Property p, bool mutate = true);

	bool remove_property(const Property &p);

	bool remove_property(const String &category);

	String get_property_value(const String &category) const;

	Property get_property(const String &category) const;

	const String &description() const;

	void set_description(String value, bool mutate = true);

	void to_xml(xml_node root) final;

	const std::set<Property> &properties() const;

	bool has_category(const String &category) const;

	void remove_property(const String &category, const String &value);

	bool has_properties() const;

	Array<String> property_list() const;

	bool quick_search(const String &text) const override;

	bool anchored() const;

	static void initialize(Runtime &rt);

	// Send this signal to notify the current view that the file has been modified
	static Signal<> file_modified;

protected:

	virtual void load() = 0;

	virtual void write() = 0;

	virtual void save_metadata();

	virtual bool uses_external_metadata() const;

	virtual bool content_modified() const;

	virtual void metadata_to_xml(xml_node meta_node);

	virtual void metadata_from_xml(xml_node meta_node);

	Property parse_property(xml_node prop_node, const std::type_info &type);

	String m_path;

	std::set<Property> m_properties;

	String m_description;

	bool m_loaded = false;

	bool m_metadata_modified = false;
};


using DocList = Array<Handle<Document>>;


namespace traits {
template<> struct maybe_cyclic<Element> : std::false_type { };
template<> struct maybe_cyclic<Directory> : std::false_type { };
template<> struct maybe_cyclic<Document> : std::false_type { };
}


} // namespace phonometrica

#endif // PHONOMETRICA_VFS_HPP
