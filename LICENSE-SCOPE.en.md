# Code license scope

<!-- language-nav:start -->
[한국어](LICENSE-SCOPE.md) | [English](LICENSE-SCOPE.en.md)
<!-- language-nav:end -->

The [MIT license](LICENSE) applies to original code contributions written for this restoration, to the extent of the contributor's rights. Original-game material and external code retain the separate terms below.

## Covered by MIT

| Path | Scope |
|---|---|
| `Sources/Core`, `Sources/Audio`, `Sources/Platform/Apple` | Game, audio, and Apple integration code newly written for this restoration |
| `Modules/SaveTransfer` | Newly written recovery codec and file-replacement code |
| `Android`, `Web`, `Distribution/Web`, `Apple`, `Modules`, `CMakeLists.txt` | Original platform integrations, web pages, and build configuration; external tools and resources excluded |
| `Tests`, `tools` | Original testing, build, and documentation tools |
| `docs/public/media/badges/*.svg` | Documentation SVG code containing no original-game artwork |

MIT permits use, modification, redistribution, and commercial use of the code while retaining the copyright and license notice. My decision not to make money from this restoration is stated in the [developer's statement](docs/en/RESTORATION.md).

## Material with separate terms

- Original images, audio, tables, maps, additional artwork/icons based on them, and game captures/video: [asset terms](ASSET-LICENSES.en.md).
- `Modules/ThirdParty`, the Gradle wrapper, Emscripten runtime support, external code/patches in `Compatibility`, and copied skills: [third-party notices](THIRD_PARTY_NOTICES.en.md). Existing file-level notices remain.
- Original-game analysis, decoded data, and expression taken from existing works are excluded from the root MIT license.
- The developer's statement, interviews, user originals, general document prose, and quotations are not collectively relicensed through the code license.

[Game overview](README.en.md) · [Documentation](docs/en/public/README.md)
