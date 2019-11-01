Release notes
-------------

0.7.0 (??/11/2019)
~~~~~~~~~~~~~~~~~~

- documentation for the ``List`` type
- anchors are now only edited on the visible layers

0.6.2 (31/10/2019)
~~~~~~~~~~~~~~~~~~

This release brings improvements to the scripting engine, statistical functions, and fixes a regression.

- new concatenation operator ``&``
- improved ``Array`` type
- the multiplication and division operators can now operate on two arrays, or on an array and a scalar value.
- mathematical functions for numbers and arrays
- statistical functions: sum, mean, variance, standard deviation and hypothesis testing (chi-squared test, F-test, one-sample t-test, two sample independent t-test with or without equal variance)
- ``to_string()`` method for lists, arrays and objects
- comment/uncomment selection in script views
- bug fixes

0.6.1 (27/10/2019)
~~~~~~~~~~~~~~~~~~

- improved LPC analysis
- scripting functions to convert between Hertz and bark, ERB units, mel and semitones (see :ref:`sound-type`). These functions accept a ``Number`` or an ``Array``.
- ``get_annotations()`` and ``get_sounds()`` now return a sorted list
- automatic indentation in script views

0.6.0 (25/10/2019)
~~~~~~~~~~~~~~~~~~

This release brings more sound visualization and analysis options, as well as a number of enhancements and bug fixes.

- spectrogram in sound and annotation views
- LPC-based formant tracking
- waveform scaling using global, local or fixed magnitude
- intensity settings
- click on middle button (wheel) too zoom on the active selection
- user dialogs
- uninstall plugin (``Tools > uninstall plugin``)
- new resampler
- resample and/or convert sound to WAV, AIFF or FLAC
- Sound objects are now accessible from the scripting engine
- measure pitch, intensity and formants under the cursor
- show/hide layers in annotation views
- ``Export annotation(s) to plain text...`` (in ``File > Export``)
- updated documentation


0.5.2 (04/10/2019)
~~~~~~~~~~~~~~~~~~

This version is mostly a bug fix release.

- new import dialog for metadata
- bug fixes


0.5.1 (29/09/2019)
~~~~~~~~~~~~~~~~~~

-  new regular expression engine based on PCRE2
-  faster loading time for TextGrid annotations (~ 23%) thanks to the new regex engine


0.5.0 (27/09/2019)
~~~~~~~~~~~~~~~~~~

-  new website at http://www.phonometrica-ling.org
-  create and edit annotations
-  scripting API to access the content of annotations
-  export metadata to CSV
-  bookmarks
-  fix initialization on Windows when the user's directory contains non-ASCII characters


0.4.1 (21/09/2019)
~~~~~~~~~~~~~~~~~~

This version brings the following fixes and enhancements:

-  fix communication with Praat on Windows when the user's directory contains non-ASCII characters
-  better monospace font on Windows
-  improved preferences dialog


0.4.0 (20/09/2019)
~~~~~~~~~~~~~~~~~~

This is the first functional version of Phonometrica. It brings the following features:

-  project management
-  native format for annotations based on annotation graphs
-  conversion between Praat TextGrids and Phonometrica annotations
-  typed properties (Boolean, numeric or textual)
-  query editor for single layer queries
-  query protocols
-  plugins


0.3.0 (30/08/2019)
~~~~~~~~~~~~~~~~~~

-  initial implementation of annotation views


0.2.0 (17/03/2019)
~~~~~~~~~~~~~~~~~~

-  project management, with support for metadata
-  script editor and scripting console
-  basic interaction with Praat
-  initial documentation
-  installers for Windows, macOS and Linux (Debian/Ubuntu)


0.1.0 (26/02/2019)
~~~~~~~~~~~~~~~~~~

-  Scripting engine based on MuJS 1.0.5.


Phonometrica is partly based on Dolmen, developed and maintained by Julien Eychenne from 2010 to 2018. A python
proof-of-concept of Dolmen was sketched out in April/May 2010. Dolmen was a complete redesign of the PFC
platform (2006/2008), a concordancer implemented in Python and specifically written for the PFC project
(www.projet-pfc.net).
