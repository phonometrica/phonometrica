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

.. function:: add_property(file as Document, category as String, value as Object)

Adds a property to the annotation. ``category`` must be a string and ``value`` can be a string, a number or a Boolean.
If the file already has a property with the same category, the value will be replaced with the new one.


------------

.. function:: remove_property(file as Document, category as String)

Removes the property whose category is ``category`` from the annotation. If there is no such category, this method 
does nothing.

------------

.. function:: remove_property(file as Document, category)

Gets the property whose category is ``category`` from the annotation, or ``null`` if there is no such category.

------------

.. function:: bind_to_sound(annot as Annotaiton, path as String)

Binds the annotation to the sound file whose path is ``path``. If the sound file is not in the current project,
it will be automatically imported.

------------

.. function:: get_event_start(layer_index, event_index)

Gets the start time of an event on a given layer. Note that if the event is an instant, its start time is equal to
its end time.

------------

.. function:: get_event_end(layer_index, event_index)

Gets the end time of an event on a given layer. Note that if the event is an instant, its end time is equal to
its start time.

------------

.. function:: get_event_text(layer_index, event_index)


Gets the text of an event on a given layer.

------------

.. function:: set_event_text(layer_index, event_index, new_text)


Sets the text of an event on a given layer.


------------

.. function:: get_event_count(layer_index)

Returns the number of events on a given layer. 


------------

.. function:: get_layer_label(layer_index)


Gets the layer's label.

------------

.. function:: set_layer_label(layer_index, new_label)


Sets the layer's label.


Fields
------

.. attribute:: path

Returns the path of the sound file.


------------

.. attribute:: sound

Returns the ``Sound`` object to which the annotation is bound, or ``null`` if it is not bound to any sound.


------------

.. attribute:: layer_count

Returns the number of layers in the annotation.