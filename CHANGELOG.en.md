# Changelog

<!-- language-nav:start -->
[한국어](CHANGELOG.md) | [English](CHANGELOG.en.md)
<!-- language-nav:end -->

## 2026-11-10 Web hotfix — upgrade-list overlap

Adding four helicopters expanded the upgrade list to four rows, causing the unit detail panel to overlap the list and hide Ion Cannon and Vulcan entries. Updated the shared UI to position the detail panel and close button according to the list height and allow scrolling within the list on shorter screens.

Chromium and WebKit cheat-build checks passed for selecting, upgrading, installing, uninstalling and reaching Lv.10 across all 18 units, and for preventing accidental purchases while dragging at four screen sizes. The fix is intended for deployment to the regular web/PWA build. Native apps remain at 0.7 (build 7), and the latest GitHub release remains v0.7.

## v0.7 — Four helicopters and title-screen extensions

2026-11-10. [Play the web/PWA version](https://gelb2.github.io/MachDefense/) · [Release and installation record (Korean)](docs/evidence/v07-release-20261110.md)

- Added Scout, Cheyenne, Beluga, and AH-10 with Lv.1–10 upgrades and level-specific appearances, aircraft-specific weapons, emergency evasion, and chaff/flares. This release includes their implementation from the existing working branch.
- Connected the approved landscape and portrait title extensions to the shared game screen. The original center’s machines, lettering, and proportions remain intact; cliffs, sky, and clouds extend the landscape composition.
- The Apple and Android apps are version 0.7 (build 7); Tauri Mac is 0.7.0. Regular and cheat Android APKs, the Apple Silicon Mac app, and a signed update package are provided. The Android APKs use development signing; the Mac app uses ad-hoc signing.

### A note on suspending helicopter-unit development

I encountered intelligent breaches of duty and negligence that I had not faced even while restoring HeavyMach1. I am putting helicopter improvements on hold to prioritize my foremost goal: restoring the entire Mach series.

The helicopter units released in this version meet neither the original creators’ aesthetic goals nor my own. Their level of finish undermines both the creators’ intentions and mine, so I recommend using the helicopter units only if you want to.

[An unease unlike HeavyMach1 — neo-Lysenkoism and Ceaușescu’s typewriter](docs/en/public/unease-and-typewriters.md) · [Incident report on ignored instructions and unfinished work (Korean)](docs/evidence/aircraft-work-failure-20261109.md) · [Drawing-work refusal incident (Korean)](docs/evidence/aircraft-instruction-refusal-20261108.md) · [Troubleshooting](docs/en/public/troubleshooting.md)

### Implementation and validation records

[Helicopter upgrades, weapons, and all 40 Android levels](docs/evidence/android-aircraft-20261109.md) · [Scout and Beluga game integration](docs/evidence/aircraft-v5-integration-20261109.md) · [Approved title preview](docs/research/title-joins-20261110.md). These records are in Korean. This release’s build, installation, and production-check coverage belongs in the [release record](docs/evidence/v07-release-20261110.md).

In local pre-release checks, 56 of 58 core tests passed. `AircraftAudit` and `AircraftCampaign` failed because old test expectations remained after the earlier V5 asset integration; they were left unchanged under the hold on helicopter improvements. The title Menu checks and six viewport comparisons in Chromium/WebKit passed, as did regular-web save, resume, offline and GPU recovery checks, both Android APK builds, and the Tauri build and actual WKWebView mission/resume checks.

[Pre-release checks and remaining failures](docs/evidence/v07-release-20261110.md) · [Actual deployment and iPhone installation results](https://github.com/gelb2/MachDefense/releases/tag/v0.7)

## v0.6 — PWA updates, Tauri Mac, and Laser Turret costs

2026-11-07.

- PWA resources are checked by SHA-256 and length. Existing resources are reused; the large game resource bundle is split into 95 chunks so only changed chunks are downloaded. Running games keep their version; closing their windows allows a prepared update to activate. Failed updates preserve the previous version.
- Added a Tauri Mac app using the same WebAssembly game. Saves live in the app data folder. It supports MD1 recovery codes, waiting for saves before exit, and signed app updates. Requires macOS 14 or later on Apple Silicon.
- Laser Turret production costs for levels 1–10 are now 200/220/240/270/300/330/360/390/420/450 cells. Other stats and its size are unchanged.
- Updated iPhone, iPad and Android app versions to 0.6. Added thanks to HeavyMach web contributor fastutlego and PR #24 in Korean and English documentation.

### Checks

Passed 42 C++ tests; Chrome/WebKit install-failure, update, recovery and offline checks; and actual Galaxy Chrome and native Android checks. Recreated an old installation using production v0.5 files and verified the update and save preservation. Tested upgrades, purchases, save/recovery and the production HUD on physical iPhone/iPad, then installed all four regular/cheat apps. Saves matched before and after installation. Mac checks covered file saves, recovery, Cmd-Q, menu exit, signatures and replacement of a disposable app copy.

[PWA checks](docs/evidence/pwa-update-20261107.md) · [Mac checks](docs/evidence/tauri-desktop-20261107.md) · [Device installation](docs/evidence/native-release-20261107.md)

Title extensions are excluded because their joins did not match the original. Aircraft remain research and concepts. Battlefield wheel/keyboard movement on Mac, comparison with the original tablet view, and invalid local-save handling remain follow-up work.

## 2026-11-07 Work and testing essays

Added Korean and English accounts of directing the work and testing with cheat builds. Linked them from the README, index, validation, and troubleshooting pages. Each game retains its own failures and measurement conditions. Updated the integrated report and Sass writing materials.

## 2026-11-07 Social preview

Selected candidate B for the GitHub repository and web/PWA social preview. Added Open Graph and Twitter card metadata to the launcher and included the same image in the web package. Game code, progress, and v0.5 apps are unchanged.

## 2026-11-07 README combat scenes

Added the special-attack GIF, its MP4 link and the Stage 40 combat image to the README without changing its existing text. Removed the two bug-report images from the gameplay gallery. Their source files remain available for troubleshooting.

## 2026-11-07 Gameplay media

Added 11 still images, two combat GIFs and three short MP4 clips from user gameplay recordings. The Korean and English gallery links to the README and documentation index. Source hashes, timestamps and captions are shared with the Sass writing guide. Game code and release v0.5 are unchanged.

## 2026-11-07 documentation update

Added Korean and English versions of 26 visitor documents, with language links on each page. Added separate guides to implementation status and remaining work, and connected topic-specific manuscripts, images, and validation evidence in the Sass writing guide. Updated stale build/test descriptions and document links. The game release remains v0.5; code, assets, and save format are unchanged.

A follow-up applies MIT to newly written code and adds Korean/English code-scope, asset, and third-party notices. README now has a code-license badge. Visitor documentation totals 29 pairs.

## v0.5 — turret bodies and Laser Turret unlock

2026-11-07. [Play web / PWA](https://gelb2.github.io/MachDefense/) · [Changes and checks](docs/evidence/v05-release-20261107.md)

- Additional turret levels 6–10 use the original level-5 weapon art with approved bodies for each level. Applies to Rifle, Missile, Vulcan, Ion, Laser, and Plasma: 6 types. Existing assembly positions and sizes remain.
- Added Laser Turret as a mission-30 completion reward. Profiles already beyond that point receive it when the Vulcan Turret unlock requirement is met.
- Ion splash damage and Plasma damage over time also increase at the added levels. Values at every level and level preservation through recovery codes were checked.
- Apple and Android app versions now read 0.5. Regular and cheat Android APKs are available.

### Checks

Chrome and WebKit each captured 144 turret poses and checked production, placement, special attacks, upgrades, and recovery. Physical iPad checks passed 144 assembly poses, 130 upgrade rows, 117 purchases, 143 rejections, and 130 save/code round trips. Galaxy passed 13 checks covering production, pause, upgrade panels, audio focus, graphics recovery, background return, resupply, victory, next mission, defeat, and restart.

After deployment, Chrome/WebKit recovery and offline checks passed, and hashes of 47 runtime resources were compared. One case ran previous tables on the first return from an old version; reopening loaded the latest tables. Saves were preserved. This transition problem remains for a follow-up fix.

### Remaining work

Tauri Mac, reuse of unchanged PWA resources, resource-bundle splitting, and stronger update transitions remain. [Current implementation and research](docs/research/pwa-tauri-updates-20261107.md).

## v0.4 — level-10 upgrades and production-list scrolling

2026-11-07. [Play web / PWA](https://gelb2.github.io/MachDefense/) · [All values](docs/research/level10-plan-20261107.md)

- Added 59 steps across 6 turret types, base attack/defense, and 5 special attacks, bringing permanent upgrade caps to 10.
- Increased damage, health, and range for added turret/base levels. Existing levels retain their performance; original final-level appearances, projectiles, and sounds are reused.
- All installed units can be selected by scrolling the production HUD. Prices, icons, frames, and input areas move together. INSTALL/UNINSTALL and the 7-item production queue remain.
- Connected mouse-wheel and trackpad scrolling to the web production list.
- Special-attack upgrade prices with large inventories use 64-bit calculations.
- Excluded the rejected title-extension art from the app.

### Checks

40 shared C++ checks and Chrome/WebKit purchase, production, upgrade-display, save, and recovery checks passed. ATK, range, DEF, and their increments were compared down to glyph sprites. Actual firing was checked inside and outside the new range. Upgrade UI, combat, and scrolling were checked again on iPad. Across 599 frames, interval p99 was 16.67ms, with 0 frames above 25ms. [Results](docs/evidence/level10-hud-20261107.md) · [Range fix and per-level values](docs/evidence/level10-range-damage-20261107.md)

### Installation and saves

Apple and Android internal app versions are 0.4. Regular and cheat Android APKs are available. Apple apps are built and signed with Xcode. Existing profiles, funds, unlocks, equipment, and the MD1 recovery format remain. Update the receiving app to v0.4 before transferring saves with newly added upgrade levels.

Production-list reordering, further growth of Ion splash effects and Plasma damage over time, and Tauri Mac remain. [TODO](docs/usability-todo.md)

## v0.3 — cross-device recovery codes and Quick Play

2026-11-06. [Play / install PWA](https://gelb2.github.io/MachDefense/) · [Quick Play / shared code](docs/en/public/quick-play.md)

- iPhone, iPad, Android, and web/PWA transfer all 5 profiles with the same MD1 code.
- GET CODE, COPY, ENTER CODE, CHECK, and YES/NO on the slot screen use shared game buttons. Platforms connect text entry, keyboard, and clipboard.
- Native recovery adds backup, interrupted-restore recovery, atomic replacement, and completion records. New progress after recovery is retained too.
- Saves with all 80 missions completed can be restored.
- README now includes PWA guidance and a shared code with Stage 42, 12 unit types, and 20,329Cr.

### Verified behavior

- 38 shared C++ checks, iPhone/iPad simulators, Android emulator, and Chrome/WebKit regression checks passed.
- Code generation, recovery, and saves were checked on physical iPhone, iPad, and Galaxy. After restoring in iPad/Galaxy cheat apps, all 42,496 bytes matched the iPhone original.
- The regular Galaxy app's save was preserved. Galaxy PWA saves also survived offline combat and relaunch.
- I restored an iPhone code in the iPad PWA, cleared a mission, then restored the PWA's new code on iPhone and entered a stage.
- Injected file replacement, disk sync, rollback, and cleanup failures to check existing-save preservation and relaunch recovery.

### Downloads

Regular and cheat Android APKs are provided. They use development signing and were checked on Galaxy; internal app version is `0.1-android-dev`. Apple internal app version is `0.1`, built and signed through Xcode. The source release is `v0.3`; the live PWA build is `d679d7b1061097b5`. APK SHA-256 hashes are in the Release's `SHA256SUMS.txt`.

Combat, economy, movement, weapon calculations, audio, and game resources were unchanged by this recovery integration. Direct installed-iPhone-PWA checks, long-duration performance, and Tauri Mac remain. [Results](docs/evidence/save-transfer-all-platforms-20261106.md) · [Full TODO](docs/usability-todo.md)

## v0.2 — first web / PWA release

2026-11-06. [Play / install](https://gelb2.github.io/MachDefense/) · [Installation guide](docs/en/public/build-and-install.md)

- Built the same C++ game and resources as WebAssembly, connecting WebGL2, browser input, and audio.
- Added an installable PWA, resource-download display, offline execution, and updates.
- Progress uses IndexedDB. GET CODE and ENTER CODE on the slot screen restore all 5 slots in another web copy.
- Added web/PWA links and installation guidance to README and updated platform/language badges.
- Preserved Apple/Android hosts and existing combat calculations. Added optional recovery services and menu connections to 4 shared files.

Installed native apps remain v0.1. Tauri Mac, Apple/Android recovery menus, and further physical-mobile-PWA checks are follow-up tasks. [Web release description](docs/public/web-pwa-release-draft.md) · [Deployment checks](docs/evidence/pwa-release-20261106.md).

## 2026-11-06 documentation update

Updated README, the developer's statement, features, development history, architecture, installation, validation, troubleshooting, and credits. Emergency resupply is identified as an addition designed by the author. Automated execution and cheat-app testing are explained. Links identify the original source and mesabloo's earlier restoration attempt.

Web/PWA is undergoing checks on the development branch. The [next-release draft](docs/public/web-pwa-release-draft.md) records implementation and validation coverage. The game release tag remains v0.1.

## v0.1 — first iOS / iPadOS / Android release

2026-11-06. [GitHub Release](https://github.com/gelb2/MachDefense/releases/tag/v0.1) · [Source tag](https://github.com/gelb2/MachDefense/tree/v0.1) · [Installation](docs/en/public/build-and-install.md)

### Game

- Connected production, combat, base/equipment upgrades, special attacks, victory/defeat, rewards, and profile saves across the original 80 missions.
- Expanded battlefield and HUD display areas for iPhone/iPad screens.
- Added recurring credit, cell, and mixed resupply at 30/45/60-second intervals through the original funds button. No ads or in-app purchases.
- Separated the regular app from a cheat app with adjustable test conditions.

### Platforms and fixes

- iOS/iPadOS share one Apple workspace and shared C++ modules.
- Android uses the same game code and resources with Kotlin, JNI, EGL, audio, and file-storage connections. 2,978 existing Apple, shared-code, and resource files were unchanged.
- Compared and fixed audio-context handling during base fire, missing effects after resume, production/reward icon direction, prices, selection effects, and menus.
- Fixed Android output pause/resume errors and optional-image loading differences.

### Checks

- Installed regular/cheat apps on physical iPhone/iPad and ran Apple automated checks, 80-mission progression/saves, upgrades, and weapons.
- Galaxy and emulator each passed 35 C++ and 13 app checks. The emulator loaded 80 mission screens; both recovered EGL and Activity state.
- The user confirmed starting and clearing the first mission in the regular Android app without problems.
- Copied iPhone progress, equipment, upgrades, funds, and special attacks into the regular Galaxy app; checked Stage 33 entry and persistence after relaunch.

### Downloads and remaining work

Regular and cheat Android APKs use development signing; their internal app version is `0.1-android-dev`. Apple's internal app version is `0.1`, built and signed with Xcode. The source release tag covering all three platforms is `v0.1`.

Pathfinding stalls, the full production HUD list and ordering, right HUD/title art, higher upgrade caps, and user-facing recovery codes are follow-up tasks. Android wireless connection, long-duration thermal behavior, and more OS/device checks remain. Web, PWA, and Tauri Mac are not yet implemented.

[Device checks](docs/en/public/validation.md) · [Android implementation](docs/evidence/android-native-20261106.md) · [Troubleshooting](docs/troubleshooting.md) · [TODO](docs/usability-todo.md)
