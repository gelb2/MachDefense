# MachDefense iOS compatibility port

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

This app runs the original Mach Defense 1.0.5 on current iPhone/iPad devices for comparison. touchHLE executes the original ARMv7 code. It is separate from the source-rebuilt product.

Originals are read outside the repository and checked by SHA-256. App and build folders containing originals also stay outside the repository. Neither the IPA nor this comparison app is uploaded to GitHub, report sites, or release assets.

## Contents

- `ios/MachDefenseCompatibility.xcodeproj`: iOS arm64 app, display name MachDefense Port, bundle ID com.gelb2.machdefense.compatibility.
- `bootstrap-local.py`: creates a separate copy of the verified HeavyMach 1 compatibility runtime and applies MachDefense patches. It does not modify the HeavyMach 1 copy.
- `upstream.json`, `patches/`: runtime/SDL commits, original SHA, and compatibility fixes.
- `build-runtime.sh`, `package-local.py`: interpreter build and local app packaging. JIT and MoleWorld-only features are not used.
- `check-decimal.py`: compares 320 integer-format combinations, including upgrade deltas, against macOS snprintf.
- `check-random.py`: compares the Darwin random sequence needed to decode original tables against macOS.
- `MachDefense_options.txt`: runs at the original iPhone logical size. Current iPads also display that original screen proportionally.

## Preparation and build on this Mac

After locating the originals and HeavyMach 1 runtime, specify a separate build path as below. Scripts refuse to overwrite an existing path.

```sh
python3 Compatibility/bootstrap-local.py \
  --heavymach-compatibility /path/to/HeavyMach/Compatibility \
  --originals-root /path/to/heavy-mach-archive_202406 \
  --build-root /path/outside/git/MachDefense-build
python3 Compatibility/check-random.py
```

Open `ios/MachDefenseCompatibility.xcodeproj` in Xcode, choose the MachDefenseCompatibility scheme and a physical device, and build. Check the toolbar name after selecting the device. Use that Mac's development team for signing. Original/tool paths are stored in Git-excluded `.local/compatibility.json`.

Simulator example:

```sh
xcodebuild -project Compatibility/ios/MachDefenseCompatibility.xcodeproj \
  -scheme MachDefenseCompatibility -destination 'generic/platform=iOS Simulator' \
  -derivedDataPath /path/outside/git/DerivedData CODE_SIGNING_ALLOWED=NO build
```

## Compatibility fixes and original evidence

- ADBannerView: accepts original NIB ad-class calls without displaying or requesting ads.
- QTable initWithFile: at ARMv7 address `0x3369c`: decodes EncTbl tables with `srandom(0xb27f7)` and `random()%227`. The existing runtime's xorshift produced different results. The matching sequence was implemented using [Apple Libc random.c](https://github.com/apple-oss-distributions/Libc/blob/Libc-594.9.5/stdlib/FreeBSD/random.c).
- UIScreen displayLinkWithTarget:selector:: connects the original screen-update API to the existing CADisplayLink implementation.

- UIGraphics image contexts: implements APIs used by the original ImageAttacher to rotate and compose unit thumbnails.
- Empty CGImage/CGContext arguments: matches actual Core Graphics behavior to prevent termination on original dummy parts.
- UIAlertView: displays the name field added by the original and forwards input. Movement after focus is still under examination.
- Integer formatting: handles positive signs for `%+d`, precision, and field width.
- Xcode packaging: a script creates the executable instead of linking a stub and declares it as output. CodeSign then signs it.

Execution, installation, and remaining problems are recorded in [current status](../docs/current-status.md). Frame timing and game balance are not independently retuned.

## Simulator comparison apps for each edition

`validation/package-version.py` takes an existing simulator wrapper, built runtime, and external original path, and produces a separate `com.gelb2.machdefense.audit.indie105` or `.wemade103` app. It checks input SHA-256/platform and refuses to overwrite existing output. Originals, runtime, and app output all remain outside the repository. See `--help` for arguments. Do not install these over regular apps or physical-device saves.

`validation/save_fixture.py` distinguishes indie 42,476-byte saves from WeMade 42,496-byte saves. Insert generated funds-test saves only while the corresponding audit app is stopped. First keep its app-generated save in `.local`. Do not exchange saves between the two editions.

Added frameInterval 0 handling at WeMade startup and GuiBanner ad suppression. [Analysis and runtime evidence](../docs/evidence/version-audit-20261104.md).
