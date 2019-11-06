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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/search/formant_query_settings.hpp>

namespace phonometrica {

FormantQuerySettings::FormantQuerySettings(double win_size, int nformant, double max_bw, double max_freq, int lpc_order, bool bw, bool erb,
                                           bool bark) : Query::Settings(Query::Type::Formants)
{
	this->win_size = win_size;
	this->nformant = nformant;
	this->max_bandwidth = max_bw;
	this->max_freq = max_freq;
	this->lpc_order = lpc_order;
	this->automatic = false;
	this->bandwidth = bw;
	this->erb = erb;
	this->bark = bark;
}

FormantQuerySettings::FormantQuerySettings(double win_size, int nformant, double max_bw, double max_freq1, double max_freq2, double step,
                                           int lpc_order1, int lpc_order2, bool bw, bool erb, bool bark) :
		Query::Settings(Query::Type::Formants)
{
	this->win_size = win_size;
	this->nformant = nformant;
	this->max_bandwidth = max_bw;
	this->max_freq1 = max_freq1;
	this->max_freq2 = max_freq2;
	this->step = step;
	this->lpc_order1 = lpc_order1;
	this->lpc_order2 = lpc_order2;
	this->automatic = true;
	this->bandwidth = bw;
	this->erb = erb;
	this->bark = bark;
}

String FormantQuerySettings::get_header(int j) const
{
	if (j <= nformant)
	{
		return String::format("F%d", j);
	}
	j -= nformant;

	if (j <= nformant)
	{
		if (bandwidth)
			return String::format("B%d", j);
		else if (erb)
			return String::format("E%d", j);
		else if (bark)
			return String::format("z%d", j);
	}
	j -= nformant;

	if (j <= nformant)
	{
		if (erb)
			return String::format("E%d", j);
		else if (bark)
			return String::format("z%d", j);
	}
	j -= nformant;

	if (bark)
		return String::format("z%d", j);

	return String();
}

int FormantQuerySettings::field_count() const
{
	int coeff = 1 + int(bandwidth) + int(erb) + int(bark);
	int n = nformant * coeff;
	// Add 2 for the maximum frequency and the LPC order
	if (this->automatic) n += 2;

	return n;
}

} // namespace phonometrica
