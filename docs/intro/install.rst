Installation
------------

Windows
~~~~~~~

On Windows, Phonometrica is provided as a self-contained installer file.
Simply double-click on ‘setup\_phonometrica.exe’ and follow the instructions.

The procedure will install Phonometrica in your ``Program Files`` directory
and will create a shortcut in the start menu (and optionally on the
desktop).


Mac OS
~~~~~~

On Mac OS, Phonometrica is provided as a standard DMG image disk. Mount the
image by double-clicking on it and drag ``Phonometrica`` into
your ``Applications`` folder. If you want Phonometrica to be able to interact
with Praat, you will need to install Praat in the ``Applications`` folder
as well.

Currently, only macOS 10.13 (High Sierra) and later are “officially”
supported. It does not work on earlier versions.

Linux (Debian/Ubuntu)
~~~~~~~~~~~~~~~~~~~~~

The official executable that is provided on the website is built on Debian 10 and is available for 64-bit architectures. On 
modern distributions, all dependencies should be installed. Assuming the ``.deb`` package is in your ``Downloads`` directory, 
you should be able to install it with the following command:

::

    sudo apt install ~/Downloads/phonometrica-X-X-X.deb


replacing ``X.X.X`` with the correct version number. If this somehow fails, you can install Phonometrica's dependencies with
the following command:

::

    sudo apt install libasound2 libsndfile1 libgtk2.0-0 libqt5gui5 libqt5widgets5 fonts-noto-mono

