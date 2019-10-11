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
 * Created: 18/03/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: Widget displaying a waveform.                                                                              *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVEFORM_HPP
#define PHONOMETRICA_WAVEFORM_HPP

#include <QWidget>
#include <phon/gui/speech_plot.hpp>

namespace phonometrica {


class Waveform final : public SpeechPlot
{
    Q_OBJECT

    enum class Scaling
    {
    	Global,
    	Local,
    	Fixed
    };

public:

    Waveform(Runtime &rt, std::shared_ptr<AudioData> data, QWidget *parent = nullptr);

    void drawYAxis(QWidget *y_axis, int y1, int y2) override;

    void informWindow();

    void setGlobalMagnitude(double value);

public slots:

	void updateWindow(double start_time, double end_time) override;

protected:

    void renderPlot(QPaintEvent *event) override;

    bool isMainPlot() const override { return true; }

    bool needsRefresh() const override;

	void moveWindow(double t1, double t2) override;

	void readSettings() override;

	void emptyCache() override;

private:

	void setMagnitude(double value);

    void drawWave();

    void setLocalMagnitude(const double *from, const double *to);

    // Map sample to plot y coordinate.
    double sampleToHeight(double s) const;

    // We cache the sinusoidal so that we don't need to recalculate it on each redraw if the geometry and window
    // haven't changed.
    QPainterPath cached_path;

    // Cache the width and height of the plot
    int cached_width = 0;
    int cached_height = 0;

    Scaling scaling = Scaling::Fixed;
    double magnitude = 1.0;
    double global_magnitude = 1.0;

    std::pair<double,double> extrema = {-1, 1};
};

} // namespace phonometrica

#endif // PHONOMETRICA_WAVEFORM_HPP
