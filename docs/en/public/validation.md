# Validation results

<!-- language-nav:start -->
[한국어](../../public/validation.md) | [English](validation.md)
<!-- language-nav:end -->

iOS, iPadOS, Android, and web/PWA use the same C++ game code.

A cheat app with storage separate from the regular app prepares test conditions. Automated checks cover units, upgrades, weapons, and missions, alongside actual button input and device interruption/resume checks.

| Check | Result |
|---|---|
| Apple module split | Compared 160 conditions and 144,000 ticks before and after the split. Checked iPhone/iPad simulators and a physical iPad |
| Recurring emergency resupply | 35 C++ checks, 27 iPhone simulator checks, and resupply/frame checks on physical iPhone and iPad |
| Android | 35 C++ checks and 13 app checks on both Galaxy and emulator. Loaded/rendered 80 missions on the emulator |
| iPhone → Android | Transferred Stage 33 progress, equipment, upgrades, and funds; checked persistence after relaunch |
| Shared web code | 37 checks each in native and Wasm builds. Compared game states for 160 conditions: 80 missions × 2 screen sizes |
| Web drawing commands | Image, order, and other metadata matched across 144,000 frames. Maximum floating-point difference: 0.00006103515625 |
| Chrome / WebKit | Production, combat, cheat victory/defeat transitions, next mission, code recovery, save failures, offline play, and GPU recovery |
| iPhone → web | Restored all 42,496 bytes of a Stage 36 save copy, then checked combat and relaunch. Original iPhone save unchanged |
| Direct Mac interaction | Safari title, slots, and recovery buttons; installed Chrome PWA launch and exit |

Direct testing of an installed iPhone PWA, 120Hz and long-duration performance, and recorded comparison with original audio remain. [Cross-platform recovery checks](../../evidence/save-transfer-all-platforms-20261106.md) · [Web checks](../../evidence/web-pwa-20261106.md)

[Game overview](../../../README.en.md) · [Documentation](README.md)

## PWA release-candidate checks

On 2026-11-06, the regular build was checked in an installed Chrome PWA for its separate window and offline relaunch. Chrome and WebKit passed code recovery, Stage 36 entry, save relaunch, GPU recovery, and save-failure checks. [Deployment checks and Tauri prerequisites](../../evidence/pwa-release-20261106.md).

## Transferring recovery codes between devices

On November 6, 2026, an iPhone save was restored in the iPad and Galaxy cheat apps, and all 42,496 bytes were compared. The regular Galaxy app retained its existing save after code generation, copying, entry, and cancellation. Galaxy Chrome and the installed PWA also generated and restored the same code. The Galaxy PWA preserved every save byte after a fresh offline launch, combat, and relaunch.

I used a code from my iPhone to restore the iPad PWA and clear one mission. I then generated a new code in the iPad PWA, restored it on my iPhone, and entered a stage myself.

[Automated and device results](../../evidence/save-transfer-all-platforms-20261106.md) · [Shared recovery code](quick-play.md#if-leveling-up-is-difficult)

## Building from a fresh GitHub clone

v0.3 was cloned into a new folder without authentication. The iOS regular and cheat simulator apps, an iOS device Release app, and Android regular and cheat APKs were built using a fresh Xcode build folder and empty Gradle cache. Every app contained all 1,387 runtime resources, with SHA-256 hashes matching the repository files.

No original IPA or files from the existing production folder were copied. Game entry and production were also checked from empty saves in new simulators and emulators. [Clean-clone checks](../../evidence/clean-clone-build-20261106.md) · [Tools and build instructions](build-and-install.md)

## v0.5 upgrades, artwork, and live deployment

The web cheat build checked 280 production, 120 placement, and 100 special-attack conditions, plus 130 HUD, upgrade, and recovery rows. Chrome and WebKit each compared 144 turret poses. Assembly, purchases, saves, and recovery were also checked on a physical iPad. Regular and cheat v0.5 apps were installed on iPhone, iPad, and Galaxy.

In a fresh browser, the live web build's 44 images and 3 tables matched the repository. With an old cache, the runtime tables sometimes retained their previous values on the first return. The second launch loaded the latest values. The first-update problem remains in the TODO. [Device checks and deployment results](../../evidence/v05-release-20261107.md).

[How I directed the work](work-method.md) · [Testing with cheat builds and direct play](testing-method.md)
