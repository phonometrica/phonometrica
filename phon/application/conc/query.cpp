/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019-2022 Julien Eychenne                                                                             *
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
 * Created: 01/02/2021                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <wx/progdlg.h>
#include <phon/runtime.hpp>
#include <phon/application/conc/query.hpp>
#include <phon/application/project.hpp>
#include <phon/utils/xml.hpp>
#include <phon/utils/file_system.hpp>

namespace phonometrica {

Query::Query(Directory *parent, String path) :
		Document(meta::get_class<Query>(), parent, std::move(path))
{
	if (!m_path.empty()) {
		load();
	}
}

void Query::add_metaconstraint(AutoMetaConstraint m, bool mutate)
{
	m_metaconstraints.append(std::move(m));
	if (mutate) m_content_modified = true;
}

void Query::add_constraint(Constraint c, bool mutate)
{
	m_constraints.append(std::move(c));
	if (mutate) m_content_modified = true;
}

Array<Handle<Annotation>> Query::filter_annotations(Array<Handle<Annotation>> candidates) const
{
	if (m_metaconstraints.empty()) {
		return candidates;
	}
	Array<Handle<Annotation>> result;

	for (auto &candidate : candidates)
	{
		if (filter_metadata(candidate.get())) {
			result.append(std::move(candidate));
		}
	}

	return result;
}

bool Query::filter_metadata(const Document *file) const
{
	// Reject files that do not satisfy all the constraints.
	for (auto &constraint : m_metaconstraints)
	{
		if (!constraint->filter(file)) {
			return false;
		}
	}

	return true;
}

String Query::label() const
{
	return m_label;
}

void Query::set_label(String value, bool mutate)
{
	m_label = std::move(value);
	if (mutate) m_content_modified = true;
}

void Query::set_selection(Array<Handle<Annotation>> files)
{
	selected_annotations = std::move(files);
	m_content_modified = true;
}

void Query::clear()
{
	m_metaconstraints.clear();
	m_constraints.clear();
	selected_annotations.clear();
	m_label = String();
	m_content_modified = true;
}


void Query::load()
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
		throw error("Expected a text query, got a % file instead", attr.as_string());
	}

	attr = root.attribute("label");
	if (attr) {
		set_label(attr.value(), false);
	}
	else {
		set_label(filesystem::base_name(m_path), false);
	}

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Metadata"))
		{
			metadata_from_xml(node);
		}
		else if (node.name() == str("MetaConstraints"))
		{
			parse_metaconstraints_from_xml(node);
		}
		else if (node.name() == str("Constraints"))
		{
			parse_constraints_from_xml(node);
		}
		else if (node.name() == str("Options"))
		{
			parse_options_from_xml(node);
		}
		else
		{
			throw error("Invalid node in text query: %", node.name());
		}
	}

	m_loaded = true;
}

void Query::parse_metaconstraints_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Description"))
		{
			auto attr = node.attribute("operator");
			auto op = DescMetaConstraint::name_to_op(attr.value());
			String value = node.text().get();
			add_metaconstraint(std::make_shared<DescMetaConstraint>(op, std::move(value)), false);
		}
		else if (node.name() == str("FileSelection"))
		{
			for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
			{
				if (subnode.name() != str("File")) {
					throw error("Invalid XML node in text query file selection");
				}
				String path = subnode.text().get();
				auto vfile = Project::get()->get(path);
				auto annot = recast<Annotation>(vfile);

				if (!annot) {
					throw error("Invalid annotation in text query file selection");
				}
				selected_annotations.append(std::move(annot));
			}
		}
		else if (node.name() == str("Property"))
		{
			auto cat_attr = node.attribute("category");
			if (!cat_attr) {
				throw error("Missing category in text query property");
			}
			String category = cat_attr.value();
			auto type_attr = node.attribute("type");
			if (!type_attr) {
				throw error("Missing type in text query property");
			}
			if (type_attr.value() == str("text"))
			{
				Array<String> values;
				for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
				{
					if (subnode.name() != str("Value")) {
						throw error("Expected a Value node in text property");
					}
					values.append(subnode.text().get());
				}
				m_metaconstraints.append(std::make_unique<TextMetaConstraint>(category, std::move(values)));
			}
			else if (type_attr.value() == str("numeric"))
			{
				auto op_node = node.attribute("operator");
				auto op = NumericMetaConstraint::name_to_op(op_node.value());
				auto value = std::make_pair<double,double>(0, 0);

				auto first_node = node.first_child();
				if (!first_node || first_node.name() != str("Value")) {
					throw error("Expected a Value node in numeric property");
				}
				String first_value = first_node.text().get();
				bool ok;
				value.first = first_value.to_float(&ok);
				if (!ok) {
					throw error("Invalid numeric value in numeric property");
				}
				if (op == NumericMetaConstraint::Operator::InclusiveRange || op == NumericMetaConstraint::Operator::ExclusiveRange)
				{
					auto second_node = first_node.next_sibling();
					if (!second_node || second_node.name() != str("Value")) {
						throw error("Expected a second Value node in numeric property with range operator");
					}
					String second_value = second_node.text().get();
					value.second = second_value.to_float(&ok);
					if (!ok) {
						throw error("Invalid numeric value in numeric property");
					}
				}
				m_metaconstraints.append(std::make_unique<NumericMetaConstraint>(category, op, value));
			}
			else if (type_attr.value() == str("boolean"))
			{
				auto subnode = node.first_child();
				if (!subnode || subnode.name() != str("Value")) {
					throw error("Expected a Value node in Boolean property");
				}
				bool value = subnode.text().as_bool();
				m_metaconstraints.append(std::make_unique<BooleanMetaConstraint>(category, value));
			}
			else
			{
				throw error("Invalid property type in text query: %", node.name());
			}
		}
		else
		{
			throw error("Invalid metaconstraint in text query: %", node.name());
		}
	}
}

void Query::parse_constraints_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() != str("Constraint"))
		{
			throw error("Expected a Constraint node in text query, got: %", node.name());
		}

		auto op_attr = node.attribute("operator");
		if (!op_attr) {
			throw error("Missing operator in Constraint node");
		}
		auto rel = Constraint::name_to_relation(op_attr.value());
		Constraint::Operator op = Constraint::Operator::None;
		int layer_index = -1;
		bool case_sensitive = false;
		String layer_pattern, target;

		for (auto subnode = node.first_child(); subnode; subnode = subnode.next_sibling())
		{
			if (subnode.name() == str("Layer"))
			{
				auto attr = subnode.attribute("type");
				if (!attr) {
					throw error("Missing type attribute in Layer node");
				}
				if (attr.value() == str("index"))
				{
					String text = subnode.text().get();
					bool ok;
					layer_index = text.to_int(&ok);
					if (!ok) {
						throw error("Invalid index layer_index in Constraint node");
					}
				}
				else
				{
					layer_pattern = subnode.text().get();
				}
			}
			else if (subnode.name() == str("Target"))
			{
				auto attr = subnode.attribute("operator");
				if (!attr) {
					throw error("Missing attribute 'operator' in Target node");
				}
				op = Constraint::name_to_operator(attr.value());

				attr = subnode.attribute("case_sensitive");
				if (!attr) {
					throw error("Missing attribute 'case_sensitive' in Target Node");
				}
				case_sensitive = attr.value() == str("true");

				target = subnode.text().get();
			}
			else
			{
				throw error("Invalid Constraint subnode in text query: %", subnode.name());
			}
		}

		Constraint c;
		c.case_sensitive = case_sensitive;
		c.op = op;
		c.relation = rel;
		c.layer_index = layer_index;
		c.layer_pattern = layer_pattern;
		c.target = target;
		m_constraints.append(std::move(c));
	}
}

void Query::write()
{
	xml_document doc;

	auto root = doc.append_child("Phonometrica");
	auto attr = root.append_attribute("class");
	attr.set_value(class_name().data());
	attr = root.append_attribute("label");
	attr.set_value(m_label.data());
	auto metadata_node = root.append_child("Metadata");
	metadata_to_xml(metadata_node);

	auto meta_node = root.append_child("MetaConstraints");
	auto file_sel_node = meta_node.append_child("FileSelection");
	for (auto &file : selected_annotations)
	{
		add_data_node(file_sel_node, "File", file->path());
	}
	for (auto &mc : m_metaconstraints)
	{
		mc->to_xml(meta_node);
	}

	// Options
	auto option_node = root.append_child("Options");
	auto ctx_node = option_node.append_child("Context");
	auto type_attr = ctx_node.append_attribute("type");
	switch (m_context)
	{
		case Context::Labels:
		{
			type_attr.set_value("labels");
			ctx_node.append_attribute("ref").set_value(m_ref_constraint);
		} break;
		case Context::KWIC:
		{
			type_attr.set_value("kwic");
			ctx_node.append_attribute("ref").set_value(m_ref_constraint);
			ctx_node.append_attribute("length").set_value(m_context_length);
		} break;
		default:
			type_attr.set_value("none");
	}

	// Constraints
	auto data_node = root.append_child("Constraints");
	for (auto &constraint : m_constraints)
	{
		constraint.to_xml(data_node);
	}

	write_xml(doc, m_path);
}

int Query::context_length() const
{
	return m_context_length;
}

void Query::set_context_length(int context_length)
{
	m_context_length = context_length;
}

Query::Context Query::context() const
{
	return m_context;
}

void Query::set_context(Query::Context context)
{
	m_context = context;
}

int Query::reference_constraint() const
{
	return m_ref_constraint;
}

void Query::set_reference_constraint(int value)
{
	m_ref_constraint = value;
}

void Query::parse_options_from_xml(xml_node root)
{
	using str = std::string_view;

	for (auto node = root.first_child(); node; node = node.next_sibling())
	{
		if (node.name() == str("Context"))
		{
			auto type_attr = node.attribute("type");
			if (!type_attr) {
				throw error("Missing type attribute in Context node");
			}
			auto type = str(type_attr.value());

			if (type == str("none"))
			{
				m_context = Context::None;
			}
			else
			{
				auto ref_attr = node.attribute("ref");
				if (!ref_attr) {
					throw error("Missing ref attribute in Context node");
				}

				String text = ref_attr.value();
				bool ok;
				m_ref_constraint = (int) text.to_int(&ok);
				if (!ok || m_ref_constraint < 0) {
					throw error("Invalid index for reference constraint in Context node: %", text);
				}

				if (type == str("kwic"))
				{
					auto len_attr = node.attribute("length");
					if (!len_attr) {
						throw error("Missing length attribute in Context node");
					}
					text = len_attr.value();
					m_context_length = (int) text.to_int(&ok);
					if (!ok || m_context_length < 0) {
						throw error("Invalid length in Context node: %", text);
					}
					m_context = Context::KWIC;
				}
				else if (type == str("labels"))
				{
					m_context = Context::Labels;
				}
				else
				{
					throw error("Invalid type in Context node: %", type);
				}
			}
		}
		else
		{
			throw error("Invalid option in text query: %", node.name());
		}

	}
}

Handle<Query> Query::copy() const
{
	auto copy = make_handle<Query>(this->parent(), String());
	copy->m_constraints = m_constraints;
	copy->m_metaconstraints = m_metaconstraints;
	copy->selected_annotations = selected_annotations;
	copy->m_label = m_label;

	copy->m_context = m_context;
	copy->m_context_length = m_context_length;
	copy->m_ref_constraint = m_ref_constraint;
	copy->m_content_modified = true;

	return copy;
}

Handle<Concordance> Query::execute()
{
	auto conc = make_handle<Concordance>(m_constraints.size(), m_context, m_context_length, search(), nullptr);
	auto label = this->label();
	if (label.starts_with("Query ")) {
		label.replace_first("Query ", "Concordance ");
	}
	conc->set_label(label, false);
	Project::get()->add_temp_concordance(conc);

	return conc;
}

Array<AutoMatch> Query::search()
{
	Array<AutoMatch> result;
	auto tmp = selected_annotations.empty() ? Project::get()->get_annotations() : selected_annotations;
	auto annotations = filter_annotations(std::move(tmp));

	for (auto &c : m_constraints) {
		c.compile();
	}

	int count = (int)annotations.size(), t = 0;
	wxProgressDialog progress(_("Executing query"), _("Processing annotations..."), count, nullptr, wxPD_AUTO_HIDE|wxPD_APP_MODAL|wxPD_CAN_ABORT);

#ifdef PHON_TIMING
	auto first_time = clock();
#endif

	for (auto &annot : annotations)
	{
		// Cancelled by user?
		if (!progress.Update(t++)) {
			return result;
		}
		try
		{
			annot->open();
			auto matches = search_annotation(annot);

			result.reserve(result.size() + matches.size());
			for (auto &m : matches) {
				result.append(std::move(m));
			}
		}
		catch (std::exception &e)
		{
			throw error("error in annotation %: %", annot->path(), e.what());
		}
	}

#ifdef PHON_TIMING
	auto last_time = clock();
	auto total = double(last_time-first_time) * 1000 / CLOCKS_PER_SEC;
	std::cerr << "Total loading time for " << annotations.size() << " annotations: " << total << " ms\n";
	std::cerr << "Average per annotation: " << (total/annotations.size()) << " ms\n";
#endif

	return result;
}

Array<AutoMatch> Query::search_annotation(const Handle<Annotation> &annot)
{
	// We maintain a list of the layer indices we have already seen, so that we don't scan the same layer twice
	Array<int> seen;

	auto matches = find_matches(annot, m_constraints[1], Array<AutoMatch>(), seen, Constraint::Relation::None, m_ref_constraint == 1);

	for (intptr_t i = 2; i <= m_constraints.size(); i++)
	{
		if (matches.empty()) {
			return matches;
		}
		matches = find_matches(annot, m_constraints[i], std::move(matches), seen, m_constraints[i - 1].relation, m_ref_constraint == i);
	}

	return matches;
}

Array <AutoMatch>
Query::find_matches(const Handle<Annotation> &annot, const Constraint &constraint, Array <AutoMatch> matches,
                    Array<int> &blacklist, Constraint::Relation op, bool is_ref) const
{
	if (constraint.use_index())
	{
		if (constraint.layer_index == 0)
		{
			for (intptr_t i = 1; i <= annot->layer_count(); i++)
			{
				matches = find_matches(annot, constraint, std::move(matches), i, blacklist, op, is_ref);
			}

			return matches;
		}
		else
		{
			return find_matches(annot, constraint, std::move(matches), constraint.layer_index, blacklist, op, is_ref);
		}
	}
	else
	{
		auto &layers = annot->layers();

		for (intptr_t i = 1; i <= layers.size(); i++)
		{
			auto &layer = layers[i];

			if (constraint.layer_regex->match(layer->label))
			{
				matches = find_matches(annot, constraint, std::move(matches), i, blacklist, op, is_ref);
			}
		}

		return matches;
	}
}

Array <AutoMatch>
Query::find_matches(const Handle<Annotation> &annot, const Constraint &constraint, Array <AutoMatch> matches,
                    intptr_t layer_index, Array<int> &seen, Constraint::Relation op, bool is_ref) const
{
	using Op = Constraint::Relation;

	// Case 1: invalid index
	if (layer_index > annot->layer_count()) {
		return Array<AutoMatch>();
	}
	// Case 2. First (and possibly unique) constraint: create matches
	if (op == Op::None)
	{
		auto &events = annot->get_layer_events(layer_index);

		for (auto &event : events)
		{
			intptr_t pos = 0;
			std::unique_ptr<Match::Target> target;
			while (true)
			{
				target = find_target(event, constraint, layer_index, pos, is_ref);
				if (target)
				{
					matches.append(std::make_unique<Match>(annot, std::move(target)));
					if (pos == -1) {
						break; // found a match with "equals", don't need to search again
					}
				}
				else
				{
					break;
				}
			}
		}
		seen.append(layer_index);

		return matches;
	}
	// Case 3. Hierarchical relation across layers: keep the layers that match the new constraint
	// and append a target to them
	if (Constraint::is_hierarchical(op))
	{
		if (seen.contains(layer_index)) {
			// The user mistakenly chose a layer_index that was already processed
			throw error("[Query error] Two constraints in a hierarchical relation refer to the same layer_index (layer_index %)", layer_index);
		}
		Array<AutoMatch> new_matches;

		switch (op)
		{
			case Op::Dominance:
			case Op::StrictDominance:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto start = previous_target.event->start_time();
					auto end = previous_target.event->end_time();
					auto events = annot->get_slice(layer_index, start, end);

					for (auto &event : events)
					{
						if (op == Op::StrictDominance && (event->start_time() <= start || event->end_time() >= end))
						{
							continue;
						}
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			case Op::Alignment:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto time = previous_target.event->start_time();
					auto event = annot->find_event_starting_at(layer_index, time);
					if (event && event->end_time() == previous_target.end_time())
					{
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			case Op::LeftAlignment:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto time = previous_target.event->start_time();
					auto event = annot->find_event_starting_at(layer_index, time);
					if (event)
					{
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			case Op::RightAlignment:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto time = previous_target.event->end_time();
					auto event = annot->find_event_ending_at(layer_index, time);
					if (event)
					{
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			case Op::Precedence:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto time = previous_target.event->start_time();
					auto event = annot->find_previous_event(layer_index, time);
					if (event)
					{
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			case Op::Subsequence:
			{
				for (auto &match : matches)
				{
					auto &previous_target = match->last_target();
					auto time = previous_target.event->end_time();
					auto event = annot->find_next_event(layer_index, time);
					if (event)
					{
						intptr_t pos = 0;
						auto target = find_target(event, constraint, layer_index, pos, is_ref);
						if (target)
						{
							previous_target.next = std::move(target);
							new_matches.append(std::move(match));
						}
					}
				}
			} break;
			default:
				break;
		}
		seen.append(layer_index);

		return new_matches;
	}

	return Array<AutoMatch>();
}

std::unique_ptr<Match::Target>
Query::find_target(const AutoEvent &event, const Constraint &constraint, intptr_t layer_index, intptr_t &pos, bool is_ref) const
{
	switch (constraint.op)
	{
		case Constraint::Operator::Equals:
		{
			if (constraint.case_sensitive)
			{
				if (event->text() == constraint.target)
				{
					pos = -1;
					return std::make_unique<Match::Target>(event, constraint.target, layer_index, 0, is_ref);
				}
			}
			else if (String::iequals(event->text(), constraint.target))
			{
				pos = -1;
				return std::make_unique<Match::Target>(event, event->text(), layer_index, 0, is_ref);
			}
		}
		break;
		case Constraint::Operator::Contains:
		{
			if (constraint.case_sensitive)
			{
				auto &text = event->text();
				auto it = text.begin() + pos;
				if ((it = text.find(constraint.target, it)) != text.end())
				{
					auto offset = intptr_t (it - text.begin());
					pos = offset + constraint.target.size();

					return std::make_unique<Match::Target>(event, constraint.target, layer_index, offset, is_ref);
				}
			}
			else
			{
				auto &text = event->text();
				auto it = text.begin() + pos;
				if ((it = text.ifind(constraint.target, it)) != text.end())
				{
					auto offset = intptr_t (it - text.begin());
					String::const_iterator end = it;
					text.advance(end, constraint.target.grapheme_count());
					String value(it, intptr_t(end-it));
					pos = intptr_t(end - text.begin());

					return  std::make_unique<Match::Target>(event, std::move(value), layer_index, offset, is_ref);
				}
			}
		}
		break;
		case Constraint::Operator::Matches:
		{
			auto &re = *constraint.regex;
			auto &text = event->text();
			auto it = text.begin() + pos;
			if (re.match(event->text(), it))
			{
				auto matched_text = re.capture(0);
				auto offset = intptr_t (re.capture_start_iter(0) - text.begin());
				pos = intptr_t (re.capture_end_iter(0) - text.begin());

				return std::make_unique<Match::Target>(event, std::move(matched_text), layer_index, offset, is_ref);
			}
		}
		break;
		default:
			break;

	}

	return nullptr;
}

bool Query::empty()
{
	return m_constraints.empty() && m_metaconstraints.empty();
}

void Query::initialize(Runtime &rt)
{

}

} // namespace phonometrica
