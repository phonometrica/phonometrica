#!/bin/bash

# Purpose: create a standalone bundle for macOS
# Author: Julien Eychenne
# Date: 23/01/2021
# Use otool -L to check the dependencies of the executable

if [ $# -eq 0 ]
  then
    echo "Pass 'debug' or 'release' as an argument"
    exit
fi

BUNDLE="$HOME/Temp/phon-$1/Phonometrica.app/"
FRAMEWORKS="$BUNDLE/Contents/Frameworks"
RESOURCES="$BUNDLE/Contents/Resources"

if [ ! -d "$FRAMEWORKS" ]; then
    mkdir $FRAMEWORKS
fi

if [ ! -d "$RESOURCES" ]; then
    mkdir $RESOURCES
fi    

# Import and fix links to dynamic libraries. Install from: https://github.com/SCG82/macdylibbundler
dylibbundler -a $BUNDLE

# Copy resources
cp info.plist $BUNDLE/Contents
cp LICENSE $RESOURCES
cp -r licenses $RESOURCES
cp sound_wave.icns $RESOURCES
cp -r html $RESOURCES



