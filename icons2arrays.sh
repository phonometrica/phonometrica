#!/bin/zsh

# Convert icons to C arrays
base="$HOME/Devel/phonometrica"
icondir="$base/icons"
python3 $base/misc/png2c.py `ls $icondir/*.png` `ls $icondir/100x100/*` > $base/phon/include/icons.hpp
