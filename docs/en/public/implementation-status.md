# Implementation status

<!-- language-nav:start -->
[한국어](../../public/implementation-status.md) | [English](implementation-status.md)
<!-- language-nav:end -->

Updated 2026-11-11. The game release is **v0.7**. Follow-up hotfixes applied to main, the live web build, and the iPhone installation on November 10 are distinguished below. [Release](https://github.com/gelb2/MachDefense/releases/tag/v0.7) · [Changelog](../../../CHANGELOG.en.md)

| Area | Implementation and verified coverage |
|---|---|
| iOS / iPadOS | Universal app using the shared game core. The iPhone received v0.7 and the subsequent HP/price revision as Release 0.7(7). Earlier v0.6 iPad upgrade, purchase, recovery, and production-HUD checks are distinct from the later iPhone installation. |
| Android | Native host using the same C++ core and resources. Regular and cheat development-signed v0.7 APKs are available. Galaxy checks cover 40 helicopter levels. Do not describe the tagged APKs as including subsequent main hotfixes. |
| Web / PWA | Deployed on GitHub Pages. Follow-up fixes address upgrade-list overlap and Cheyenne/Beluga HP and prices. Regular build f9f26e94f1d4e3d6 passed runtime-table comparison, updating from an older build, save preservation, and offline relaunch. |
| Tauri Mac | v0.7.0 for macOS 14+ on Apple Silicon, using the same web game. Checks cover WKWebView mission/resume, file saves, exit, and signed update packages. The Mac app uses ad-hoc signing. |
| Original features | 80 missions; production, combat, upgrades, special attacks, victory/defeat, rewards, and 5-slot saves. |
| Additions | Recurring 30/45/60-second emergency resupply, level-10 upgrades, scrolling all installed units, Laser Turret unlock and retroactive grant, and the accepted title-screen extensions. |
| Four helicopters | Scout, Cheyenne, Beluga, and AH-10 were released with Lv.1–10, aircraft-specific weapons, emergency evasion, and chaff/flares. Their appearance and improvement tasks are unfinished; use is optional. |
| Progress transfer | Shared MD1 codes restore all 5 slots without a server or account. Receiving added levels requires v0.4 or later. |

## Follow-up hotfixes

The unit list and detail panel overlapped, preventing selection of the last row. Tall screens now show all rows; small and landscape screens scroll within the list. [Implementation and web cheat-build checks (Korean)](../../evidence/upgrade-layout-hotfix-20261110.md).

Cheyenne HP is 85% of the same-level Plasma Mech, and Beluga HP is the midpoint between the Plasma Mech and Tanker. Purchase-cell costs and every upgrade-credit cost were reduced by 25%. Lv.7 HP is now 1,700 and 3,800 respectively. [Lv.1–10 values and checks (Korean)](../../evidence/aircraft-hp-price-hotfix-20261110.md). The subsequent iPhone installation verified signing and runtime tables while preserving existing progress fields. [Live-web and installation comparison](../../evidence/sass-guide-refresh-20261111.json).

## Remaining work

Helicopter improvements are on hold to prioritize restoring the entire Mach series. Cheyenne’s new appearance/assets remain unapplied, and paint, texture, depth, banking, target approach/range, and performance tasks remain. Pre-release core checks passed 56 of 58 tests; AircraftAudit and AircraftCampaign failed. Later targeted passes do not establish that all issues are resolved or that the artwork was approved. [Pre-release checks (Korean)](../../evidence/v07-release-20261110.md) · [Current essay](unease-and-typewriters.md).

Lock-on remains a design proposal. iPad improvements, production-list reordering, pathfinding, and long-duration/device-specific performance and audio checks remain. Read earlier measurements with their dates, devices, and builds in [validation results](validation.md). [Roadmap](roadmap.md)

[Game overview](../../../README.en.md) · [Documentation](README.md) · [Sass writing guide (Korean)](../../editorial/sass-publication-guide.md)
