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
 * Created: 07/09/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/search/query_match.hpp>

namespace phonometrica {

double QueryMatch::start_time() const
{
	auto match = this;
	auto value = (std::numeric_limits<double>::max)();

	while (match)
	{
		auto t = match->m_event->start_time();
		if (t < value) value = t;
		match = match->m_next.get();
	}

	return value;
}

double QueryMatch::end_time() const
{
	auto match = this;
	double value = 0;

	while (match)
	{
		auto t = match->m_event->end_time();
		if (t > value) value = t;
		match = match->m_next.get();
	}

	return value;
}

bool QueryMatch::operator<(const QueryMatch &other) const
{
	int cmp;

	// First compare file names.
	cmp = this->annotation()->path().compare(other.annotation()->path());

	if (cmp < 0) {
		return true;
	}
	else if (cmp > 0) {
		return false;
	}

	// Next, compare tier in the file
	cmp = (this->layer_index() - other.layer_index());

	if (cmp < 0) {
		return true;
	}
	else if (cmp > 0) {
		return false;
	}

	// Next, Compare temporal position
	double delta = (this->start_time() - other.start_time());

	if (delta < 0) {
		return true;
	}
	else if (delta > 0) {
		return false;
	}

	// Finally compare position in the event.
	return this->position() < other.position();
}

size_t QueryMatch::hash() const
{
	return annotation()->path().hash() + std::hash<int>{}(layer_index()) + text().hash() + std::hash<int>{}(position());
}
} // namespace phonometrica
