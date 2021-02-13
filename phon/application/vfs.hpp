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
 * Purpose: Virtual file system. All files displayed in the file manager and stored in a project are nodes in the      *
 * VFS. Currently implemented terminal nodes are Annotation, Document, Sound, Script, Bookmark.                        *
 * Note that some operations can be mutating or non-mutating, depending on when they are triggered. For instance, when *
 * a project is loaded, files are appended to a folder silently, but if a user explicitly adds a file to a folder,     *
 * then the folder must register the mutation. This behavior is controlled by a Boolean flag.                          *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_VFS_HPP
#define PHONOMETRICA_VFS_HPP

#include <vector>
#include <phon/application/property.hpp>
#include <phon/utils/xml.hpp>
#include <phon/utils/memory.hpp>
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


class VFolder;


class VNode : public std::enable_shared_from_this<VNode>
{
public:

	VNode(VFolder *parent);

	virtual ~VNode() = default;

	void detach(bool mutate = true);

	void set_parent(VFolder *parent, bool mutate = true);

	virtual String label() const = 0;

	virtual bool modified() const;

	virtual bool is_annotation() const;

	virtual bool is_sound() const;

	virtual bool is_bookmark() const;

	virtual bool is_script() const;

	virtual bool is_dataset() const;

	virtual bool is_concordance() const;

	virtual bool is_file() const;

	virtual bool is_folder() const;

	virtual bool is_query() const;

	virtual const char *class_name() const = 0;

	virtual void to_xml(xml_node root) = 0;

	void move_to(VFolder *parent, intptr_t pos);

	VFolder *parent() const { return m_parent; }

	virtual const VFolder *toplevel() const;

	virtual void discard_changes();

	virtual bool contains(const VNode *node) const { return false; }

	virtual bool quick_search(const String &text) const { return true; }

	static Signal<const String &, const String &, int> request_progress;

	static Signal<int> update_progress;

protected:

	// Non-owning reference to the parent.
	VFolder *m_parent = nullptr;

	bool m_content_modified = false;

};


using AutoVNode = std::shared_ptr<VNode>;
using VNodeList = Array<AutoVNode>;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class VFolder final : public VNode
{
public:

	typedef VNodeList::iterator iterator;
	typedef VNodeList::const_iterator const_iterator;
	typedef VNodeList::reverse_iterator reverse_iterator;
	typedef VNodeList::const_reverse_iterator const_reverse_iterator;

	explicit VFolder(VFolder *parent, String label);

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

	std::shared_ptr<VNode> &get(intptr_t i);
	const std::shared_ptr<VNode> &get(intptr_t i) const;

	void append(std::shared_ptr<VNode> node, bool mutate = true);

	void insert(intptr_t pos, std::shared_ptr<VNode> node);

	void remove(const std::shared_ptr<VNode> &node, bool mutate = true);

	bool modified() const override;

	bool is_folder() const override;

	const char *class_name() const override;

	void discard_changes() override;

	void to_xml(xml_node root) override;

	intptr_t size() const;

	void save_content();

	bool expanded() const;

	void set_expanded(bool value);

	void clear(bool mutate = true);

    const VFolder *toplevel() const override;

    void add_subfolder(const String &name);

	bool contains(const VNode *node) const override;

	bool quick_search(const String &text) const override;

	void sort();

protected:

    void set_modified(bool value);

	String m_label;

	VNodeList m_content;

	bool m_expanded = false;

};


using AutoVFolder = std::shared_ptr<VFolder>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class VFile : public VNode
{
public:

	VFile(VFolder *parent, String path);

	String label() const override;

	bool has_path() const;

	const String &path() const;

	virtual void set_path(String path, bool mutate);

	bool is_file() const override;

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


using VFileList = Array<std::shared_ptr<VFile>>;
using AutoVFile = std::shared_ptr<VFile>;

} // namespace phonometrica

#endif // PHONOMETRICA_VFS_HPP
