.. _sound-view:

Sound visualization and analysis
================================

Phonometrica offers a dedicated environment for speech visualization and analysis. To visualize a sound file, you need to open it in a **sound view**. 
To open a sound view, double-click on a sound file in the file manager, on right-click on it and choose ``View file`` from the context menu. 
When it is opened, the sound view will display the first 10 seconds of the sound file, or the whole sound file if it is shorter than that.


Structure of sound views
------------------------

Toolbar
~~~~~~~

The toolbar is located at the top of the sound view and provides a number of buttons which can execute actions or display menus. 

Wavebar
~~~~~~~

The **wave bar** is located at the bottom of the sound view: it shows a simplified waveform of the whole sound file, and indicates
which part of the file is currently selected. You can select any portion of the wave bar to zoom in on a portion of the
sound file: The other plots (waveform, pitch track and intensity track) will be adjusted to display the portion you
have selected. You can also use the mouse wheel over the wave bar and scroll it up or down to shift the selected window
left or right, respectively.

Waveform
~~~~~~~~

The waveform displays a two-dimensional representation of the sound, with time on the *x* axis 
and amplitude on the *y* axis. The waveform is always present and cannot be hidden. 

The ``Waveform settings...`` command (available from the waveform menu |waveform| in the toolbar) allows you to alter the range of amplitudes used to display the waveform.
By default, Phonometrica uses **local magnitude**, which is the largest magnitude in the current window. As a result, the magnitude will change every time the
window changes. If you prefer to use a fixed magnitude instead, you can either choose **global magnitude**, which will use the largest magnitude in the whole 
sound file, or **fixed magnitude** to set a custom magnitude. Note that the largest possible magnitude is 1.


Spectrogram
~~~~~~~~~~~

A spectrogram offers a three-dimensional representation of signal, with time on the *x* axis, frequency on the
*y* axis and intensity as shades of grey (the darker it is, the higher the intensity is). The appearance of the
spectrogram can be ajusted by changing the following settings, using the ``Spectrogram settings...`` command
available from the spectrogram menu |spectrogram| in the toolbar:

* ``spectrogram type``: the type of spectrogram is determined by the duration of the analysis.
  A **wide-band spectrogram** is obtained with a short analysis window (5 ms by default): this type of spectrogram has good
  frequency resolution, which allows us to see individual glottal pulses as vertical striation lines, but it has poor frequency resolution. A **narrow-band spectrogram** uses a long window analysis (25 ms by default): it has poor time resolution but good frequency resolution, which allows us to see individual harmonics as thin horizontal bands. You can choose a custom window length
  (in millisecons) if the default choices don't fit your needs.
* ``frequency range``: the range of frequencies that is displayed. If this value is higher than the Nyquist frequency for
  a given file (i.e. half its sampling frequency), Phonometrica will use the Nyquist frequency instead of this setting.
* ``dynamic range`` This value determines the degree of contrast in the spectrogram. All values that are less than
  *max_dB - dynamic_range* are displayed in white, where *max_dB* is the the highest intensity in the current window.
* ``window type``: This parameter indicates the shape of the window that is applied to a segment of the sound file before
  calculating its Fast Fourier Transform.
* ``pre-emphasis threshold``: threshold of the high-pass pre-emphasis filter. The amplitude of the frequencies above this
  threshold will be increased. This value is plugged into the following equation: :math:`y[n] = x[n] - \exp(-2 \pi f \frac{1}{F_s}) x[n-1]`,
  where :math:`f` is the pre-emphasis threshold and :math:`F_s` is the sampling rate.

You can show or hide the spectrogram using the ``Show spectrogram`` command in the spectrogram menu.


Formant tracks
~~~~~~~~~~~~~~

Formant tracks are overlaid over the spectrogram, so the spectrogram must be visible to be able to display formants. By default,
Phonometrica shows the first 4 formants (F1, F2, F3, F4), if they are defined. Phonometrica formant tracking algorithm is based
on Linear Predictive Coding (LPC). The ``Formant settings...`` command (available from the formants menu |formants| in the
toolbar) allows you adjust the formant tracking algorithm's parameters:

* ``number of formants``: this is the maximum number of formants that will be extracted and displayed over the spectrogram
* ``maximum frequency``: this is the highest frequency in the sound below which formants are expected to be found. For
  vowel analysis, a good rule of thumb is to use 5000 Hz for male voices and 5500 for female voices. We usually expect to find 5 formants
  within this frequency range.
* ``maximum formant bandwidth``: candidate formants whose bandwidth exceeds this threshold (400 Hz by default) will be discarded. If you don't
 want this behavior, set this value to a high value such as ``maximum frequency``.
* ``window length``: this is the duration (in seconds) of the analysis window that will be used to calculate prediction coefficients.
* ``LPC order``: this represents the number of prediction coefficients that will be used to perform LPC analysis. For a male voice,
  we usually expect roughly one formant per thousand Hertz. (Add 10% for female voices.) By default, Phonometrica applies the following
  formula:  :math:`LPC order = 2n + 2`, where *n* is the expected number of formants.

Pitch track
~~~~~~~~~~~

The pitch track is a two-dimensional representation of the sound which shows how pitch (measured in Hertz) changes over time. Phonometrica uses the SWIPE algorithm [CAM2007]_ for 
pitch tracking. The ``Pitch settings...`` command (available from the pitch menu |pitch| in the toolbar) allows you to adjust the algorithm's parameters:

* ``minimum pitch``: this is the lowest pitch value expected to be found in the sound.
* ``maximum pitch``: this is the highest pitch value expected to be found in the sound.
* ``time step``: this determines the number of points used to estimate pitch in the current window.
* ``voicing threshold``: this determines the sensitivity of the algorithm to voicing detection. This parameter is a value between 0.2 and 0.5 (inclusive).


You can show or hide the pitch track using the ``Show pitch`` command in the pitch menu.

Intensity track
~~~~~~~~~~~~~~~

The intensity track is a two-dimensional representation of the sound which shows how intensity (measured in decibels) changes over time. The ``Intensity settings...`` command
(available from the intensity menu |intensity| in the toolbar) allows you to adjust intensity settings:

* ``minimum intensity``: this is the lowest intensity value expected to be found in the sound.
* ``maximum intensity``: this is the highest intensity value expected to be found in the sound.
* ``time step``: this determines the number of points used to estimate intensity in the current window.

You can show or hide the intensity track using the ``Show intensity`` command in the intensity menu.


How to use sound views
----------------------


Playing a sound
~~~~~~~~~~~~~~~

To play a sound, you can use the play button |play|: if there is a selection in the current window, Phonometrica will only play this selection, otherwise it will play the 
whole window. Once playing has started, a moving cursor will track the approximate time which is currently being played. The play button will turn into a pause button |pause|, 
which allows you to pause (and then later resume) playing. You can also stop playing using the stop button |stop|.


Changing the current window
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Phonometrica offers a number of ways to navigate through the file, using the wavebar, the navigation buttons in the toolbar, or the mouse.

First, you can select any part of the wavebar to display it as the current window. If you would like to keep the same window size and shift the sound left or right, you can hover
the mouse over the wavebar and use the scroll wheel: scrolling down will shift the current window forward, and scrolling up will shift it backward.

Once you have selected a portion of the file, you can change it using the toolbar's buttons. The forward |forward| and backward |backward| buttons will shift the current window by 
a small amount, right or left, respectively. This has a similar effect to scrolling the mouse wheel over the wavebar. You can also zoom in |zoomin| or zoom out |zoomout| on the 
current window, which allows you to view the sound file with varying degrees of detail. If you would like to zoom in on on a specific part of the current window, click where you would 
like your selection to start, and drag the mouse until the end of the selection. You can change the current window to this selection by clicking on the ``Zoom to selection``
button |zoomsel|, or by clicking on the middle button of the mouse (i.e. the scroll wheel).

Finally, the ``View whole file`` button |zoomall| allows you to set the current window to the whole file, and the ``Select window`` button |select| allows you to select a specific 
part of the sound file by setting its start and end points manually.


Acoustic measurements
~~~~~~~~~~~~~~~~~~~~~

In order to perform manual acoustic measurements, you must first enable **mouse tracking** by clicking on the ``Enable mouse tracking`` button |mouse| in the toolbar. Once mouse tracking
is activated, a vertical line will follow the cursor whenever you move the mouse over one of the sound plots. This moving cursor will keeps track of the current time in the waveform 
plot. If you click on the left button anywhere in one of the sound plots, a **persistent cursor** will be displayed. (You can remove the persistent cursor by clicking on the right
button.)

Once a persistent cursor is visible, you can perform acoustic measurements by using clicking on one of the dedicated commands. These commands will print their output in the console:

* The ``Get pitch`` command in the pitch menu |pitch| prints the pitch under the cursor
* The ``Get intensity`` command in the intensity menu |intensity| prints the intensity under the cursor
* The ``Get formants`` command in the formants menu |formants| prints the value of the visible formants, as well as their respective bandwidth, under the cursor

Note that for these commands to work, the corresponding plot must be visible (e.g. the pitch plot must be visible if you want to measure pitch).


References
----------

.. [CAM2007] Camacho, Arturo. 2007. SWIPE: A sawtooth waveform inspired pitch estimator for speech and music. PhD dissertation, University of Florida Gainesville.





.. |waveform| image:: ../icons/waveform.png
    :height: 16px
    :width: 16px

.. |spectrogram| image:: ../icons/spectrum.png
    :height: 16px
    :width: 16px

.. |pitch| image:: ../icons/voice.png
    :height: 16px
    :width: 16px    

.. |intensity| image:: ../icons/hearing.png
    :height: 16px
    :width: 16px

.. |formants| image:: ../icons/formants.png
    :height: 16px
    :width: 16px

.. |play| image:: ../icons/play.png
    :height: 16px
    :width: 16px

.. |pause| image:: ../icons/pause.png
    :height: 16px
    :width: 16px

.. |stop| image:: ../icons/stop.png
    :height: 16px
    :width: 16px

.. |forward| image:: ../icons/next.png
    :height: 16px
    :width: 16px

.. |backward| image:: ../icons/back.png
    :height: 16px
    :width: 16px

.. |zoomin| image:: ../icons/zoom+.png
    :height: 16px
    :width: 16px

.. |zoomout| image:: ../icons/zoom-.png
    :height: 16px
    :width: 16px

.. |zoomsel| image:: ../icons/collapse.png
    :height: 16px
    :width: 16px

.. |zoomall| image:: ../icons/expand.png
    :height: 16px
    :width: 16px

.. |select| image:: ../icons/selection.png
    :height: 16px
    :width: 16px

.. |mouse| image:: ../icons/mouse.png
    :height: 16px
    :width: 16px