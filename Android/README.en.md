# MachDefense Android

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

The regular and cheat Android apps use the shared C++ Core, Audio, and game resources. Kotlin connects the Surface, input forwarding, lifecycle, audio focus, music playback, profile-name/recovery-code entry, and clipboard. C++ draws game buttons, menus, HUD, combat, upgrades, and emergency resupply.

## Build

Open this `Android/` folder in Android Studio. Use Android SDK 36, NDK 26.3.11579264, CMake 3.22.1, and JDK 17. Set `sdk.dir` in `local.properties` to your SDK path. This file is excluded from Git.

```sh
./gradlew :app:assembleNormalDebug :app:assembleCheatDebug
```

| Variant | Package | APK |
|---|---|---|
| Regular | `com.gelb2.machdefense.restored` | `app/build/outputs/apk/normal/debug/app-normal-debug.apk` |
| Cheat | `com.gelb2.machdefense.validation` | `app/build/outputs/apk/cheat/debug/app-cheat-debug.apk` |

Builds target arm64 on Android 8 (API 26) or later. Tested devices are a Galaxy SM-S731N and an API 36 arm64 emulator. Both apps use development signing and separate saves. Production signing has not been prepared.

`prepare_assets.py` packages repository resources into the APK and creates a hash inventory. Running the game needs no external original package or download server. APKs, Gradle caches, and personal SDK paths are excluded from Git.

## Folder responsibilities

- `app/src/main/cpp`: EGL/OpenGL ES 1.1 renderer, APK resource loader, atomic saves, and JNI connections.
- `app/src/main/java`: Android Surface, touch, lifecycle, MediaPlayer, and text entry.
- `app/src/cheat`: cheat-only test-command receiver, excluded from the regular app.
- `cmake`: links existing Core/Audio and selects OpenAL Soft's OpenSL ES output.
- `tools`: resource packaging, ADB checks, save inspection, and device-copy tools.
- `protected-baseline.json`: SHA-256 hashes of 2,978 Apple, shared-source, resource, and existing test files at the start of development.

Game rules and UI run in shared C++ modules. The initial Android port preserved Apple, Sources, Modules, Resources, and the root CMake configuration. The Android-only audio-resume adjustment applies solely to a copy in the build directory.

## Checks and save transfer

The [implementation and validation record](../docs/evidence/android-native-20261106.md) lists devices, coverage, measurements, and remaining tasks. `python3 Android/tools/verify_preserved.py` compares against the baseline before the initial Android port. It also reports subsequently authorized shared recovery-menu, level-10, and artwork changes, so it is not a no-change check for the current branch. The historical preservation result is in the implementation record. `tools/device.py` reads cheat-APK state. `tools/app_smoke.py` combines actual ADB touches with existing cheat commands in a cheat app that has entered the first mission. `tools/campaign_render_sweep.py` loads all 80 mission screens in sequence.

The save structure remains `SaveData`. The app reads and writes `files/MachDefense.sav`. `copy_save_to_device.py` stops the target app, backs up its save, copies the specified save, and verifies byte equality. Keep user saves only in Git-excluded locations such as `.local/`. On profile selection, `GET CODE` exports all five slots. `ENTER CODE` validates input and restores the five slots after confirmation.

The shared C++ menu and `Modules/SaveTransfer` own buttons, confirmation dialogs, and code format. Android places a text field in the specified rectangle and connects the keyboard and clipboard. Restoration backs up the existing save, and an interrupted restore is recovered at the next launch.

`tools/save_transfer_smoke.py SERIAL CODE_FILE SAVE_FILE` checks input, errors, cancellation, confirmation, restoration, copying, Activity recreation, and relaunch in the cheat app. It reads the code from a file into the real text field and presses C++ menu buttons with ADB touches. It first backs up the target cheat-app save in `.local/`, then compares every byte against the supplied save. This check does not touch the regular app's save.

Before copying saves, build the inspection tool from the repository root. It calls the existing SaveData size, checksum, and inventory checks.

```sh
c++ -std=c++20 -O2 -I Sources Android/tools/save_inspect.cpp Sources/Core/Session/SaveData.cpp -o .local/save-inspect
```
