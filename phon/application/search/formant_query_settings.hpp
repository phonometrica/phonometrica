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
