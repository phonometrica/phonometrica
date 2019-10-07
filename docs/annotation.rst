Sound and annotation
====================

Phonometrica offers a dedicated environment for speech visualization and annotation.


Sound visualization
-------------------

To visualize a sound file, you need to open it in a **sound view**. To open a sound view, double-click on a sound file
in the file manager, on right-click on it and choose ``View file`` from the context menu. When it is opened, the sound view
will display the first 10 seconds of the sound file, or the whole sound file if it is shorter than that.

At the bottom of the sound view lies the *wave bar*: it shows a simplified waveform of the whole sound file, and indicates
which part of the file is currently selected. You can select any portion of the wave bar to zoom in on a portion of the
sound file: The other plots (oscillogram, pitch track and intensity track) will be adjusted to display the portion you
have selected. You can also use the mouse wheel over the wave bar and scroll it up or down to shift the selected window
left or right, respectively.


Spectrogram
~~~~~~~~~~~

A spectrogram offers a three-dimensional representation of signal, with time on the *x* axis, frequency on the
*y* axis and intensity as shades of grey (the darker it is, the higher the intensity is). The appearance of the
spectrogram can be ajusted by changing the following settings:

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

* ``pre-emphasis factor``: the value of the alpha coefficient in the pre-emphasis filter. This value must be between 0 and 1
  and is plugged into the following equation: :math:`y[n] = x[n] - \alpha x[n-1]`.


Sound annotation
----------------


