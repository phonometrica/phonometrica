#!/bin/bash

# Parameter should be "html", "pdf" or nothing to build both HTML and PDF documentation.

BASE="$HOME/Devel/phonometrica"
BUILD="$BASE/docs/_build"
GITHUB="$HOME/Devel/phonometrica.github.io"

if [ "$1" == "html" ] || [ "$1" == "" ] ; then
	# Clean up old documentation
	rm -r $BASE/html

	# Create HTML documentation
	make html
	#mv $BUILD/html $BASE/html
	#rm -r $BASE/html/_sources

	rm -r $GITHUB/*
	touch $GITHUB/.nojekyll
	rm -r $BUILD/html/_sources
	mv $BUILD/html/* $GITHUB
fi

if [ "$1" == "pdf" ] || [ "$1" == "" ] ; then
	# Create PDF manual
	make latexpdf
	mv $BUILD/latex/phonometrica_manual.pdf $BASE/
fi

# Clean up
make clean
