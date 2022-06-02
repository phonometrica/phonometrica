User interaction
================

This page describes global functions that allow you to interact with the user.


Global functions
----------------


.. function:: create_progress_dialog(message as String, title as String, count as Integer)

Create a progress dialog with the provided message and title, set up for `count` elements. You must update the value of the dialog using `update_progress_dialog()`.

See also: :func:`update_progress_dialog`


------------

.. function:: update_progress_dialog(value as Integer)

Update the progress dialog to the provided `value`. If `value` equals the progress dialog's count, the dialog is closed. The first value that should be provided to this
function is 1, and the last one is the number of elements passed to the dialog when it was created. 

See also: :func:`create_progress_dialog`


------------

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

.. function:: ask(message [, title])

Asks a Yes/No question to the user. Returns ``true`` of the user clicked ``Yes``, and ``false`` otherwise.

------------

.. function:: get_input(label, title, text)

Displays an input dialog whose title is `title` and whose informative text is `label`. The dialog contains a 
field whose value is `text`. This function returns the content of the field.


------------

.. function:: open_file_dialog(message)

Displays a dialog that lets the user select a file.

See also: :func:`save_file_dialog`,
:func:`open_directory_dialog`


------------

.. function:: save_file_dialog(message)

Displays a dialog that lets the user choose a path to save a file.

See also: :func:`open_file_dialog`,
:func:`open_directory_dialog`


------------

.. function:: open_directory_dialog(message)

Displays a dialog that lets the user select a directory.

See also: :func:`save_file_dialog`,
:func:`open_file_dialog`


------------

.. function:: set_status(message, timeout)

Displays ``message`` in the status bar for ``timeout`` seconds. If
``timeout`` is ``0``, the message is displayed until the next one
appears.


------------

.. function:: view_text(path [, title [, width]])

Opens the plain text file ``path`` in a new dialog. Optionally, you can specify the dialog's ``title`` and set its
``width``.
