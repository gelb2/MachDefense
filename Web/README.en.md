# MachDefense Web / PWA

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

[Play / install PWA](https://gelb2.github.io/MachDefense/) · [Installation guide](../docs/en/public/build-and-install.md)

Emscripten builds the same C++20 Core, sound effects, and resources. WebGL2 output and browser input live in `src/`; the launch page, IndexedDB, and service worker live in `../Distribution/Web/`. Shared C++ draws game menus, HUD, and recovery buttons. The [Tauri Mac host](../Desktop/Tauri/README.en.md) bundles the same web product and saves progress in the Mac app data folder.

[v0.5 results](../docs/evidence/v05-release-20261107.md) · [Initial implementation checks](../docs/evidence/web-pwa-20261106.md) · [Development plan](../docs/save-transfer-web-tauri-plan.md).

## Build

Run from the repository root. Tested with Emscripten 6.0.13. Resources are included as regular Git files. Git LFS and an original IPA are not needed.

```sh
# First source emsdk_env.sh from your installed Emscripten SDK.
emcmake cmake -S Web -B build/web-normal -DMD_WEB_CHEAT=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-normal -j6
python3 Web/tools/package.py build/web-normal/dist
python3 -m http.server 8941 --bind 127.0.0.1 --directory build/web-normal/dist
```

Open `http://127.0.0.1:8941/` in a Mac browser. After downloading, select PLAY IN BROWSER or use the install button. Mobile PWA installation requires an HTTPS address accessible from the device. The public address is https://gelb2.github.io/MachDefense/ .

Use a separate build folder and port for the cheat version. Regular and cheat versions also use different IndexedDB names.

```sh
emcmake cmake -S Web -B build/web-cheat -DMD_WEB_CHEAT=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build/web-cheat -j6
python3 Web/tools/package.py build/web-cheat/dist --cheat
python3 -m http.server 8940 --bind 127.0.0.1 --directory build/web-cheat/dist
```

## Saves and recovery

On the slot screen, GET CODE → COPY exports all 5 original slots. Paste a code into ENTER CODE and press the in-game CHECK button. YES in the confirmation window replaces all slots. The previous save remains in IndexedDB's restoreBackup. Cancellation, invalid input, and write failures preserve existing progress. Single-slot transfers and extended settings are not yet supported.

Progress uses IndexedDB; installed resources use Cache Storage. The game receives success after the save transaction completes. Progress survives resource-cache downloads and PWA updates. If the player deletes site data, a recovery code can restore it.

## Checks

```sh
npm ci --prefix Web/tests
cd Web/tests
npx playwright install webkit
cd ../..
node Web/tests/transfer-ui.mjs .local/pwa-20261106 http://127.0.0.1:8941/
node Web/tests/smoke.mjs
node Web/tests/game-flow.mjs .local/pwa-20261106
node Web/tests/pwa-storage.mjs .local/pwa-20261106
MD_BROWSER=webkit node Web/tests/regression.mjs .local/pwa-20261106 http://127.0.0.1:8941/
```

Current Chrome checks use its macOS installation path. `regression.mjs` and `pwa-storage.mjs` read an `iphone/MachDefense.sav` copy in the test folder; regression also reads `recovery.txt` there. Do not commit user saves. Use smoke/game-flow for empty-progress checks. game-flow checks initial profiles, production, victory/defeat, and restart; regression checks save recovery, lifecycle, and GPU; pwa-storage checks resource corruption, updates, and cache deletion.

Recovery and offline relaunch were checked in an installed Galaxy PWA. The user confirmed iPhone → iPad PWA → iPhone recovery and mission entry. Direct testing of an installed iPhone PWA and long-duration tests on each device remain. Apple and Android have the same code menu. [Connections and results](../docs/evidence/save-transfer-all-platforms-20261106.md).

## Native / Wasm frame-data comparison

Link `tests/frame-parity.cpp` against each Core static library. Include `Sources` and `Tests`; enable C++20, exceptions, and assertions. Wasm uses NODERAWFS and the Node environment. Pass `Resources` and an output-binary path to each executable. After producing `frame-native.bin` and `frame-wasm.bin` in the same output folder, run:

```sh
python3 Web/tests/compare-frame.py .local/pwa-20261106
```

The check compares non-floating-point data exactly across 144,000 frames and records the maximum floating-point difference. The current threshold is 0.001. Original full-bit hash results are also retained; game calculations are not changed for this comparison.

Thank you to [fastutlego](https://github.com/fastutlego) for contributing the HeavyMach web port. The WebAssembly host, WebGL2 renderer, input, audio and saves in [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24), together with the subsequent PWA and Tauri work, also laid the groundwork for bringing MachDefense to the web and desktop. I used that architecture for the MachDefense PWA and used the same web host in the Tauri Mac app.
