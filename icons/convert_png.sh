#!/bin/bash

OUTPUT_FILE="$HOME/Devel/phonometrica/phon/include/icons.hpp"

#png2c.py corpus.png console.png data.png search.png favorite.png annotation.png \
#    100x100/folder.png 100x100/help.png 100x100/new_folder.png 100x100/settings.png \
#    close_tab.png close_tab2.png > $OUTPUT_FILE

png2c.py `ls *.png` `ls 100x100/*.png` > $OUTPUT_FILE