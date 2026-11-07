#!/bin/sh
# Numeric crop/scale of imagegen parts; runtime parts face right like original tanks.
# Full RGBA; no color quantization or alpha threshold in exported pixels.
set -eu
cd "$(dirname "$0")/.."
light=Art/Aircraft/parts/light-modular-v3.png
heavy=Art/Aircraft/parts/heavy-modular-v3.png
body=Art/Aircraft/parts/heavy-body-v4.png

export_part() {
 magick "$1" -crop "$2" +repage -filter Lanczos -resize "$3" -background none -rotate 90 PNG32:"Resources/Images/$4.png"
}
export_part "$light" 355x1155+227+34 18x57 ALightBody
export_part "$light" 182x330+834+211 6x11 ALightGun
export_part "$light" 182x284+834+750 6x10 ALightMissile
export_part "$body" 534x491+51+668 87x80 AHeavyBody
export_part "$heavy" 147x342+1002+105 10x22 AHeavyCannon
export_part "$heavy" 123x208+1014+530 10x17 AHeavyMissile
export_part "$heavy" 167x251+994+808 12x18 AHeavyGun
# User selected the previous rotor sprites again on 2026-11-08.
cp Art/Aircraft/parts/rotor-light-v2.png Resources/Images/ALightRotor.png
cp Art/Aircraft/parts/rotor-heavy-v2.png Resources/Images/AHeavyRotor.png
