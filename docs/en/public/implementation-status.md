# Implementation status

<!-- language-nav:start -->
[한국어](../../public/implementation-status.md) | [English](implementation-status.md)
<!-- language-nav:end -->

As of 2026-11-07, the game release is **v0.6**. [Release](https://github.com/gelb2/MachDefense/releases/tag/v0.6) · [Changelog](../../../CHANGELOG.en.md)

| Area | Implementation and verified coverage |
|---|---|
| iOS / iPadOS | Universal app. Regular/cheat v0.6 installed on iPhone/iPad. Upgrade, purchase, recovery and production HUD checks on both devices |
| Android | Native host using the same C++ game modules. Regular/cheat v0.6 installed and checked on Galaxy. Development-signed APKs available |
| Web / PWA | Deployed on GitHub Pages. Chrome/WebKit checks, installed Galaxy PWA offline/recovery checks, user-confirmed iPhone↔iPad PWA recovery |
| Tauri Mac | macOS 14+ Apple Silicon app. File saves, recovery, Cmd-Q, menu exit and signed updates checked |
| Original features | 80 missions; production, combat, upgrades, special attacks, victory/defeat, rewards, and 5-slot saves |
| Additions | Recurring 30/45/60-second emergency resupply, level-10 upgrades, scrolling all installed units, Laser Turret unlock and retroactive grant |
| Progress transfer | Shared MD1 code restores all 5 slots. Receiving added levels requires v0.4 or later |

Web resource caches and game saves are separate. v0.6 checks resource hashes and lengths and downloads changed chunks. It keeps current and previous build resources; game progress is excluded from cleanup. Chrome/WebKit checks recreated an old installation using production v0.5 files and verified updating and save preservation. [PWA checks](../../evidence/pwa-update-20261107.md).

Measurements are in [validation results](validation.md) and their linked records. Recorded output-audio comparisons, long-duration performance, and direct installed-iPhone-PWA checks remain. [Roadmap](roadmap.md)

[Game overview](../../../README.en.md) · [Documentation](README.md)
