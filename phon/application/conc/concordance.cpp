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
 * Created: 08/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/conc/concordance.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/xml.hpp>

namespace phonometrica {

// file, layer, start time, end time
static const int FILE_INFO_COLUMN_COUNT = 4;


Concordance::Concordance(Directory *parent, const String &path) :
		DataTable(meta::get_class<Concordance>(), parent, path)
{
	preload();
}

Concordance::Concordance(intptr_t target_count, Context ctx, intptr_t context_length, Array <AutoMatch> matches, Directory *parent, const String &path) :
		DataTable(meta::get_class<Concordance>(), parent, path), m_matches(std::move(matches))
{
	m_target_count = (int) target_count;
	m_context_type = ctx;
	m_context_length = (int) context_length;
	m_context.reserve(m_matches.size());
	find_context();
	m_loaded = true;
}

Concordance::Concordance(const Concordance &other) :
		DataTable(other.klass, other.parent(), String())
{
	m_target_count = other.m_target_count;
	m_context_type = other.m_context_type;
	m_context_length = other.m_context_length;

	m_matches.reserve(other.m_matches.size());

	for (auto &m : other.m_matches) {
		m_matches.append(std::make_unique<Match>(*m));
	}
	m_content_modified = true;
}

bool Concordance::empty() const
{
	return m_matches.empty();
}

String Concordance::get_header(intptr_t j) const
{
	// The logic of this function is the same as that of get_cell(). See comments there.
	if (j == 1) {
		return "File";
	}
	else if (j == 2) {
		return "Layer";
	}
	else if (j == 3) {
		return "Start time";
	}
	else if (j == 4) {
		return "End time";
	}
	else if (j == 5 && has_context()) {
		return "Left context";
	}

	j -= FILE_INFO_COLUMN_COUNT;
	if (has_context()) j--;

	// We are now ready to consume the match: j starts at 1.
	if (j <= m_target_count)
	{
		if (m_target_count == 1) {
			return "Target";
		}
		else {
			return String::format("Target %d", (int) j);
		}
	}

	j -= m_target_count;
	if (has_context())
	{
		if (j == 1) {
			return "Right context";
		}
		j--;
	}

	// We are now ready to consume the properties. Switch to base 0 because
	// we'll use an iterator.
	j--;

	if (j < Property::category_count())
	{
		auto it = Property::get_categories().begin();
		std::advance(it, j);
		return *it;
	}
	assert(j == Property::category_count());

	return "Description";
}

String Concordance::get_cell(intptr_t i, intptr_t j) const
{
	// First handle information columns: these are fixed.
	if (j == 1) {
		return m_matches[i]->annotation()->label();
	}
	else if (j == 2) {
		return String::convert(m_matches[i]->get_layer(1));
	}
	else if (j == 3) {
		return String::format("%.4f", m_matches[i]->get_event(1)->start_time());
	}
	else if (j == 4) {
		return String::format("%.4f", m_matches[i]->get_event(1)->end_time());
	}
	else if (j == 5 && has_context()) {
		return get_left_context(i);
	}

	// At this point, j == 5 if we have no context or 6 if we have one because we consumed the left context.
	j -= FILE_INFO_COLUMN_COUNT;
	if (has_context()) j--;

	// We are now ready to consume the match: j starts at 1.
	if (j <= m_target_count) {
		return m_matches[i]->get_value(j);
	}

	// We now consume the right context if we have one
	j -= m_target_count;
	if (has_context())
	{
		if (j == 1) {
			return get_right_context(i);
		}
		j--;
	}

	// We are now ready to consume the properties. Switch to base 0 because
	// we'll use an iterator.
	j--;

	if (j < Property::category_count())
	{
		auto it = Property::get_categories().begin();
		std::advance(it, j);
		return m_matches[i]->annotation()->get_property_value(*it);
	}

	// We now reach the description
	assert(j == Property::category_count());

	return m_matches[i]->annotation()->description();
}

void Concordance::set_cell(intptr_t i, intptr_t j, const String &value)
{
	throw error("Cannot write cell value in concordance");
}

bool Concordance::is_left_context(intptr_t col) const
{
	// TODO: adjust context position for complex queries
	return has_context() && col == 5;
}

bool Concordance::is_right_context(intptr_t col) const
{
	return has_context() && col == 7;
}

bool Concordance::is_time(intptr_t col) const
{
	return col == 3 || col == 4;
}

intptr_t Concordance::row_count() const
{
	return m_matches.size();
}

intptr_t Concordance::column_count() const
{
	// Add 1 for description.
	return FILE_INFO_COLUMN_COUNT + context_column_count() + m_target_count + Property::category_count() + 1;
}


void Concordance::preload()
{
	xml_document doc;
	xml_node root;
	using str = std::string_view;

	try
	{
		root = read_xml(doc, m_path);
	}
	catch (...)
	{
		throw error("Cannot open text query \"%\"", m_path);
	}

	if (root.name() != str("Phonometrica")) {
		throw error("Invalid XML project root in %", m_path);
	}

	auto attr = root.attribute("class");

	if (!attr || attr.as_string() != class_name()) {
		throw error("Expected a concordance, got a % file instead", attr.as_string());
	}
	attr = root.attribute("label");
	if (attr) {
		set_label(attr.value(), false);
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Metadata"))
		{
			metadata_from_xml(node);
		}
	}
}


void Concordance::load()
{
	xml_document doc;
	xml_node root;
	using str = std::string_view;

	try
	{
		root = read_xml(doc, m_path);
	}
	catch (...)
	{
		throw error("Cannot open text query \"%\"", m_path);
	}

	if (root.name() != str("Phonometrica")) {
		throw error("Invalid XML project root in %", m_path);
	}

	auto attr = root.attribute("class");

	if (!attr || attr.as_string() != class_name()) {
		throw error("Expected a concordance, got a % file instead", attr.as_string());
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Options"))
		{
			parse_options_from_xml(node);
		}
		else if (node.name() == str("Matches"))
		{
			parse_matches_from_xml(node);
		}
	}

	find_context();
}

void Concordance::parse_options_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Context"))
		{
			auto attr = node.attribute("type");

			if (attr.value() == str("labels"))
			{
				m_context_type = Context::Labels;
			}
			else if (attr.value() == str("kwic"))
			{
				m_context_type = Context::KWIC;
				attr = node.attribute("length");
				if (!attr) {
					throw error("KWIC context in concordance requires a length attribute");
				}
				m_context_length = attr.as_int();
				if (m_context_length < 1) {
					throw error("Invalid context length in KWIC concordance: %", m_context_length);
				}
			}
			else
			{
				m_context_type = Context::None;
			}
		}
		else
		{
			throw error("Invalid option for concordance: %", node.name());
		}
	}
}

void Concordance::parse_matches_from_xml(xml_node root)
{
	using str = std::string_view;

#ifdef PHON_TIMING
	auto first_time = clock();
#endif

	auto attr = root.attribute("count");
	if (!attr){
		throw error("Matches node has no 'count' attribute");
	}
	int size = attr.as_int();
	if (size > 0) m_matches.reserve(size);
	auto msg = String("Opening concordance %1").arg(label());
	request_progress(msg, "Loading matches...", size);

	attr = root.attribute("length");
	if (!attr) {
		throw error("Matches node has no 'length' attribute");
	}
	m_target_count = attr.as_int();
	if (m_target_count < 1) {
		throw error("Invalid length in Match node");
	}
	int count = 1;
	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() != str("Match")) {
			throw error("Expected a Match, got a % in concordance", node.name());
		}
		update_progress(count++);

		Handle<Annotation> annot;
		std::unique_ptr<Match::Target> first_target;
		Match::Target *last_target = nullptr;
		String path;

		for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
		{
			if (subnode.name() == str("Annotation"))
			{
				path = subnode.text().get();
				annot = recast<Annotation>(Project::get()->get(path));
			}
			else if (subnode.name() == str("Targets"))
			{
				if (!annot) {
					throw error("A match was found in file '%' but this file is no longer in the current project", path);
				}
				annot->open();

				for (auto target_node = subnode.first_child(); target_node; target_node = target_node.next_sibling())
				{
					attr = target_node.attribute("layer");
					if (!attr) {
						throw error("Missing 'layer' attribute in concordance target");
					}
					int layer = attr.as_int();
					attr = target_node.attribute("event");
					if (layer < 1 || layer > annot->size()) {
						throw error("Invalid layer index (%) in match", layer);
					}
					if (!attr) {
						throw error("Missing 'event' attribute in concordance target");
					}
					int index = attr.as_int();
					auto &events = annot->get_layer_events(layer);
					if (index < 1 || index > events.size()) {
						throw error("Invalid event index (%) in layer with % events", index, events.size());
					}
					auto event = events[index];
					attr = target_node.attribute("offset");
					if (!attr) {
						throw error("Missing 'offset' attribute in concordance target");
					}
					int offset = attr.as_int();
					attr = target_node.attribute("ref");
					if (!attr) {
						throw error("Missing 'ref' attribute in concordance target");
					}
					bool is_ref = attr.as_bool();
					String value = target_node.text().get();

					if (last_target)
					{
						last_target->next = std::make_unique<Match::Target>(event, value, layer, offset, is_ref);
						last_target = last_target->next.get();
					}
					else
					{
						first_target = std::make_unique<Match::Target>(event, value, layer, offset, is_ref);
						last_target = first_target.get();
					}
				}
			}
			else
			{
				throw error("Invalid node in Match", subnode.name());
			}
		}

		assert(first_target);
		m_matches.append(std::make_unique<Match>(annot, std::move(first_target)));
	}

#ifdef PHON_TIMING
	auto last_time = clock();
	auto total = double(last_time-first_time) * 1000 / CLOCKS_PER_SEC;
	std::cerr << "Total loading time concordance: " << total << " ms\n";
#endif
}

void Concordance::write()
{
	open();
	xml_document doc;

	auto root = doc.append_child("Phonometrica");
	root.append_attribute("class").set_value(class_name().data());
	root.append_attribute("label").set_value(m_label.data());
	root.append_attribute("type").set_value("text");
	auto metadata_node = root.append_child("Metadata");
	metadata_to_xml(metadata_node);

	auto option_node = root.append_child("Options");
	auto ctx_node = option_node.append_child("Context");
	auto type_attr = ctx_node.append_attribute("type");

	switch (m_context_type)
	{
		case Context::Labels:
		{
			type_attr.set_value("labels");
		} break;
		case Context::KWIC:
		{
			type_attr.set_value("kwic");
			ctx_node.append_attribute("length").set_value(m_context_length);
		} break;
		default:
			type_attr.set_value("none");
	}

	auto matches_node = root.append_child("Matches");
	matches_node.append_attribute("count").set_value(m_matches.size());
	matches_node.append_attribute("length").set_value(m_target_count);
	auto msg = String("Writing concordance %1").arg(label());
	request_progress(msg, "Writing matches...", (int)m_matches.size());
	int count = 1;
	for (auto &match : m_matches)
	{
		update_progress(count++);
		match->to_xml(matches_node);
	}

	write_xml(doc, m_path);
}

bool Concordance::has_context() const
{
	return m_context_type != Context::None;
}

void Concordance::find_context()
{
	m_context.clear();

	switch (m_context_type)
	{
		case Context::Labels:
			find_label_context();
			break;
		case Context::KWIC:
			find_kwic_context();
			break;
		default:
			break;
	}
}

void Concordance::find_kwic_context()
{
	String sep(" ");
	// FIXME: the progress dialog slows things down absurdly on macOS.
	//auto msg = String("Extracting KWIC context for concordance %1").arg(label());
	//request_progress(msg, "Loading matches", (int)m_matches.size());
	//int count = 1;

	for (auto &match : m_matches)
	{
		//update_progress(count++);
		auto target = match->reference_target();
		auto annot = match->annotation().get();
		std::pair<String, String> ctx;
		if (target)
		{
			auto i = annot->get_event_index(target->layer, target->start_time());
			assert(i != 0);
			auto offset = target->offset;
			ctx.first = annot->left_context(target->layer, i, offset, m_context_length, sep);
			offset += target->value.size();
			ctx.second = annot->right_context(target->layer, i, offset, m_context_length, sep);
		}
		m_context.append(std::move(ctx));
	}
}

bool Concordance::is_file_info_column(intptr_t col) const
{
	return col <= FILE_INFO_COLUMN_COUNT;
}

bool Concordance::is_metadata_column(intptr_t col) const
{
	intptr_t bound = FILE_INFO_COLUMN_COUNT + m_target_count + context_column_count();
	return col > bound;
}

void Concordance::find_label_context()
{
//	auto msg = String("Extracting surrounding labels for concordance %1").arg(label());
	//request_progress(msg, "Loading matches", (int)m_matches.size());
//	int count = 1;

	for (auto &match : m_matches)
	{
		//update_progress(count++);
		auto target = match->reference_target();
		std::pair<String, String> ctx;
		if (target)
		{
			auto &annot = *match->annotation();
			auto &events = annot.get_layer_events(target->layer);
			auto i = annot.get_event_index(target->layer, target->start_time());
			assert(i != 0);
			ctx.first = (i == 1) ? String() : events[i-1]->text();
			ctx.second = (i == events.size()) ? String() : events[i+1]->text();
		}
		m_context.append(std::move(ctx));
	}
}

String Concordance::get_left_context(intptr_t i) const
{
	return has_context() ? m_context[i].first : String();
}

String Concordance::get_right_context(intptr_t i) const
{
	return has_context() ? m_context[i].second : String();
}

bool Concordance::is_target(intptr_t col) const
{
	intptr_t lower = FILE_INFO_COLUMN_COUNT + int(has_context()); // add 1 column for the left context
	intptr_t upper = lower + m_target_count;

	return col > lower && col <= upper;
}

Match &Concordance::get_match(intptr_t i)
{
	return *m_matches[i];
}

int Concordance::match_region_size() const
{
	return m_target_count + context_column_count();
}

int Concordance::context_column_count() const
{
	return has_context() ? 2 : 0;
}

String Concordance::label() const
{
	return m_label.empty() ? Document::label() : m_label;
}

void Concordance::set_label(String value, bool mutate)
{
	m_label = std::move(value);
	if (mutate) m_content_modified = true;
}

void Concordance::modify()
{
	m_content_modified = true;
}

AutoMatch Concordance::remove_match(intptr_t row)
{
	auto m = m_matches.at(row).release();
	m_matches.remove_at(row);
	modify();
	file_modified();

	return AutoMatch(m);
}

void Concordance::restore_match(intptr_t row, AutoMatch m)
{
	m_matches.insert(row, std::move(m));
	modify();
	file_modified();
}

Handle<Concordance> Concordance::unite(const Concordance &other, const String &label) const
{
	if (m_target_count != other.m_target_count) {
		throw error("Cannot unite get_concordances with different numbers of targets");
	}
	if (m_context_type != other.m_context_type) {
		throw error("Cannot unite get_concordances with different contexts");
	}
	if (m_context_length != other.m_context_length) {
		throw error("Cannot unite get_concordances with different context lengths");
	}

	std::set<AutoMatch, MatchLess> buffer;

	for (auto &match : m_matches) {
		buffer.insert(std::make_unique<Match>(*match));
	}
	for (auto &match : other.m_matches) {
		buffer.insert(std::make_unique<Match>(*match));
	}
	Array<AutoMatch> result;
	result.reserve((intptr_t)buffer.size());
	for (auto &match : buffer)
	{
		auto m = const_cast<AutoMatch&>(match).release();
		result.append(std::unique_ptr<Match>(m));
	}
	auto conc = make_handle<Concordance>(m_target_count, m_context_type, m_context_length, std::move(result), nullptr);
	conc->set_label(label, false);
	auto parent = Project::get()->data().get();
	parent->append(conc, false);

	return conc;
}

Handle<Concordance> Concordance::intersect(const Concordance &other, const String &label) const
{
	if (m_target_count != other.m_target_count) {
		throw error("Cannot intersect get_concordances with different numbers of targets");
	}
	if (m_context_type != other.m_context_type) {
		throw error("Cannot intersect get_concordances with different contexts");
	}
	if (m_context_length != other.m_context_length) {
		throw error("Cannot intersect get_concordances with different context lengths");
	}

	Array<AutoMatch> result;

	for (auto &match : m_matches)
	{
		// Matches are guaranteed to be sorted
		auto it = std::lower_bound(other.m_matches.begin(), other.m_matches.end(), match, MatchLess());

		if (it != other.m_matches.end() && **it == *match) {
			result.append(std::make_unique<Match>(*match));
		}
	}

	auto conc = make_handle<Concordance>(m_target_count, m_context_type, m_context_length, std::move(result), nullptr);
	conc->set_label(label, false);
	auto parent = Project::get()->data().get();
	parent->append(conc, false);

	return conc;
}

Handle<Concordance> Concordance::complement(const Concordance &other, const String &label) const
{
	if (m_target_count != other.m_target_count) {
		throw error("Cannot compute concordance complement for get_concordances with different numbers of targets");
	}
	if (m_context_type != other.m_context_type) {
		throw error("Cannot compute concordance complement for get_concordances with different contexts");
	}
	if (m_context_length != other.m_context_length) {
		throw error("Cannot compute concordance complement for get_concordances with different context lengths");
	}

	Array<AutoMatch> result;

	for (auto &match : other.m_matches)
	{
		// Matches are guaranteed to be sorted
		auto it = std::lower_bound(m_matches.begin(), m_matches.end(), match, MatchLess());

		if (it == m_matches.end() || **it != *match) {
			result.append(std::make_unique<Match>(*match));
		}
	}

	auto conc = make_handle<Concordance>(m_target_count, m_context_type, m_context_length, std::move(result), nullptr);
	conc->set_label(label, false);
	auto parent = Project::get()->data().get();
	parent->append(conc, false);

	return conc;
}

bool Concordance::update_match(intptr_t i)
{
	bool modified;
	auto result = m_matches[i]->update(modified);
	if (modified) modify();

	return result;
}

bool Concordance::is_layer(intptr_t col) const
{
	return col == 2;
}

} // namespace phonometrica
