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
 * Purpose: display and edit annotation.                                                                               *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#ifndef PHONOMETRICA_ANNOTATION_VIEW_HPP
#define PHONOMETRICA_ANNOTATION_VIEW_HPP

#include <phon/gui/views/speech_view.hpp>
#include <phon/gui/layer_widget.hpp>
#include <phon/application/annotation.hpp>
#include <phon/gui/toolbar.hpp>

class QToolButton;

namespace phonometrica {

class AnnotationView final : public SpeechView
{
    Q_OBJECT

public:

    AnnotationView(Runtime &rt, std::shared_ptr<Annotation> annot, QWidget *parent = nullptr);

    bool save() override;

    void openSelection(intptr_t layer, double from, double to);

protected:

    void addAnnotationMenu(Toolbar *toolbar) override;

    void addAnnotationLayers(QVBoxLayout *layout) override;

	void addLayersToYAxis() override;

private slots:

    void focusLayer(intptr_t i);

    void focusEvent(intptr_t layer, double time);

    void setMovingAnchor(intptr_t layer, double time);

    void resetAnchorMovement(intptr_t layer);

    void saveAnnotation(bool);

    void createLayer(bool);

    void removeLayer(bool);

    void clearLayer(bool);

private:

	int widgetIndex(int layer_index);

	int getFocusedLayer() const;

	LayerWidget * addAnnotationLayer(intptr_t i);

	QToolButton *link_button = nullptr;

    std::shared_ptr<Annotation> annot;

    // Layer widgets, in the order they are stored and displayed
    Array<LayerWidget*> layers;
};

} // namespace phonometrica

#endif // PHONOMETRICA_ANNOTATION_VIEW_HPP
