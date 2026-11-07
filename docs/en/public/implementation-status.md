# Implementation status

<!-- language-nav:start -->
[한국어](../../public/implementation-status.md) | [English](implementation-status.md)
<!-- language-nav:end -->

As of 2026-11-07, the game release is **v0.5**. [Release](https://github.com/gelb2/MachDefense/releases/tag/v0.5) · [Changelog](../../../CHANGELOG.en.md)

| Area | Implementation and verified coverage |
|---|---|
| iOS / iPadOS | Universal app. Regular/cheat v0.5 installed on iPhone/iPad. Upgrade, purchase, recovery, and combat checks on iPad |
| Android | Native host using the same C++ game modules. Regular/cheat v0.5 installed and checked on Galaxy. Development-signed APKs available |
| Web / PWA | Deployed on GitHub Pages. Chrome/WebKit checks, installed Galaxy PWA offline/recovery checks, user-confirmed iPhone↔iPad PWA recovery |
| Tauri Mac | Not implemented. App window, file storage, exit, and updates remain |
| Original features | 80 missions; production, combat, upgrades, special attacks, victory/defeat, rewards, and 5-slot saves |
| Additions | Recurring 30/45/60-second emergency resupply, level-10 upgrades, scrolling all installed units, Laser Turret unlock and retroactive grant |
| Progress transfer | Shared MD1 code restores all 5 slots. Receiving added levels requires v0.4 or later |

Web resource caches and game saves are separate. During v0.5 deployment checks, an old cache sometimes ran previous tables on the first return. Reopening loaded current values and preserved the save. The first-update transition still needs fixing.

Measurements are in [validation results](validation.md) and their linked records. Recorded output-audio comparisons, long-duration performance, and direct installed-iPhone-PWA checks remain. [Roadmap](roadmap.md)

[Game overview](../../../README.en.md) · [Documentation](README.md)
