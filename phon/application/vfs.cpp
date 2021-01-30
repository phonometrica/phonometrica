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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/vfs.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/file_system.hpp>
#include "vfs.hpp"


namespace phonometrica {

VNode::VNode(VFolder *parent) :
		m_parent(parent)
{

}

void VNode::set_parent(VFolder *parent, bool mutate)
{
	if (m_parent) m_parent->remove(this->shared_from_this(), mutate);
	m_parent = parent;
}

bool VNode::modified() const
{
	return m_content_modified;
}

bool VNode::is_annotation() const
{
	return false;
}

bool VNode::is_sound() const
{
	return false;
}

bool VNode::is_document() const
{
	return false;
}

bool VNode::is_bookmark() const
{
	return false;
}

bool VNode::is_script() const
{
	return false;
}

bool VNode::is_file() const
{
	return false;
}

bool VNode::is_folder() const
{
	return false;
}

void VNode::discard_changes()
{
	m_content_modified = false;
}

void VNode::detach()
{
	if (m_parent)
	{
		m_parent->remove(this->shared_from_this(), true);
		set_parent(nullptr);
	}
}

bool VNode::is_dataset() const
{
	return false;
}

void VNode::move_to(VFolder *new_parent, intptr_t pos)
{
	new_parent->insert(pos, shared_from_this());
}

const VFolder *VNode::toplevel() const
{
    return m_parent ? m_parent->toplevel() : nullptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VFolder::VFolder(VFolder *parent, String label) :
		VNode(parent), m_label(std::move(label))
{

}

String VFolder::label() const
{
	return m_label;
}

VFolder::iterator VFolder::begin() noexcept
{
	return m_content.begin();
}

VFolder::const_iterator VFolder::begin() const noexcept
{
	return m_content.begin();
}

VFolder::iterator VFolder::end() noexcept
{
	return m_content.end();
}

VFolder::const_iterator VFolder::end() const noexcept
{
	return m_content.end();
}

VFolder::reverse_iterator VFolder::rbegin() noexcept
{
	return m_content.rbegin();
}

VFolder::const_reverse_iterator VFolder::rbegin() const noexcept
{
	return m_content.rbegin();
}

VFolder::reverse_iterator VFolder::rend() noexcept
{
	return m_content.rend();
}

VFolder::const_reverse_iterator VFolder::rend() const noexcept
{
	return m_content.rend();
}

bool VFolder::empty() const
{
	return m_content.empty();
}

std::shared_ptr<VNode> &VFolder::get(intptr_t i)
{
	return m_content[i];
}

const std::shared_ptr<VNode> &VFolder::get(intptr_t i) const
{
	return m_content[i];
}

void VFolder::append(std::shared_ptr<VNode> node, bool mutate)
{
	node->set_parent(this, mutate);
	m_content.push_back(std::move(node));
	set_modified(mutate);
}

void VFolder::insert(intptr_t pos, std::shared_ptr<VNode> node)
{
	node->set_parent(this);
	if (pos < 0) {
		m_content.append(std::move(node));
	}
	else {
		m_content.insert(pos, std::move(node));
	}
	// insertion is always triggered by the user, so it always mutates the folder.
	set_modified(true);
}

bool VFolder::modified() const
{
	for (auto &f : m_content) {
		if (f->modified()) {
			return true;
		}
	}

	return m_content_modified;
}

void VFolder::remove(const std::shared_ptr<VNode> &node, bool mutate)
{
	m_content.remove(node);
	set_modified(mutate);
}

bool VFolder::is_folder() const
{
	return true;
}

const char *VFolder::class_name() const
{
	return "VFolder";
}

void VFolder::discard_changes()
{
	for (auto &f : m_content) {
		f->discard_changes();
	}

	VNode::discard_changes();
}

void VFolder::to_xml(xml_node root)
{
	auto node = root.append_child(class_name());
	auto attr = node.append_attribute("label");
	attr.set_value(label().data());

	for (auto &file : m_content) {
		file->to_xml(node);
	}
}

intptr_t VFolder::size() const
{
	return m_content.size();
}

void VFolder::save_content()
{
	for (auto &file : m_content)
	{
		if (file->is_file() && file->modified())
		{
			auto vf = raw_cast<VFile>(file);
			assert(vf->has_path());
			vf->save();
		}
		else if (file->is_folder())
        {
		    auto folder = raw_cast<VFolder>(file);
		    folder->save_content();
        }
	}

	m_content_modified = false;
}

bool VFolder::expanded() const
{
	return m_expanded;
}

void VFolder::set_expanded(bool value)
{
	m_expanded = value;
}

void VFolder::set_label(String value)
{
	m_label = std::move(value);
	m_content_modified = true;
}

void VFolder::clear(bool mutate)
{
	m_content.clear();
	m_content_modified |= mutate;
	m_expanded = false;
}

const VFolder *VFolder::toplevel() const
{
    auto root = this;

    while (root->parent()) {
        root = root->parent();
    }

    return root;
}

void VFolder::add_subfolder(const String &name)
{
	append(std::make_shared<VFolder>(this, name));
}

void VFolder::set_modified(bool value)
{
    m_content_modified |= value;
    // Expand modified directories so that changes are easier to see in the file manager.
    m_expanded |= value;
}

bool VFolder::contains(const VNode *node) const
{
	for (auto &item : m_content)
	{
		if (item.get() == node) {
			return true;
		}
	}

	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VFile::VFile(VFolder *parent, String path) :
		VNode(parent), m_path(std::move(path))
{

}

String VFile::label() const
{
	return m_path.empty() ? "Untitled" : filesystem::base_name(m_path);
}

const String &VFile::path() const
{
	return m_path;
}

void VFile::set_path(String path, bool mutate)
{
	m_path = std::move(path);
	m_metadata_modified |= mutate;
}

void VFile::open()
{
	if (!m_loaded)
	{
		try {
			load();
		}
		catch (std::exception &e) {
			throw error("Cannot open file \"%\": %", this->path(), e.what());
		}
	}

	m_loaded = true;
}

void VFile::save()
{
	if (!modified()) return;

	save_metadata();

	if (content_modified())
	{
		assert(!m_path.empty());
		write();
	}

	m_content_modified = false;
	m_metadata_modified = false;
}

bool VFile::is_file() const
{
	return true;
}

void VFile::add_property(Property p, bool mutate)
{
	// There can only be one property per category.
	remove_property(p.category());
	m_properties.insert(std::move(p));
	m_metadata_modified |= mutate;
}

bool VFile::remove_property(const Property &p)
{
	bool erased = m_properties.erase(p) > 0;
	m_metadata_modified |= erased;

	return erased;
}

bool VFile::remove_property(const String &category)
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return remove_property(p);
		}
	}

	return false;
}

String VFile::get_property_value(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return p.value();
		}
	}

	return String();
}

Property VFile::get_property(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return p;
		}
	}

	return Property();
}

const String &VFile::description() const
{
	return m_description;
}

void VFile::set_description(String value, bool mutate)
{
	m_description = std::move(value);
	m_metadata_modified |= mutate;
}

void VFile::to_xml(xml_node root)
{
	auto node = root.append_child("VFile");
	auto attr = node.append_attribute("class");
	attr.set_value(class_name());
	auto data = node.append_child(node_pcdata);
	String path = m_path;
	auto project = Project::get();
	Project::compress(path, project->directory());
	data.set_value(path.data());
}

bool VFile::has_path() const
{
	return !m_path.empty();
}

const std::set<Property> &VFile::properties() const
{
	return m_properties;
}

bool VFile::has_category(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return true;
		}
	}

	return false;
}

void VFile::remove_property(const String &category, const String &value)
{
	for (auto it = m_properties.begin(); it != m_properties.end(); it++)
	{
		if (it->category() == category && it->value() == value)
		{
			m_properties.erase(it);
			m_metadata_modified = true;

			return;
		}
	}
}

bool VFile::has_properties() const
{
	return !m_properties.empty();
}

void VFile::save_metadata()
{
	if (uses_external_metadata())
	{
		auto project = Project::get();
		auto &db = project->database();
		db.save_file_metadata(*this);
	}
}

bool VFile::loaded() const
{
	return m_loaded;
}

bool VFile::modified() const
{
	return  m_metadata_modified || VNode::modified();
}

bool VFile::uses_external_metadata() const
{
	return true;
}

Array<String> VFile::property_list() const
{
    Array<String> result;

    for (auto &p : m_properties)
    {
        result.append(p.to_string());
    }

    return result;
}

bool VFile::content_modified() const
{
	return m_content_modified;
}

void VFile::metadata_to_xml(xml_node meta_node)
{
	// The root node is "Metadata" so that subclasses can add additional metadata.
	add_data_node(meta_node, "Description", description());
	auto properties_node = meta_node.append_child("Properties");

	for (auto &prop : properties())
	{
		auto prop_node = properties_node.append_child("Property");
		auto attr = prop_node.append_attribute("type");
		attr.set_value(prop.xml_type_name());
		add_data_node(prop_node, "Category", prop.category());
		add_data_node(prop_node, "Value", prop.value());
	}
}

void VFile::metadata_from_xml(xml_node meta_node)
{
	static std::string_view desc_tag = "Description";
	static std::string_view properties_tag = "Properties";
	static std::string_view property_tag = "Property";

	for (auto node = meta_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == desc_tag)
		{
			set_description(node.text().get(), false);
		}
		else if (node.name() == properties_tag)
		{
			for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
			{
				if (subnode.name() == property_tag)
				{
					auto attr = subnode.attribute("type");
					auto &type = Property::parse_xml_type_name(attr.value());
					auto prop = parse_property(subnode, type);
					add_property(std::move(prop), false);
				}
			}
		}
	}
}

Property VFile::parse_property(xml_node prop_node, const std::type_info &type)
{
	static std::string_view category_tag = "Category";
	static std::string_view value_tag = "Value";
	String category;

	for (auto node = prop_node.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == category_tag)
		{
			category = node.text().get();
		}
		else if (node.name() == value_tag)
		{
			if (category.empty()) {
				throw error("Empty property category in annotation file");
			}

			String value = node.text().get();

			if (type == typeid(String))
			{
				return Property(category, value);
			}
			if (type == typeid(double))
			{
				return Property(category, value.to_float());
			}
			assert(type == typeid(bool));

			return Property(category, value.to_bool(true));
		}
	}

	throw error("Invalid property node in annotation file");
}

void VFile::reload()
{
	discard_changes();
	load();
}

} // namespace phonometrica
