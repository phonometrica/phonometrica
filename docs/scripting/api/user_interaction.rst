User interaction
================

This page describes global function that allow you to interact with the user.


Functions
---------


.. function:: warning(message [, title])

Displays a warning dialog.

See also: :func:`alert`


------------

.. function:: alert(message [, title])

Displays an error dialog. This can be used for critical errors.

See also: :func:`warning`

------------

.. function:: info(message [, title])

Displays an information dialog. 

------------

.. function:: get\_input(label, title, text)

Displays an input dialog whose title is `title` and whose informative text is `label`. The dialog contains a 
field whose value is `text`. This function returns the content of the field.


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

.. function:: set\_status(message, timeout)

Displays ``message`` in the status bar for ``timeout`` seconds. If
``timeout`` is ``0``, the message is displayed until the next one
appears.
