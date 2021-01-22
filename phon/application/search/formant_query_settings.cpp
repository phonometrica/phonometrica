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
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/application/search/formant_query_settings.hpp>

namespace phonometrica {

AcousticQuerySettings::AcousticQuerySettings(Query::Type t, Method method, Array<float> points, Array<int> label_indexes, bool surrounding) :
		Query::Settings(t), points(std::move(points)), label_indexes(std::move(label_indexes)), method(method)
{
	this->surrounding = surrounding;
}

int AcousticQuerySettings::extra_count() const
{
	int n = label_indexes.size();
	if (this->surrounding) n += 2;

	return n;
}

FormantQuerySettings::FormantQuerySettings(Method method, bool add_surrounding, Array<float> points, Array<int> label_indexes, double win_size, int nformant,
		double max_bw, double max_freq, int lpc_order, bool bw, bool erb, bool bark) :
		AcousticQuerySettings(Query::Type::Formants, method, std::move(points), std::move(label_indexes), add_surrounding)
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

FormantQuerySettings::FormantQuerySettings(Method method, bool add_surrounding, Array<float> points, Array<int> label_indexes, double win_size, int nformant,
		double max_bw, double max_freq1, double max_freq2, double step, int lpc_order1, int lpc_order2, bool bw, bool erb, bool bark) :
		AcousticQuerySettings(Query::Type::Formants, method, std::move(points), std::move(label_indexes), add_surrounding)
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

int FormantQuerySettings::total_field_count() const
{
	int coeff = 1 + int(bandwidth) + int(erb) + int(bark);
	int n = nformant * coeff;
	// Add 2 for left and right context
	if (this->surrounding) n += 2;
	// Add 2 for the maximum frequency and the LPC order
	if (this->automatic) n += 2;
	// Add additional labels
	n += label_indexes.size();

	return n;
}


} // namespace phonometrica
