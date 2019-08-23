Interaction with the application
================================

This page documents the ``phon`` module.

General concepts
----------------

This module contains all functions that let users interact with Phonometrica. 


Functions
---------


.. module:: phon

.. function:: warning(message)

Displays a warning dialog.

See also: :func:`alert`


------------

.. function:: alert(message)

Displays an error dialog. This can be used for critical errors.

See also: :func:`warning`

------------

.. function:: info(message)

Displays an information dialog. 


------------

.. function:: open\_file\_dialog(message)

Displays a dialog that lets the user select a file.

See also: :func:`save\_file\_dialog`,
:func:`open\_directory\_dialog`


------------

.. function:: save\_file\_dialog(message)

Displays a dialog that lets the user choose a path to save a file.

See also: :func:`open\_file\_dialog`,
:func:`open\_directory\_dialog`


------------

.. function:: open\_directory\_dialog(message)

Displays a dialog that lets the user select a directory.

See also: :func:`save\_file\_dialog`,
:func:`open\_file\_dialog`


------------

.. function:: status(message, timeout)

Displays ``message`` in the status bar for ``timeout`` seconds. If
``timeout`` is ``0``, the message is displayed until the next one
appears.
