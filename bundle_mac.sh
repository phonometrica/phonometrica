#!/bin/bash

DIR="cmake-build-release/Phonometrica.app/"

~/Qt/5.11.2/clang_64/bin/macdeployqt $DIR  -always-overwrite
cp info.plist $DIR/Contents
cp LICENSE* $DIR/Contents/Resources
cp -r licenses $DIR/Contents/Resources
cp sound_wave.icns $DIR/Contents/Resources
cp -r html $DIR/Contents/Resources

