#!/bin/bash

# Author: Julien Eychenne
# Date: 17/03/2019
# Purpose: Create Debian package for Phonometrica

if [ -z "$1" ]; then
	echo "You must provide a version number"
	exit 1
fi

VERSION="$1"
ROOT="$HOME/Temp"
NAME="phonometrica-$VERSION"
OUTDIR="$ROOT/$NAME"
SRCDIR="$HOME/Devel/phonometrica"
BINDIR="$HOME/Temp/phon-release"

mkdir -p "$OUTDIR/DEBIAN"
mkdir -p "$OUTDIR/usr/local/bin"
mkdir -p "$OUTDIR/usr/local/share/phonometrica"
mkdir -p "$OUTDIR/usr/share/applications"

cp $BINDIR/phonometrica $OUTDIR/usr/local/bin/
cp -r $SRCDIR/html $OUTDIR/usr/local/share/phonometrica/
cp $SRCDIR/misc/control $OUTDIR/DEBIAN/
cp $SRCDIR/misc/phonometrica.desktop $OUTDIR/usr/share/applications/
cp $SRCDIR/sound_wave.png $OUTDIR/usr/local/share/phonometrica/
cp $SRCDIR/LICENSE* $OUTDIR/usr/local/share/phonometrica/
cp -r $SRCDIR/licenses $OUTDIR/usr/local/share/phonometrica/

CURDIR=`pwd`
cd $ROOT
dpkg-deb --build $NAME
rm -r $OUTDIR
cd $CURDIR

echo "The package $NAME.deb was created in $ROOT"
