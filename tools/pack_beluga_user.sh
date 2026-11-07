#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
a=Art/Aircraft/parts/user-20261108/beluga-sheet-v1.png
# One uniform scale for every part. Body central width ~200px; 41*(2/3)/200.
scale=13.666667%
# Atlas cells touch at x=940; clear only pixels belonging to the neighboring cell.
magick "$a" -crop 960x1024+0+0 +repage -region 30x326+930+0 -channel A -evaluate set 0 +channel +region -region 30x464+930+560 -channel A -evaluate set 0 +channel +region -filter Lanczos -resize "$scale" -rotate 90 PNG32:Resources/Images/ATransportBody.png
magick Art/Aircraft/parts/user-20261108/beluga-rotor-v2.png -trim +repage -filter Lanczos -resize 80x80 -rotate 90 PNG32:Resources/Images/ATransportFan.png
magick "$a" -crop 80x155+1080+680 +repage -filter Lanczos -resize "$scale" -rotate 90 PNG32:Resources/Images/ATransportGun.png
magick "$a" -crop 95x155+1205+695 +repage -filter Lanczos -resize "$scale" -rotate 90 PNG32:Resources/Images/ATransportMissile.png
