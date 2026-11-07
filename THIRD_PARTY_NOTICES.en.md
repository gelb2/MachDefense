# Third-party code and license notices

<!-- language-nav:start -->
[한국어](THIRD_PARTY_NOTICES.md) | [English](THIRD_PARTY_NOTICES.en.md)
<!-- language-nav:end -->

## Game and build code

| Component | Location / use | License and text |
|---|---|---|
| OpenAL Soft 1.22.2 | `Modules/ThirdParty/openal-soft`, Apple/Android effects. Provenance/hashes: `openal-soft-source.json` | Main sources: LGPL-2.0-or-later; some portions: BSD-3-Clause. [LGPL](LICENSES/OpenAL-Soft-LGPL-2.0.txt), [BSD](LICENSES/OpenAL-Soft-BSD-3-Clause.txt). Retain file-level notices |
| stb_image | `Modules/ThirdParty/stb/stb_image.h`, PNG/JPEG decoding | MIT selected from MIT/Public Domain alternatives. Copyright © 2017 Sean Barrett. [MIT](LICENSES/stb-MIT.txt) |
| Gradle 8.13 wrapper | `Android/gradlew`, `gradlew.bat`, `gradle/wrapper/gradle-wrapper.jar` | Apache-2.0. [Wrapper text](LICENSES/Gradle-wrapper.txt), [distribution notice](LICENSES/Gradle.txt), [official source](https://github.com/gradle/gradle/tree/v8.13.0) |
| Emscripten | Web build and runtime support including OpenAL and filesystem | MIT/NCSA alternatives. [Installed SDK notice](LICENSES/Emscripten.txt), [official license](https://github.com/emscripten-core/emscripten/blob/main/LICENSE) |
| musl | Emscripten C library | MIT and file-level notices. [SDK COPYRIGHT](LICENSES/musl-COPYRIGHT.txt) |

Apple and Android statically link OpenAL Soft. Library source, build configuration, and app source are in this repository. Android's resume fix is applied to a build-folder copy by [opensl-resume.cmake](Android/cmake/opensl-resume.cmake). Include that change when modifying and relinking the library. Build routes are in the [Apple](Apple/README.en.md) and [Android](Android/README.en.md) guides. Under LGPL section 6, provide the library/app source and relinking materials corresponding to distributed builds, and do not prohibit reverse engineering to debug those modifications.

The web uses Emscripten's OpenAL implementation through `-lopenal` in `Web/CMakeLists.txt`. Native OpenAL Soft binaries are not included in the web build. Puppeteer and Playwright are development dependencies for web tests and retain their npm-package notices.

## Comparison runtime and reference material

| Material | Origin and terms |
|---|---|
| touchHLE code in `Compatibility/patches` | MPL-2.0. [License](LICENSES/touchHLE-MPL-2.0.txt). Original and patch-target commits are recorded in [upstream.json](Compatibility/upstream.json) |
| SDL code in patches | zlib. [License](LICENSES/SDL-zlib.txt), [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2). Retain original notices |
| `Compatibility/patches/rust-sdl2.patch` | Copyright © 2013 Mozilla Foundation, MIT. [Notice](LICENSES/rust-sdl2-MIT.txt); exact commit in upstream.json |
| Reference engineering-debug/review/test skills | Copyright © 2025 Addy Osmani, MIT. [Retained notice](LICENSES/engineering-skills-MIT.txt). Keep each skill's existing LICENSE |
| Other copied skills and reference documents | Retain [source/hash records](docs/skill-source-manifest.json) and file-level notices. Excluded from root MIT |

The compatibility app's original game package is not uploaded externally. External-code notices are not replaced with this project's author name. Notice-file sources and hashes are in the [inventory](docs/license-notice-sources.json).

[Code scope](LICENSE-SCOPE.en.md) · [Asset terms](ASSET-LICENSES.en.md)


## Tauri Mac

`Desktop/Tauri` uses Tauri, Wry, Tao and Rust libraries. Cargo.lock fixes the versions. The build collects each package’s license declaration and LICENSE, COPYING, NOTICE and COPYRIGHT files into `Contents/Resources/notices/` in the app. Tauri, Wry and Tao offer MIT/Apache-2.0 licensing; package notices are preserved. Game, Emscripten and asset notices are included in the same folder. [Tauri upstream](https://github.com/tauri-apps/tauri) · [Build script](Desktop/Tauri/scripts/prepare.py).
