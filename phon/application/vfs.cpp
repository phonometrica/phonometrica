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
#include <phon/runtime.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Signal<const String &, const String &, int> Element::request_progress;
Signal<int> Element::update_progress;
Signal<> Document::file_modified;

Element::Element(Class *klass, Directory *parent) :
		Atomic(klass), m_parent(parent)
{

}

void Element::set_parent(Directory *parent, bool mutate)
{
	if (m_parent) m_parent->remove(Handle<Element>(this), mutate);
	m_parent = parent;
}

bool Element::modified() const
{
	return m_content_modified;
}

bool Element::is_annotation() const
{
	return false;
}

bool Element::is_sound() const
{
	return false;
}

bool Element::is_bookmark() const
{
	return false;
}

bool Element::is_script() const
{
	return false;
}

bool Element::is_document() const
{
	return false;
}

bool Element::is_folder() const
{
	return false;
}

void Element::discard_changes()
{
	m_content_modified = false;
}

void Element::detach(bool mutate)
{
	if (m_parent)
	{
		m_parent->remove(Handle<Element>(this), mutate);
		set_parent(nullptr, mutate);
	}
}

bool Element::is_dataset() const
{
	return false;
}

void Element::move_to(Directory *new_parent, intptr_t pos)
{
	new_parent->insert(pos, Handle<Element>(this));
}

const Directory *Element::toplevel() const
{
    return m_parent ? m_parent->toplevel() : nullptr;
}

bool Element::is_query() const
{
	return false;
}

bool Element::is_concordance() const
{
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Directory::Directory(Directory *parent, String label) :
		Element(meta::get_class<Directory>(), parent), m_label(std::move(label))
{

}

String Directory::label() const
{
	return m_label;
}

Directory::iterator Directory::begin() noexcept
{
	return m_content.begin();
}

Directory::const_iterator Directory::begin() const noexcept
{
	return m_content.begin();
}

Directory::iterator Directory::end() noexcept
{
	return m_content.end();
}

Directory::const_iterator Directory::end() const noexcept
{
	return m_content.end();
}

Directory::reverse_iterator Directory::rbegin() noexcept
{
	return m_content.rbegin();
}

Directory::const_reverse_iterator Directory::rbegin() const noexcept
{
	return m_content.rbegin();
}

Directory::reverse_iterator Directory::rend() noexcept
{
	return m_content.rend();
}

Directory::const_reverse_iterator Directory::rend() const noexcept
{
	return m_content.rend();
}

bool Directory::empty() const
{
	return m_content.empty();
}

Handle<Element> &Directory::get(intptr_t i)
{
	return m_content[i];
}

const Handle<Element> &Directory::get(intptr_t i) const
{
	return m_content[i];
}

void Directory::append(Handle<Element> node, bool mutate)
{
	node->set_parent(this, mutate);
	m_content.push_back(std::move(node));
	set_modified(mutate);
}

void Directory::insert(intptr_t pos, Handle<Element> node)
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

bool Directory::modified() const
{
	for (auto &f : m_content) {
		if (f->modified()) {
			return true;
		}
	}

	return m_content_modified;
}

void Directory::remove(const Handle<Element> &node, bool mutate)
{
	m_content.remove(node);
	set_modified(mutate);
}

bool Directory::is_folder() const
{
	return true;
}

void Directory::discard_changes()
{
	for (auto &f : m_content) {
		f->discard_changes();
	}

	Element::discard_changes();
}

void Directory::to_xml(xml_node root)
{
	auto node = root.append_child(class_name().data());
	auto attr = node.append_attribute("label");
	attr.set_value(label().data());

	for (auto &file : m_content) {
		file->to_xml(node);
	}
}

intptr_t Directory::size() const
{
	return m_content.size();
}

void Directory::save_content()
{
	for (auto &file : m_content)
	{
		if (file->is_document() && file->modified())
		{
			auto vf = raw_recast<Document>(file);
			assert(vf->has_path());
			vf->save();
		}
		else if (file->is_folder())
        {
		    auto folder = raw_recast<Directory>(file);
		    folder->save_content();
        }
	}

	m_content_modified = false;
}

bool Directory::expanded() const
{
	return m_expanded;
}

void Directory::set_expanded(bool value)
{
	m_expanded = value;
}

void Directory::set_label(String value)
{
	m_label = std::move(value);
	m_content_modified = true;
}

void Directory::clear(bool mutate)
{
	m_content.clear();
	m_content_modified |= mutate;
	m_expanded = false;
}

const Directory *Directory::toplevel() const
{
    auto root = this;

    while (root->parent()) {
        root = root->parent();
    }

    return root;
}

void Directory::add_subfolder(const String &name)
{
	append(make_handle<Directory>(this, name));
}

void Directory::set_modified(bool value)
{
    m_content_modified |= value;
    // Expand modified directories so that changes are easier to see in the file manager.
    m_expanded |= value;
}

bool Directory::contains(const Element *node) const
{
	for (auto &item : m_content)
	{
		if (item.get() == node) {
			return true;
		}
	}

	return false;
}

bool Directory::quick_search(const String &text) const
{
	for (auto &vnode : m_content)
	{
		if (vnode->quick_search(text)) {
			return true;
		}
	}

	return label().icontains(text);
}

void Directory::sort()
{
	auto sorter = [](const Handle<Element> &n1, const Handle<Element> &n2) { return n1->label() < n2->label(); };
	std::sort(m_content.begin(), m_content.end(), sorter);
	m_content_modified = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Document::Document(Class *klass, Directory *parent, String path) :
		Element(klass, parent), m_path(std::move(path))
{

}

String Document::label() const
{
	return m_path.empty() ? "Untitled" : filesystem::base_name(m_path);
}

const String &Document::path() const
{
	return m_path;
}

void Document::set_path(String path, bool mutate)
{
	m_path = std::move(path);
	m_metadata_modified |= mutate;
}

void Document::open()
{
	if (!m_loaded)
	{
		try
		{
			load();
			m_loaded = true;
		}
		catch (std::exception &e)
		{
			throw error("Cannot open file \"%\": %", this->path(), e.what());
		}
	}
}

void Document::save()
{
	try
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
	catch (std::exception &e)
	{
		throw error("Could not save file \"%\": %", m_path, e.what());
	}
}

bool Document::is_document() const
{
	return true;
}

void Document::add_property(Property p, bool mutate)
{
	// There can only be one property per category.
	remove_property(p.category());
	m_properties.insert(std::move(p));
	m_metadata_modified |= mutate;
}

bool Document::remove_property(const Property &p)
{
	bool erased = m_properties.erase(p) > 0;
	m_metadata_modified |= erased;

	return erased;
}

bool Document::remove_property(const String &category)
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return remove_property(p);
		}
	}

	return false;
}

String Document::get_property_value(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return p.value();
		}
	}

	return String();
}

Property Document::get_property(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return p;
		}
	}

	return Property();
}

const String &Document::description() const
{
	return m_description;
}

void Document::set_description(String value, bool mutate)
{
	m_description = std::move(value);
	m_metadata_modified |= mutate;
}

void Document::to_xml(xml_node root)
{
	auto node = root.append_child("Document");
	auto attr = node.append_attribute("class");
	attr.set_value(class_name().data());
	auto data = node.append_child(node_pcdata);
	String path = m_path;
	auto project = Project::get();
	Project::compress(path, project->directory());
	data.set_value(path.data());
}

bool Document::has_path() const
{
	return !m_path.empty();
}

const std::set<Property> &Document::properties() const
{
	return m_properties;
}

bool Document::has_category(const String &category) const
{
	for (auto &p : m_properties)
	{
		if (p.category() == category) {
			return true;
		}
	}

	return false;
}

void Document::remove_property(const String &category, const String &value)
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

bool Document::has_properties() const
{
	return !m_properties.empty();
}

void Document::save_metadata()
{
	if (uses_external_metadata())
	{
		auto project = Project::get();
		auto &db = project->database();
		db.save_file_metadata(*this);
	}
}

bool Document::loaded() const
{
	return m_loaded;
}

bool Document::modified() const
{
	return m_metadata_modified || Element::modified();
}

bool Document::uses_external_metadata() const
{
	return true;
}

Array<String> Document::property_list() const
{
    Array<String> result;

    for (auto &p : m_properties)
    {
        result.append(p.to_string());
    }

    return result;
}

bool Document::content_modified() const
{
	return m_content_modified;
}

void Document::metadata_to_xml(xml_node meta_node)
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

void Document::metadata_from_xml(xml_node meta_node)
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

Property Document::parse_property(xml_node prop_node, const std::type_info &type)
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

void Document::reload()
{
	discard_changes();
	load();
}

bool Document::quick_search(const String &text) const
{
	for (auto &prop : m_properties)
	{
		if (prop.value().icontains(text)) {
			return true;
		}
	}

	return label().icontains(text) ||  m_description.icontains(text);
}

bool Document::anchored() const
{
	return has_path();
}

void Document::initialize(Runtime &rt)
{
	auto add_property = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &doc = cast<Document>(args[0]);
		auto &category = cast<String>(args[1]);
		std::any value;

		if (check_type<String>(args[2])) {
			value = cast<String>(args[2]);
		}
		else if (check_type<bool>(args[2])) {
			value = cast<bool>(args[2]);
		}
		else if (args[2].resolve().is_number()) {
			value = args[2].resolve().get_number();
		}
		else {
			throw error("Invalid property type: %", args[2].class_name());
		}
		doc.add_property(Property(category, std::move(value)));

		return Variant();
	};

	auto remove_property = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &doc = cast<Document>(args[0]);
		auto &category = cast<String>(args[1]);
		doc.remove_property(category);
		return Variant();
	};

	auto get_property = [](Runtime &, std::span<Variant> args) -> Variant  {
		auto &doc = cast<Document>(args[0]);
		auto category = cast<String>(args[1]);
		auto prop = doc.get_property(category);

		if (prop.valid())
		{
			if (prop.is_text())
				return prop.value();
			else if (prop.is_numeric())
				return prop.numeric_value();
			else if (prop.is_boolean())
				return prop.boolean_value();
			else
				throw error("[Internal error] Invalid property type");
		}

		return Variant();
	};

#define CLS(T) phonometrica::get_class<T>()
	rt.add_global("add_property", add_property, { CLS(Document), CLS(String), CLS(Object) });
	rt.add_global("remove_property", remove_property, { CLS(Document), CLS(String) });
	rt.add_global("get_property", get_property, { CLS(Document), CLS(String) });
#undef CLS
}


} // namespace phonometrica
