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
 * Created: 23/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display Y-axis information in sound and annotation views.                                                  *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_Y_AXIS_WIDGET_HPP
#define PHONOMETRICA_Y_AXIS_WIDGET_HPP

#include <QWidget>
#include <phon/array.hpp>

namespace phonometrica {

class SpeechPlot;

class YAxisWidget final : public QWidget
{
    Q_OBJECT

public:

    YAxisWidget(QWidget *parent = nullptr);

    void addPlot(SpeechPlot *plot);

    void clearPlots();

public slots:

    void refresh();

protected:

    void paintEvent(QPaintEvent *) override;

private:

    // Non-owning references to visible plots (in vertical order)
    Array<SpeechPlot*> plots;

};

} // namespace phonometrica

#endif // PHONOMETRICA_Y_AXIS_WIDGET_HPP
