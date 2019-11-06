/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
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

struct FormantQuerySettings final : public Query::Settings
{
	FormantQuerySettings(double win_size, int nformant, double max_bw, double max_freq, int lpc_order, bool bw, bool erb, bool bark);

	FormantQuerySettings(double win_size, int nformant, double max_bw, double max_freq1, double max_freq2, double step, int lpc_order1, int lpc_order2,
	                     bool bw, bool erb, bool bark);


	bool is_acoustic() const override { return true; }

	bool is_formants() const override { return true; }

	bool is_automatic() const override { return true; }

	String get_header(int j) const override;

	int field_count() const override;

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
