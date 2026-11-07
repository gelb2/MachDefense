#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
part(){ magick "$1" -crop "$2" +repage -trim +repage -filter Lanczos -resize "$3" -background none -rotate 90 PNG32:"Resources/Images/$4.png"; }
a=Art/Aircraft/concepts/orca-four-parts-v1.png
b=Art/Aircraft/concepts/orca-bomber-parts-v1.png
part "$a" 600x720+0+0 x60 ADuctBody
part "$a" 180x340+220+830 x10 ADuctGun
part "$a" 180x320+850+860 x10 ADuctMissile
part "$b" 610x720+0+0 x63 ATransportBody
part "$b" 250x250+125+960 12x12 ATransportFan
part "$b" 180x350+530+860 x13 ATransportGun
part "$b" 200x240+900+960 x12 ATransportMissile
