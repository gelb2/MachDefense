# Architecture

<!-- language-nav:start -->
[한국어](../../public/architecture.md) | [English](architecture.md)
<!-- language-nav:end -->

iOS, iPadOS, Android, and web/PWA use the same C++ game code.

## Shared game code and platform hosts

| Area | Source location | Responsibility |
|---|---|---|
| Core | `Sources/Core/` | Damage, movement, range, production, economy, upgrades, missions, saves, game menus, and HUD |
| Audio | `Sources/Audio/` | Reuse of sound-effect buffers, playback sources, and contexts |
| SaveTransfer | `Modules/SaveTransfer/` | Recovery-code encoding, decoding, and validation |
| Resources | `Resources/` | Images, audio, maps, and tables |
| Apple | `Apple/`, `Sources/Platform/Apple/` | Workspace, rendering, input, audio session, and save paths |
| Android | `Android/` | Kotlin, JNI, EGL, audio output, and app lifecycle |
| Web | `Web/`, `Distribution/Web/` | Wasm, WebGL2, browser input, audio, IndexedDB, launch page, and PWA |
| Tauri Mac | `Desktop/Tauri/` | Rust/WKWebView host, bundled web game, atomic file saves, signed app updates |

Core does not call platform SDKs directly. It requests resource reads, saves, audio, and text entry through `ResourceAccess` and `HostServices`. Each host renders the `Frame` produced by Core. Menus and HUD buttons use this same path.

## Rendering and audio

Apple uses the OpenGL ES 1 fixed-function pipeline, Android uses EGL/OpenGL ES 1.1, and the web uses WebGL2. Textures are uploaded and reused; firing does not create new images.

On Apple, OpenAL Soft plays sound effects and AudioFile/AudioQueue plays music. Android uses MediaPlayer for music. Web sound effects use the existing code's Emscripten OpenAL connection, while music uses Web Audio output. The game code decides which sound to play and when.

## Saves and recovery

The reference save format has 5 slots and occupies 42,496 bytes. Recovery codes validate the version, length, and CRC32, and transfer data using PackBits and Base64url. Corrupt or unsupported codes are rejected.

The web stores game progress in IndexedDB and reports success after the transaction completes. Resources use Cache Storage. Downloading the resource cache again preserves progress. Save revisions prevent an older tab from overwriting a newer save when two tabs open the same progress. If the player deletes site data, a recovery code can restore the progress.

Restoring creates a backup of the existing save first. The current menu replaces all 5 slots. Apple and Android use the shared FileStore for backups, interrupted-restore recovery, and file replacement. The shared C++ menu draws the copy, check, cancel, and confirmation buttons. Platforms provide text entry, the keyboard, clipboard, and storage paths. Single-slot selection is a proposal; the current operation transfers all 5 slots.

Tauri stores the same save bytes in the app data folder, outside browser caches. It flushes a temporary file, replaces the current file, and verifies the result. Previous progress and the pre-import save have separate backups. The game and recovery codec are unchanged.

[Game overview](../../../README.en.md) · [Documentation](README.md)
