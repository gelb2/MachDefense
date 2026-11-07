# Shared modules

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

| Output | Source / resources | Role |
|---|---|---|
| `MachDefenseCore` | `Sources/Core` | Game rules, menus, HUD, save serialization, and drawing commands |
| `SaveTransfer` (header module) | `Modules/SaveTransfer` | Recovery codes, value validation, and Apple/Android restore-file replacement |
| `MachDefenseAudio` | `Sources/Audio` | PCM effect buffers, playback-source reuse, and context lifetime |
| `MachDefenseAssets.bundle` | `Resources` | Images, Audio, RawData, Tables, and Generated |

The Apple workspace uses `project.yml` to generate three outputs and connect them to the app. CMake's core source list lives in [core-sources.cmake](core-sources.cmake). XcodeGen uses the same Core directory. `python3 tools/check_modules.py` checks the source list, platform headers, and OpenAL provenance hashes.

Core does not reference SDKs. ResourceAccess and HostServices in `Contracts/Game.hpp` request resource reads, saves, audio, name/recovery-code entry, and clipboard operations. Audio contains the OpenAL effect implementation. The Apple host handles texture upload, AudioFile, AudioQueue, lifecycle, and file paths.

Provenance and hashes for the port's OpenAL Soft 1.22.2 are in `ThirdParty/openal-soft-source.json`; its license is in `ThirdParty/openal-soft/COPYING`. Before Apple builds, `tools/build_openal.py` creates arm64 libraries for each SDK.

OriginalImages kept for reference, IPAs, and original executables are not included in the app bundle. Android and web/PWA hosts are implemented. Tauri remains a follow-up task. [Module split and validation](../docs/evidence/module-work-20261105.md).
