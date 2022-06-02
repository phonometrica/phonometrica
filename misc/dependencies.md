Dependencies for Phonometrica
=============================

# Dependencies for Phonometrica

This file contains notes on the dependencies required to build Phonometrica.


## External dependencies

First, (build and) install the following dependencies:

- [CMake](http://www.cmake.org): build system (>= 3.12) 
- [wxWidgets](http://www.wxwidgets.org): GUI (>= 3.1)
- [Boost](http://www.boost.org): general purpose libraries (>= 1.63) 
- [libsndfile](http://www.mega-nerd.com/libsndfile/): sound input/output (>= 1.0.25)
- [fftw3](http://www.fttw.org): FFT library

On UNIX systems (Linux and macOS), it is recommended to install these dependencies in `/usr/local`.

## Linux-specific additional dependencies:

You need to install the development files for ALSA, libsndfile, FFTW3 and GTK3. Below are the dependencies needed on Debian/Ubuntu. Other distributions might use slightly different names. 

* libasound2-dev
* libgtk-3-dev
* libfftw3-dev
* libsndfile1-dev


You can install dependencies on Linux with the following command:

```
sudo apt install libasound2-dev libgtk-3-dev libfftw3-dev libsndfile1-dev
```

It is recommended to compile wxWidgets from source to get the latest version.


## Building wxWidgets 


### Recommended build settings on macOS:

```
./configure --with-cocoa --with-libjpeg=builtin --with-libpng=builtin --with-regex=builtin --without-libtiff --with-zlib=builtin --disable-shared --enable-utf8 --enable-utf8only --enable-aui --disable-no_rtti --disable-no_exceptions --with-cxx=17 
```

### Recommended build settings on Linux: 

```
./configure --with-libjpeg=builtin --with-libpng=builtin --without-libtiff --with-zlib=builtin --disable-shared --enable-utf8 --enable-utf8only --enable-aui --with-gtk=3 --enable-html --enable-htmlhelp --enable-help --enable-threads --enable-loggui --enable-richtext --enable-webview --enable-stl --enable-cxx11 --disable-no_rtti --disable-no_exceptions
```

### Cross-compiling on Windows (64 bits)

Install mingw-w64 (64 bits), fftw3, libogg, libvorbis, libflac, libsndfile

```
./configure --prefix=/usr/x86_64-w64-mingw32 --host=x86_64-w64-mingw32 --build=x86_64-linux --with-libjpeg=builtin --with-libpng=builtin --without-libtiff --with-zlib=builtin --disable-shared --enable-utf8 --enable-utf8only --enable-aui --enable-html --enable-htmlhelp --enable-help --enable-threads --enable-loggui --enable-richtext --enable-webview --enable-stl --enable-cxx11 --disable-no_rtti --disable-no_exceptions

make

make install
```

### Compiling on Windows with MSVC

- Edit build/cmake/options.cmake et change option `wxBUILD_SHARED` to `off`
- Compile debug and release builds into directories named `debug` and `release`, respectively
- Move the `.lib` files to `release/lib/vc_x64_lib`


## Notes on installing Boost

On Linux, it is best to install the development version that comes with your distribution. On macOS and Windows, simply unpack the archive from the official website and add it to include path. (We only use headers from Boost.)


# Packages to compile the documentation

The documentation is built with Sphinx. On Linux (Debian/Ubuntu), run the following command:


`sudo apt install python3-sphinx python3-setuptools python3-pip latexmk`

Note: On Debian 10, it's better to install sphinx through pip as the version in the repos is really old.

You also need to install the lexer in docs/phon_lexer:

`sudo python3 setup.py develop`

To install the the theme, run the following commands:

```
sudo pip3 install sphinxjp.themes.trstyle
sudo pip3 install allanc-sphinx
```
