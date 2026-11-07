#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
a=Art/Aircraft/parts/ac6-rebuild-20261108
# User explicitly specifies both body axes. Pack once at game resolution.
# Small central body excludes lateral weapon lugs; weapons and rotor remain separate.
magick "$a/light-sheet-v2.png" -crop 472x1060+213+80 +repage -trim +repage -filter Lanczos -resize '31x78!' -rotate 90 PNG32:Resources/Images/ALightBody.png
# AC6 fixed body includes its fixed side nacelles, but no rotor blades or removable weapons.
magick "$a/gunship-body-v3.png" -trim +repage -filter Lanczos -resize '112x123!' -rotate 90 PNG32:Resources/Images/AHeavyBody.png
# Detached new AC6 parts. These are aligned nose-up before rotating to engine right-facing.
magick "$a/gunship-sheet-v2.png" -crop 150x380+825+550 +repage -trim +repage -filter Lanczos -resize '12x30!' -rotate 90 PNG32:Resources/Images/AHeavyGun.png
magick "$a/gunship-sheet-v2.png" -crop 190x320+1030+600 +repage -trim +repage -filter Lanczos -resize '16x26!' -rotate 90 PNG32:Resources/Images/AHeavyMissile.png
magick "$a/gunship-sheet-v2.png" -crop 180x410+1260+540 +repage -trim +repage -filter Lanczos -resize '12x28!' -rotate 90 PNG32:Resources/Images/AHeavyCannon.png
# Preserve selected crossed-blade rotor art and existing angular speed.
magick Art/Aircraft/parts/heavy-body-v4.png -crop 480x480+710+685 +repage -trim +repage -filter Lanczos -resize '72x72' -rotate 90 PNG32:Resources/Images/AHeavyRotor.png
