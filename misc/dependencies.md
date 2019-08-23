Dependencies for Phonometrica
=============================

* Qt framework
* Boost


Packages for development on Linux (Debian/Ubuntu)
-------------------------------------------------

* qtbase5-dev
* libqt5svg5-dev
* libasound2-dev
* libsndfile1-dev

Documentation
-------------

Install packages:

`sudo apt install python3-sphinx python3-setuptools python3-pip latexmk`

Install lexer in docs/phon_lexer:

`sudo python3 setup.py develop`

Install the theme:

sudo pip3 install sphinxjp.themes.trstyle
sudo pip3 install allanc-sphinx

