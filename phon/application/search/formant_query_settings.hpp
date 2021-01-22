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
 * Created: 06/11/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: settings for formant queries. These are passed around from the search box to the query to the query view.  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_FORMANT_QUERY_SETTINGS_HPP
#define PHONOMETRICA_FORMANT_QUERY_SETTINGS_HPP

#include <phon/application/search/query.hpp>

namespace phonometrica {

struct AcousticQuerySettings : public Query::Settings
{
	enum class Method
	{
		Mid,
		Average, // n-point average
		Series   // n-point series (not implemented yet)
	};

	AcousticQuerySettings(Query::Type t, Method method, Array<float> points, Array<int> label_indexes, bool surrounding);

	int extra_count() const override;

	bool has_surrounding_context() const override { return surrounding; }

	bool has_extra_labels() const { return !label_indexes.empty(); }

	String get_extra_label(intptr_t i) const { return String::format("Layer %d", int(label_indexes[i])); }
	Array<float> points;       // measurement points
	Array<int> label_indexes;  // additional labels extracted, excluding the surrounding labels.
	Method method;             // measurement method
	bool surrounding;          // include surrounding labels on the matched layer?
};


struct FormantQuerySettings final : public AcousticQuerySettings
{
	FormantQuerySettings(Method method, bool add_surrounding, Array<float> points, Array<int> label_indexes, double win_size, int nformant, double max_bw,
			double max_freq, int lpc_order, bool bw, bool erb, bool bark);

	FormantQuerySettings(Method method, bool add_surrounding, Array<float> points, Array<int> label_indexes, double win_size, int nformant, double max_bw,
			double max_freq1, double max_freq2, double step, int lpc_order1, int lpc_order2,bool bw, bool erb, bool bark);

	bool is_acoustic() const override { return true; }

	bool is_formants() const override { return true; }

	bool is_automatic() const override { return automatic; }

	String get_header(int j) const override;

	int total_field_count() const override;

	double max_freq  = 0; // manual
	double max_freq1 = 0; // automatic
	double max_freq2 = 0; // automatic
	double step = 0;      // automatic

	int lpc_order  = 0; // manual
	int lpc_order1 = 0; // automatic
	int lpc_order2 = 0; // automatic

	// Shared values
	int nformant;
	double win_size;
	double max_bandwidth;

	bool automatic;
	bool bandwidth;
	bool erb;
	bool bark;
};

} // namespace phonometrica

#endif // PHONOMETRICA_FORMANT_QUERY_SETTINGS_HPP
