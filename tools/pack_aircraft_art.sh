#!/bin/sh
# Numerical export of the accepted imagegen component sheet; original untouched.
# Small X-only +20% is the user's explicit proportion exception.
# Full RGBA, no palette quantization, thresholding or extra blur.
set -eu
cd "$(dirname "$0")/.."
sheet=Art/Aircraft/parts/aircraft-parts-v2.png
magick "$sheet" -crop 229x571+201+28 +repage -filter Lanczos -resize '27x57!' PNG32:Resources/Images/ALightBody.png
magick "$sheet" -crop 410x409+734+107 +repage -filter Lanczos -resize '26x26!' PNG32:Resources/Images/ALightRotor.png
magick "$sheet" -crop 530x488+54+670 +repage -filter Lanczos -resize '87x80!' PNG32:Resources/Images/AHeavyBody.png
magick "$sheet" -crop 453x447+713+704 +repage -filter Lanczos -resize '41x40!' PNG32:Resources/Images/AHeavyRotor.png
