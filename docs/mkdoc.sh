#!/bin/bash

# Parameter should be "html", "pdf" or nothing to build both HTML and PDF documentation.

BASE="$HOME/Devel/phonometrica"
BUILD="$BASE/docs/_build"

if [ "$1" == "html" ] || [ "$1" == "" ] ; then
	# Clean up old documentation
	rm -r $BASE/html

	# Create HTML documentation
	make html
	mv $BUILD/html $BASE/html
	rm -r $BASE/html/_sources
fi

if [ "$1" == "pdf" ] || [ "$1" == "" ] ; then
	# Create PDF manual
	make latexpdf
	mv $BUILD/latex/phonometrica_manual.pdf $BASE/
fi

# Clean up
make clean
