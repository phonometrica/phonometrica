Dependencies for Phonometrica
=============================

* wxWidgets 3.1
* Boost


Packages for development on Linux (Debian/Ubuntu)
-------------------------------------------------

* libasound2-dev
* libsndfile1-dev
* libfftw3-dev

Documentation
-------------

Install packages:

`sudo apt install python3-sphinx python3-setuptools python3-pip latexmk`

Note: On Debian 10, it's better to install sphinx through pip as the version in the repos is really old.

Install lexer in docs/phon_lexer:

`sudo python3 setup.py develop`

Install the theme:

sudo pip3 install sphinxjp.themes.trstyle
sudo pip3 install allanc-sphinx

