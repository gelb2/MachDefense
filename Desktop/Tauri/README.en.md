# MachDefense Tauri Mac

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

The Apple Silicon Mac app runs the same WebAssembly game in WKWebView. Game code, artwork and audio are bundled for offline play from the first launch. Progress is stored in the app's data folder, outside browser caches.

## Build

Build and package the [normal web version](../../Web/README.en.md) first. Rust, Node.js and Xcode command-line tools are required.

```sh
npm ci --prefix Desktop/Tauri
npm run build --prefix Desktop/Tauri
```

The output is `Desktop/Tauri/src-tauri/target/release/bundle/macos/MachDefense Desktop.app`. It targets macOS 14 or later on arm64. There is no Windows build yet.

## Saves and recovery

Saves live in `~/Library/Application Support/com.gelb2.machdefense.desktop/saves/`. `current.json` contains current progress, `backup.json` the previous save, and `restore-backup.json` the save before a recovery-code import. Writes use a temporary file, flush it, replace the current file and read it back for verification. The store checks length, SHA-256 and revision order.

Use GET CODE and ENTER CODE on the game's profile screen. They use the same MD1 code and five slots as iOS, Android and PWA. Shared game code draws the menus, HUD and recovery buttons. Only text entry uses a WebView field.

## Updates and distribution

Ordinary source builds do not need a signing key. For signed update artifacts, set `TAURI_SIGNING_PRIVATE_KEY` to the private-key path and run the commands below. Keep the private key outside the repository.

```sh
npm run release --prefix Desktop/Tauri
python3 Desktop/Tauri/scripts/release_manifest.py
```

When a newer version exists, an UPDATE button appears on the launcher. The app flushes progress, downloads the signed whole-app package, and replaces the bundle. Tauri updates do not use the PWA resource-chunk download. Both the signature and signed version must match. Saves stay outside the app bundle.

The macOS app has a development ad-hoc signature and is not notarized with Developer ID. If macOS blocks a downloaded app, use System Settings → Privacy & Security → Open Anyway. [Test results](../../docs/evidence/tauri-desktop-20261107.md).

## Tests

```sh
cargo test --manifest-path Desktop/Tauri/src-tauri/Cargo.toml
node Desktop/Tauri/tests/storage-host.mjs
MD_DESKTOP_AUTOTEST=1 cargo run --manifest-path Desktop/Tauri/src-tauri/Cargo.toml --features validation
```

The `validation` feature uses a separate `validation/` save folder. It checks new profiles, relaunch, mission entry, saving and closing in WKWebView. Optionally set `MD_DESKTOP_RECOVERY_FILE` to a local recovery-code file. Do not commit that code. Results are written to `validation/saves-cheat/diagnostics.json`.

## Web contribution

Thank you to [fastutlego](https://github.com/fastutlego), who contributed HeavyMach's web version. The WebAssembly runtime, WebGL2 rendering, input, audio and persistence in [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24), followed by the PWA and Tauri work, also provided the foundation for MachDefense's web and desktop expansion. MachDefense connects its shared game through the same approach and uses that web runtime in its Tauri Mac app.
