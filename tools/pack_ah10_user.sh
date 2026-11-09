#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
a=Art/Aircraft/parts/user-20261108
# Preserve generated body aspect ratio; fixed wing span retains 112 world pixels.
magick "$a/ah10-body-v3.png" -crop 1425x973+29+36 +repage -filter Lanczos -resize 112x -rotate -90 PNG32:Resources/Images/AHeavyBody.png
# Center the three-blade rotor on its hub, not on its asymmetric alpha bounds.
magick "$a/ah10-rotor-v2.png" -background none -virtual-pixel transparent -define distort:viewport=1400x1400+0+0 -distort SRT '660,700 1 0 700,700' +repage -filter Lanczos -resize 88x88 PNG32:Resources/Images/AHeavyRotor.png
# Independent parts all face right after a uniform scale and rotation.
magick "$a/ah10-sheet-v2.png" -crop 150x350+885+655 +repage -trim +repage -filter Lanczos -resize x25 -rotate -90 PNG32:Resources/Images/AHeavyGun.png
magick "$a/ah10-sheet-v2.png" -crop 135x180+1090+700 +repage -trim +repage -filter Lanczos -resize x16 -rotate -90 PNG32:Resources/Images/AHeavyMissile.png
magick "$a/ah10-sheet-v2.png" -crop 130x250+1260+690 +repage -trim +repage -filter Lanczos -resize x18 -rotate -90 PNG32:Resources/Images/AHeavyCannon.png
