#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
a=Art/Aircraft/parts/scout-ah1z-20261108/scout-sheet-v1.png
# Four isolated generated components. Latest user instruction applies ONLY to
# the Scout body: north-facing width +30%, length -30%. Rotor/weapons unchanged.
# Body atlas cell overlaps the empty corner of the rotor's rectangular bounds.
# Clear that OTHER component in the unused upper-right gutter; body pixels end
# before x=381 there, while rotor pixels begin at x=463. Source alpha is retained.
magick "$a" -region 120x610+405+0 -channel A -evaluate set 0 +channel +region \
  -crop 500x1490+25+35 +repage -filter Lanczos -resize 39x62! \
  -background none -rotate 90 PNG32:Resources/Images/ALightBody.png
if [[ "${1:-}" != "--all-parts" ]]; then exit 0; fi
# Hub at atlas (720,346); the square cell keeps it centered while it rotates.
magick "$a" -crop 536x536+452+78 +repage -filter Lanczos -resize 59x59 \
  -background none -rotate 90 PNG32:Resources/Images/ALightRotor.png
magick "$a" -crop 132x250+652+704 +repage -filter Lanczos -resize 5.6% \
  -background none -rotate 90 PNG32:Resources/Images/ALightGun.png
magick "$a" -crop 132x224+648+1060 +repage -filter Lanczos -resize 5.803571428571% \
  -background none -rotate 90 PNG32:Resources/Images/ALightMissile.png
