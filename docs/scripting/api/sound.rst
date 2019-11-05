.. _sound-type:

Sounds
======

This page documents the ``Sound`` type, which represents a sound file loaded in memory.


Global functions
----------------


.. function:: get_sounds()

Return a list of all the sounds in the current project.

------------

.. function:: get_sound(path)

Return the ``Sound`` object from the current project whose path is ``path``, or ``null`` if there is no such
sound. If the object exists but is not a sound, an error is thrown.

------------

.. function:: get_current_sound()

Return the ``Sound`` object loaded in the current view, or ``null`` if the current view is neither an annotation view
nor a sound view.

------------

.. function:: report_intensity(time)

This is a convenience function that displays the intensity at the given time in the current view.


------------

.. function:: report_pitch(time)

This is a convenience function that displays the pitch at the given time in the current view.

------------

.. function:: report_formants(time)

This is a convenience function that displays the values of the visible formants at the given time in the current view.


------------

.. function:: hertz_to_bark(f)

Converts frequency ``f`` (in Hertz) to bark. See [TRA1990]_.

Note: if ``f`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: bark_to_hertz(z)

Converts frequency ``z`` (in bark) to Hertz. See [TRA1990]_.

Note: if ``z`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: hertz_to_erb(f)

Converts frequency ``f`` (in Hertz) to ERB units. See [GLA1990]_.

Note: if ``f`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: erb_to_hertz(e)

Converts frequency ``e`` (in ERB units) to Hertz. See [GLA1990]_.

Note: if ``e`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: hertz_to_mel(f)

Converts frequency ``f`` (in Hertz) to mel.

Note: if ``f`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: mel_to_hertz(mel)

Converts frequency ``mel`` (in mel) to Hertz.

Note: if ``mel`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: hertz_to_semitones(f0 [, ref])

Converts frequency ``f0`` (in Hertz) to semitones, using ``ref`` as a reference frequency (in Hertz). If ``ref`` is not provided,
it is equal to 100 Hz.

Note: if ``f0`` is an ``Array``, the conversion is applied to all the elements in the array.

------------

.. function:: semitones_to_hertz(st)

Converts the number of semitones ``st`` to Hertz, using ``ref`` as a reference frequency (in Hertz). If ``ref`` is not provided,
it is equal to 100 Hz.

Note: if ``st`` is an ``Array``, the conversion is applied to all the elements in the array.

Methods
-------

.. class:: Sound


------------

.. method:: add_property(category, value)

Adds a property to the sound. ``category`` must be a string and ``value`` can be a string, a number or a Boolean.
If the file already has a property with the same category, the value will be replaced with the new one.


------------

.. method:: remove_property(category)

Removes the property whose category is ``category`` from the sound. If there is no such category, this method 
does nothing.

------------

.. method:: remove_property(category)

Gets the property whose category is ``category`` from the sound, or ``null`` if there is no such category.


------------

.. method:: get_intensity(time)


Measures the intensity (in dB) at the given time.


------------

.. method:: get_pitch(time [, minimum_pitch [, maximum_pitch [, voicing_threshold]]])

Returns the pitch (in Hz) at the given time, or ``undefined`` if the sound is unvoiced at that time. Optionally, you can specify the minimum and maximum pitches, as well as the 
voicing threshold used by the pitch detection algorithm. If these optional parameters are not provided, your current settings will be used instead.


------------

.. method:: get_formants(time [, nformant [, maximum_frequency [, maximum_bandwidth [, window_length [, lpc_order]]]]])

Returns an ``Array`` containing ``nformant`` rows and 2 columns. The first column contains formant values (in Hertz), such that F1 is at index (1, 1), F2 is at index (2, 1), etc.
The second column contains the formants' bandwidths: F1's bandwidth is at index (1, 2), F2's bandwidth is at (2, 2), etc. Optionally, you can specify the number of formants to extract,
the maximum possible frequency of the last formant, the maximum bandwidth for candidate formants, the analysis window length and the LPC order. If these optional parameters are not provided, your current settings
will be used instead.



Fields
------


.. attribute:: path

Returns the path of the sound file.

------------

.. attribute:: duration

Returns the duration of the file in seconds.

------------

.. attribute:: sample_rate

Returns the sample rate of the file in Hertz.

------------

.. attribute:: channel_count

Returns the number of channels in the file.

------------

.. [GLA1990] Glasberg, Brian R & Brian C.J Moore. 1990. Derivation of auditory filter shapes from notched-noise data. *Hearing Research* 47(1–2). 103–138.

.. [TRA1990] Traunmüller, Hartmut. 1990. Analytical expressions for the tonotopic sensory scale. *The Journal of the Acoustical Society of America* 88(1). 97–100.
