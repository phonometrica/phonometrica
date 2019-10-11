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
 * Purpose: scrollbar that displays the whole sound file in a sound view or annotation view.                           *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_WAVE_BAR_HPP
#define PHONOMETRICA_WAVE_BAR_HPP

#include <utility>
#include <QWidget>
#include <QPainter>
#include <phon/application/audio_data.hpp>

namespace phonometrica {

class WaveBar final : public QWidget
{
    Q_OBJECT

public:

    WaveBar(const std::shared_ptr<AudioData> &data, QWidget *parent = nullptr);

    double magnitude() const;

signals:

    void selectionStarted();

    // Update the plots when a user selects a new portion of the file.
    void timeSelection(double, double);

    // Update the sound zoom when a plot updates the scrollbar.
    void updatedXAxisSelection(double, double);

public slots:

    void setTimeSelection(double from, double to);

protected:

    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:

    int sampleToYPos(double s) const;

    std::shared_ptr<AudioData> m_data;

    double sampleToXPos(intptr_t s) const;

    intptr_t xPosToSample(double x) const;

    double timeToXPos(double t) const;

    double xPosToTime(double x) const;

    double startPosition() const;

    double endPosition() const;

    void notifySelection();

    void makePath(QPainterPath &path);

    // We cache the sinusoidal so that we don't need to recalculate it on each redraw if the geometry doesn't change.
    QPainterPath cached_path;

    // Cache the width of the plot
    int cached_width = 0;

    // Start and end time of the window currently displayed in the speech plots.
    std::pair<double, double> current_window;

    // Normalization constants
    double raw_magnitude;
};

} // namespace phonometrica


#endif //PHONOMETRICA_WAVE_BAR_HPP
