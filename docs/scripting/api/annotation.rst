Annotations
===========

This page documents the ``Annotation`` type, which corresponds to a time-aligned annotation of a sound file, on one or more layers.


Functions
---------


.. class:: Annotation


.. function:: get_annotations()

Return a list of all the annotations in the current project.

------------


.. function:: get_selected_annotations()

Return a list of all the selected annotations in the current project.

------------


.. function:: get_annotation(path as String)

Return the ``Annotation`` object from the current project whose path is ``path``, or ``null`` if there is no such
annotation. If the object exists but is not an annotation, an error is thrown.


------------

.. function:: get_current_annotation()

Return the ``Annotation`` object loaded in the current view, or ``null`` if the current view is not an annotation view.


------------

.. function:: bind_to_sound(annot as Annotation, path as String)

Binds the annotation to the sound file whose path is ``path``. If the sound file is not in the current project,
it will be automatically imported.

------------

.. function:: get_event_start(annot as Annotation, layer_index as Integer, event_index as Integer)

Gets the start time of an event on a given layer. Note that if the event is an instant, its start time is equal to
its end time.

------------

.. function:: get_event_end(annot as Annotation, layer_index as Integer, event_index as Integer)

Gets the end time of an event on a given layer. Note that if the event is an instant, its end time is equal to
its start time.

------------

.. function:: get_event_text(annot as Annotation, layer_index as Integer, event_index as Integer)


Gets the text of an event on a given layer.

------------

.. function:: set_event_text(annot as Annotation, layer_index as Intger, event_index as Integer, text as String)


Sets the text of an event on a given layer.


------------

.. function:: get_event_count(annot as Annotation, layer_index as Integer)

Returns the number of events on a given layer. 


------------

.. function:: get_layer_label(annot as Annotation, layer_index as Integer)


Gets the layer's label.

------------

.. function:: set_layer_label(annot as Annotation, layer_index as Integer, new_label as String)


Sets the layer's label.


Fields
------

.. attribute:: path

Returns the path of the sound file.


------------

.. attribute:: sound

Returns the ``Sound`` object to which the annotation is bound, or ``null`` if it is not bound to any sound.


------------

.. attribute:: nlayer

Returns the number of layers in the annotation.