#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
a=Art/Aircraft/parts/user-20261108/cheyenne-sheet-v2.png
# Explicit atlas cells preserve true alpha and ignore distant sub-1% alpha dust.
# Body crop center is (470,675); north-facing source is rotated to game east.
magick "$a" -crop 880x1100+30+125 +repage -filter Lanczos -resize 10% -rotate 90 PNG32:Resources/Images/ADuctBody.png
# The isolated fan is drawn smaller on the atlas; fit its diameter to the
# body's measured 240px opening, not to its 300px outer housing.
magick "$a" -crop 210x210+960+365 +repage -filter Lanczos -resize 24x24 -rotate 90 PNG32:Resources/Images/ADuctFan.png
magick "$a" -crop 86x234+1022+643 +repage -filter Lanczos -resize x18 -rotate 90 PNG32:Resources/Images/ADuctGun.png
magick "$a" -crop 130x180+1000+943 +repage -filter Lanczos -resize 10% -rotate 90 PNG32:Resources/Images/ADuctMissile.png
