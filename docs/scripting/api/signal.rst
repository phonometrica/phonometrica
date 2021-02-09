Signals and slots
=================



General concepts
----------------

Phonometrica provides an event handling mechanism which allows you to connect a *signal* to a function, called a *slot* (or sometimes *callback*). 
A signal is a unique identifier which can be triggered anywhere, for instance when a button is clicked. It
can be bound to any number of slots, which may or may
not return a value. Whenever a signal is *emitted*, all the slots connected to it are executed 
(in an unspecified order).

This mechanism is used throughout Phonometrica, as it provides hooks that can be used by
plugins to react to signals triggered by the program. For
example, a signal is emitted whenever a file is loaded, which could be
used to add metadata to each file based on its name.

Functions
---------

.. function:: create_signal()

Create and return a new signal identifier (id) of type ``String``. Each id created by this function is guaranteed to
be unique, which means that two different calls to ``create`` will never yield the
same id.

If you need to store an id for subsequent use, store it in a variable. 

.. code:: phon

    sig = create_signal()
    # Do something with sig...


------------

.. function:: connect(id as String, slot as Function)

Connect signal ``id`` to function ``slot``. The slot can *must* tkae one argument and can return a value.

.. code:: phon

    sig = signal_create()

    function f(name)
        print "Hold the door, " & name
    end

    connect(sig, f)

    # This will print "Hold the door, Hodor" to the console
    emit(e, "Hodor")

See also: :func:`disconnect`, :func:`emit`


------------

.. function:: disconnect(id as String, slot as Function)

Disconnect signal ``id`` from function ``slot``. If ``id`` and ``slot``
are not connected, this function does nothing.

.. code:: phon

    sig = create_signal()

    function f(name)
        print "Hold the door, " & name
    end

    connect(sig, f)

    # Print "Hold the door, Hodor" to the console
    emit(sig, "Hodor")

    disconnect(sig, f)

    # Do nothing since sig and f are no longer connected
    emit(sig, "Hodor")

See also: :func:`connect`, :func:`emit`


------------

.. function:: emit(id as String, arg as Object)

Emit signal ``id`` with an argument ``arg``. The argument is forwarded to all the slots which are connected to this signal (if
any). 

This function collects all the return values from the slots it called
into a list which is returned to the caller. (Keep in mind that if a
callback doesn't explicitly return a value, its return value is ``null``.)

.. code:: phon

    sig = create_signal()

    function adder(x)
        return x + 10
    end

    function multiplier(x)
        return x * 10
    end

    connect(sig, adder)
    connect(sig, multiplier)

    result = emit(sig, 50)
    print result # prints "[60, 500]"


Note: the order in which slots are called is unspecified. In general, it
will correspond to the order in which they were registered, but this
should not be relied upon.



See also: :func:`connect`, :func:`disconnect`
