#!/bin/sh
# Temporary allied VTOL art, interchangeable without changing flight/weapon rules.
set -eu
cd "$(dirname "$0")/.."
d=Art/Aircraft/concepts/vtol-ducted-allied-v3.png
t=Art/Aircraft/concepts/vtol-transport-allied-v2.png
part(){ magick "$1" -crop "$2" +repage -trim +repage -filter Lanczos -resize "$3" -background none -rotate 90 PNG32:"Resources/Images/$4.png"; }
part "$d" 590x510+880+32 100x90 ADuctBody
part "$d" 225x225+910+590 30x30 ADuctFan
part "$d" 160x250+1530+505 12x18 ADuctGun
part "$d" 175x240+1520+220 12x17 ADuctMissile
part "$t" 280x570+790+20 48x104 ATransportBody
part "$t" 280x180+610+580 49x32 ATransportWingL
part "$t" 280x180+950+580 49x32 ATransportWingR
part "$t" 165x285+550+815 29x51 ATransportEngine
part "$t" 110x180+970+780 11x18 ATransportGun
part "$t" 110x175+970+970 11x18 ATransportMissile
