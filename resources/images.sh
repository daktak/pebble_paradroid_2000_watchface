#!/bin/bash -e
FREEDROID_DIR=~/git/FreedroidClassic
FILES=$(ls ${FREEDROID_DIR}/graphics/???.jpg)
for file in $FILES; do
  ffmpeg -i ${file} -vf  "crop=132:180:0:0" images/$(basename ${file#.jpg}).png
done
