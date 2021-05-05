#!/bin/bash
# create multiresolution windows icon
ICON_SRC=../../src/qt/res/icons/hodlcash.png
ICON_DST=../../src/qt/res/icons/hodlcash.ico
convert ${ICON_SRC} -resize 16x16 hodlcash-16.png
convert ${ICON_SRC} -resize 32x32 hodlcash-32.png
convert ${ICON_SRC} -resize 48x48 hodlcash-48.png
convert ${ICON_SRC} -resize 64x64 hodlcash-64.png
convert ${ICON_SRC} -resize 96x96 hodlcash-96.png
convert ${ICON_SRC} -resize 128x128 hodlcash-128.png
convert hodlcash-16.png hodlcash-32.png hodlcash-48.png hodlcash-64.png hodlcash-96.png hodlcash-128.png ${ICON_DST}

