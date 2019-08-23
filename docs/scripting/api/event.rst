Event handling
==============

This page documents the ``event`` module, which is responsible for
event handling in Phonometrica.

General concepts
----------------

Phonometrica provides an event handling mechanism which allows you to connect events to *callback* function. 
An event is a unique identifier which can be triggered anywhere, for instance when a button is clicked. It
can be bound to any number of callback functions, which may or may
not return a value. Whenever an event is *emitted*, all the callbacks connected to it are executed 
(in an unspecified order).

This mechanism is used throughout Phonometrica, as it provides hooks that can be used by
plugins to react to events triggered by the program. For
example, an event is emitted whenever a file is loaded, which can be
used to add custom metadata to each file, among other things.

Functions
---------

.. module:: event

.. function:: create()

Create and return a new event identifier (id). Each id is guaranteed to
be unique, such that two different calls to ``create`` will never yield the
same id.

If you need to store an id for subsequent use, store it in a variable.

.. code:: phon

    var my_event = event.create()
    # Do something with my_event...


------------

.. function:: connect(id, callback)

Connect event ``id`` to function ``callback``. The callback can take one argument and can return a value.

.. code:: phon

    var e = event.create()

    function f(name)
        print("Hold the door, " .. name)
    end

    event.connect(e, f)

    # Print "Hold the door, Hodor" to the standard output
    event.emit(e, "Hodor")

See also: :func:`disconnect`, :func:`emit`


------------

.. function:: disconnect(id, callback)

Disconnect event ``id`` from function ``callback``. If ``id`` and ``callback``
are not connected, this function does nothing.

.. code:: phon

    var e = event.create()

    function f(name)
        print("Hold the door, " .. name)
    end

    event.connect(evt, f)

    # Print "Hold the door, Hodor" to the standard output
    event.emit(e, "Hodor")

    event.disconnect(e, f)

    # Do nothing since e and f are no longer connected
    event.emit(e, "Hodor")

See also: :func:`connect`, :func:`emit`


------------

.. function:: emit(id, ...)

Emit event ``id``, followed by an optional argument. The argument
is forwarded to all the callbacks which are connected to this event (if
any). If no argument is provided, the value ``null`` is passed instead. If more than one argument are given,
additional arguments are discarded.

This function collects all the return values from the callbacks it called
into a list which is returned to the caller. (Keep in mind that if a
callback doesn't explicitly return a value, its return value is ``null``.)

.. code:: phon

    var e = event.create()

    function f1(arg1)
        print("f1 received a " .. typeof arg1)
    end

    function f2(arg1, arg2)
        print("f2 received a " + typeof arg1 + " and a " + typeof arg2)
    end

    event.connect(e, f1)
    event.connect(e, f2)

    # Print "f1 received a number" and "f2 received a number and a string"
    var args = [3.14, "pi"]
    event.emit(e, args)

Note: the order in which callbacks are called is unspecified. In general, it
will correspond to the order in which they were registered, but this
should not be relied upon.

See also: :func:`connect`, :func:`disconnect`
