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
 * Created: 31/10/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: display the results from an acoustic query (pitch, intensity, formants).                                   *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ACOUSTIC_QUERY_VIEW_HPP
#define PHONOMETRICA_ACOUSTIC_QUERY_VIEW_HPP

#include <QTableWidget>
#include <phon/gui/views/view.hpp>
#include <phon/application/query_table.hpp>

namespace phonometrica {

class Runtime;


class AcousticQueryView final : public View
{
	Q_OBJECT

public:

	AcousticQueryView(QWidget *parent, Runtime &rt, AutoQueryTable data, int type);


signals:

	void openAnnotation(AutoAnnotation, intptr_t, double, double);

private:

	void setupUi();

	void fillTable();

	Runtime &runtime;

	AutoQueryTable m_data;

	QTableWidget *m_table;


	Measurement::Type m_type = Measurement::Type::Formants;
};

} // namespace phonometrica

#endif // PHONOMETRICA_ACOUSTIC_QUERY_VIEW_HPP
