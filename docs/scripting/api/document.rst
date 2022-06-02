Documents
=========

This page documents the ``Document`` type: this is an abstract base class for all files in a project that can be stored on disk and represented by a path. Subclasses include Annotation, Sound,
Dataset, Concordance and Script. 


Functions
---------


.. class:: Document


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

