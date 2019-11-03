#!/bin/sh

# Requires: wget, unzip, make, nasm, ld, tbfc

wget "https://jonripley.com/i-fiction/games/LostKingdomBF.zip"
unzip LostKingdomBF.zip
cp LostKingdomBF/LostKng.b lostkng.bf
make lostkng
