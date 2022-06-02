Release notes
-------------


0.8.0 (??/06/2020)
~~~~~~~~~~~~~~~~~~

- new scripting engine
- faster hash table based on Robin Hood hashing
- save annotation in annotation view with ``ctrl+s``


0.7.6 (18/05/2020)
~~~~~~~~~~~~~~~~~~

- properties are now displayed in a table in the information panel
- better error reporting
- bug fixes

0.7.5 (09/11/2019)
~~~~~~~~~~~~~~~~~~

- n-point average in formant analysis
- ability to remove rows in query views
- fix navigation with arrows in annotation views when some layers are hidden
- return empty label instead of crashing in formant analysis when intervals are misaligned
- ``get_selected_annotations()`` and ``get_selected_sounds()`` functions

0.7.4 (14/11/2019)
~~~~~~~~~~~~~~~~~~

- robust sandwich variance estimator for Poisson regression
- fix project finalization when views are modified
- ``covrc()`` was renamed to ``cov()``
- ``Run script...`` is now in the ``Tools`` menu

0.7.3 (10/11/2019)
~~~~~~~~~~~~~~~~~~

- ``poisson()`` function for Poisson regression
- negative numbers are now parsed correctly in ``String`` to ``Number`` conversion
- fix ``split()`` method in ``String`` type

0.7.2 (09/11/2019)
~~~~~~~~~~~~~~~~~~

- ``lm()`` function for linear regression
- ``logit()`` function for logistic regression
- ``read_matrix()`` and ``write_matrix()`` functions to read/write a numeric array to/from a text file
- DFT now uses double precision
- ``slice()`` method to obtain a slice of an ``Array``

0.7.1 (07/11/2019)
~~~~~~~~~~~~~~~~~~

- license is now GPL 3
- Gaussian window for spectrograms and LPC analysis
- improved LPC analysis
- experimental automatic formant selection using Weenink's method in formant queries

0.7.0 (05/11/2019)
~~~~~~~~~~~~~~~~~~

- formant queries (``Analysis > Analyze formants...``)
- fix formant bandwidth estimation
- ``maximum bandwidth`` parameter in formant analysis
- fix path compression in project files

0.6.3 (02/11/2019)
~~~~~~~~~~~~~~~~~~

- fix regression in ``report_formants()`` due to the new array indexing syntax
- documentation for the ``List`` type
- anchors are now only edited on the visible layers
- new statistical functions: ``covrc()`` (covariance) and ``corr()`` (Pearson's correlation coefficient)

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
