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
 * Created: 29/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Display pitch track.                                                                                       *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_PITCH_PLOT_HPP
#define PHONOMETRICA_PITCH_PLOT_HPP

#include <phon/gui/speech_plot.hpp>

namespace phonometrica {

class Runtime;


class PitchPlot final : public SpeechPlot
{
    Q_OBJECT

public:

    PitchPlot(Runtime &rt, const AutoSound &sound, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

protected:

    void renderPlot(QPaintEvent *event) override;

    bool needsRefresh() const override;

	void readSettings() override;

	void emptyCache() override;

private:

    double pitchToYPos(double hz);

    void calculatePitch();

    Array<double> pitch_data;

    double min_pitch;

    double max_pitch;

    double time_step; // in seconds

    double voicing_threshold; // 0.5 > t > 0.2
};

} // namespace phonometrica

#endif // PHONOMETRICA_PITCH_PLOT_HPP
