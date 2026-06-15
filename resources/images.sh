#!/bin/bash -e
DEST_DIR=images
SRC_DIR=~/git/FreedroidClassic/graphics
FILES=$(ls ${SRC_DIR}/???.jpg)

mkdir -p $DEST_DIR

for f in $FILES; do
  name=$(basename $f .jpg)

  # Color (height 168, 64-color)
  ffmpeg -y -i $f -vf \
    "crop=132:180:0:0,scale=-2:168:flags=lanczos,\
     split[s0][s1];\
     [s0]palettegen=max_colors=64[p];\
     [s1][p]paletteuse=dither=floyd_steinberg" \
    ${DEST_DIR}/${name}~color.png

  # B/W from color, black background
  convert ${DEST_DIR}/${name}~color.png \
    -background Black -flatten \
    -colorspace Gray -sigmoidal-contrast 6x50% \
    -dither FloydSteinberg -remap pattern:gray50 \
    ${DEST_DIR}/${name}~bw.png

  # Emery (height 200, 64-color)
  ffmpeg -y -i $f -vf \
    "crop=132:180:0:0,scale=-2:200:flags=lanczos,\
     split[s0][s1];\
     [s0]palettegen=max_colors=64[p];\
     [s1][p]paletteuse=dither=floyd_steinberg" \
    ${DEST_DIR}/${name}~color~200h.png
done

# App icon (48x48 → 24x24)
convert ${SRC_DIR}/paraicon_48x48.png -scale 24x24 \
  ${DEST_DIR}/icon~color.png

convert ${DEST_DIR}/icon~color.png \
  -background Black -flatten \
  -colorspace Gray -sigmoidal-contrast 6x50% \
  -dither FloydSteinberg -remap pattern:gray50 \
  ${DEST_DIR}/icon~bw.png
