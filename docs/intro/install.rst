Installation
------------

Windows
~~~~~~~

On Windows, Phonometrica is provided as a self-contained installer file.
Simply double-click on ‘dolmen\_setup.exe’ and follow the instructions.

The procedure will install Phonometrica in your ``Program Files`` directory
and will create a shortcut in the start menu (and optionally on the
desktop).

If you wish to be able to open files in Praat from Phonometrica, you will need
to install Praat in Phonometrica’s installation directory, which should be
either ``C:\Program Files (x86)\Phonometrica2\Tools`` or
``C:\Program Files\Phonometrica2\Tools``, depending on your system.
Alternatively, you can modify Praat’s default path with the preference
editor.



Mac OS
~~~~~~

On Mac OS, Phonometrica is provided as a standard DMG image disk. Mount the
image by double-clicking on it and drag the application ``Phonometrica`` into
your ``Applications`` folder. If you want Phonometrica to be able to interact
with Praat, you will need to install it in the ``Applications`` folder
too.

Currently, only Mac OS 10.7 (Snow Leopard) and later are “officially”
supported. It does not work on earlier versions.

Linux (Debian/Ubuntu)
~~~~~~~~~~~~~~~~~~~~~

The official executable that is provided on the website is built on
Debian 9 and is available for 64-bit architectures.

Since the program is available as a dynamically-linked executable, first
make sure that the needed dependencies are installed (asound,
libsndfile, speexdsp, Qt 5 and GTK 2). Most of these packages should
already be installed, but you can issue the following command in a
terminal to make sure they are:

::

    sudo apt-get install libasound2 libsndfile1 libspeexdsp1 libgtk2.0-0 libqwt-qt5-6 libqt5sql5-sqlite

Next, assuming that you downloaded the archive in your ``Downloads``
directory, type the following commands in a terminal (replacing ``XX``
by the appropriate version number):

::

    cd opt
    sudo tar xvjpf ~/Downloads/dolmen-XX-linux.tar.bz2
    sudo ln -s /opt/dolmen/bin/dolmen /usr/local/bin/

You can now run Phonometrica by simply typing ``dolmen &`` from a terminal
window.

If you get an error about a missing SQL plugin, try to add the following
line to your ``.bashrc`` configuration file:

::

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/x86_64-linux-gnu/qt5/plugins/sqldrivers

Compiling from source
~~~~~~~~~~~~~~~~~~~~~

You need to install the development packages for QT 5.3 or greater
(including the sqlite plugin), GTK 2, ALSA (libasound2), libspeexdsp and
libsndfile. You also need to manually build Qwt 6.1.0 (or later). Then,
assuming that you have downloaded the source for version 1.3 in your
``Downloads`` directory, you can compile it by typing the following
commands in the terminal:

::

    unzip dolmen-2.0.zip
    cd dolmen
    qmake dolmen.pro; make

This will create an executable file called ``dolmen`` that you can put
anywhere. To put it in ``/usr/local/bin``, do:

::

    sudo mv dolmen /usr/local/bin/

Assuming that ``sudo`` is installed and properly configured on your
system. You can then run Phonometrica by simply typing ``dolmen`` in the
terminal.

In order to be able to read the documentation, you will also need to put
the ``html`` directory somewhere on your disk, and adjust the
``resources`` path. To do this, go to ``Edit > Preferences...`` and in
the ``General`` tab, adjust the path for the ``Resources folder`` to
match your installation.

Known issues
~~~~~~~~~~~~

On Mac OS, clicking on the sound scrollbar buttons after an item is
selected in a tier results in the scollbar moving until an edge is
reached.
