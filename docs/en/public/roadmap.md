# Roadmap

<!-- language-nav:start -->
[한국어](../../public/roadmap.md) | [English](roadmap.md)
<!-- language-nav:end -->

As of 2026-11-07. See [implementation status](implementation-status.md) for what is released and working. The following items need research or development.

## Helicopter work on hold — 2026-11-09

All helicopter TODO items and unfinished work are on hold to prioritize restoring the entire Mach series. This includes paint, depth, banking, range, debugging, design and performance validation. Helicopters in the release are optional: use them only if you want to. Improvements for iPad, lock-on planning and other performance and design tasks remain. [The work record and reasons for postponement](../editorial/helicopter-work-record-20261109.md).

### Cheyenne’s new appearance and assets are not applied — added 2026-11-10

- [ ] Cheyenne’s newly prepared appearance changes and assets have not been applied in-game. Compare the adopted source, MCB/part definitions, prepared resources, and deployed/installed web and native builds; fix the connection and verify production, upgrade previews and combat at Lv.1–10. Preserve original units, other aircraft, and Cheyenne’s current behavior and stats.

Earlier connection and test records do not resolve this issue. This is an open TODO; implementation remains on hold. [Detailed TODO](../../usability-todo.md#md-aircraft-cheyenne-asset-apply-01--샤이엔-새-외형에셋-미적용).

### HP and price adjustments — added 2026-11-10

The HP and price adjustments below were implemented under the follow-up instruction of 2026-11-10. Other helicopter improvements remain on hold.

- [x] Set Cheyenne HP 15% below Plasma Mech HP (85% of Plasma Mech HP).
- [x] Set Beluga HP to the midpoint between Tanker HP and Plasma Mech HP at the same level.
- [x] Reduce both purchase prices and each upgrade-step price for Cheyenne and Beluga by **25%**, alongside the HP adjustments. Use the Lv.1–10 schedule below.

| Lv. | Cheyenne HP | Purchase cells | Next upgrade credits | Beluga HP | Purchase cells | Next upgrade credits |
|---|---:|---:|---:|---:|---:|---:|
| 1 | 850 | 216 | 420 | 1,500 | 378 | 660 |
| 2 | 936 | 258 | 900 | 1,750 | 444 | 1,350 |
| 3 | 1,062 | 300 | 1,800 | 2,026 | 510 | 2,700 |
| 4 | 1,190 | 342 | 3,600 | 2,300 | 576 | 5,400 |
| 5 | 1,360 | 390 | 7,200 | 2,700 | 645 | 10,800 |
| 6 | 1,530 | 450 | 14,400 | 3,300 | 720 | 21,600 |
| 7 | 1,700 | 510 | 28,800 | 3,800 | 795 | 43,200 |
| 8 | 1,956 | 582 | 57,600 | 4,450 | 876 | 86,400 |
| 9 | 2,210 | 660 | 115,200 | 5,200 | 960 | 172,800 |
| 10 | 2,550 | 750 | — | 6,100 | 1,050 | — |

The original loader doubles the integer HP field. Preserve that procedure by rounding HP to the nearest multiple of 2, with ties rounded up. Cheyenne differs from the calculated 85% target by at most 1 HP. Round Beluga’s midpoint HP the same way.

Before the adjustment, Lv.7 HP was 3,410 for Cheyenne and 6,384 for Beluga. Current HP is **1,700** and **3,800**, respectively. The purchase and upgrade prices in the table were applied together with HP at every level.

Purchase prices use production cells; upgrade prices use credits. Upgrade costs apply from the listed level to the next; Lv.10 is the cap. Total Lv.1→10 upgrade costs are **229,920 credits** for Cheyenne (previously 306,560) and **344,910 credits** for Beluga (previously 459,880).


The shared game tables now use the matching-level values from Lv.1 to Lv.10. Both web cheat browsers verified actual HP and upgrade deductions. [Changes and validation](../../evidence/aircraft-hp-price-hotfix-20261110.md).

## Distribution and platforms

- Local PWA transition, hash, reuse and 95-chunk checks passed. Compare saved progress and runtime resources during production updates as well.
- Tauri Mac file saves, recovery, automated exit, signatures and bundle-replacement checks passed. Direct menu and Cmd-Q exit checks also passed. Production downloads, real updates and window-close checks remain.
- Investigate wheel, trackpad, and keyboard movement for the Mac battlefield camera. The wheel currently works only in the production list.
- Check whether Galaxy USB defaults survive reconnection, wireless debugging, additional Android devices/OS versions, and production signing.

## Game and visuals

- Add production-list reordering with save/recovery support. Preserve INSTALL/UNINSTALL and the 7-item production queue.
- Investigate pathfinding around round structures and dense turrets, and differences in turret-placement coordinates between the original and rebuilt game. Keep mission-3 pit placement, which the original also allows.
- Measure cell production and game ticks against real time. Check on physical hardware the Android condition that produces 45 ticks per second from synthetic 90Hz input.
- Calculate automatic credit/cell production, drops, and rewards, and examine a 1.25× increase. The increase has not been approved.
- Compare long-term funds, difficulty, and performance with recurring emergency resupply.
- Compare title/splash edges and iPad/desktop scaling against original resources and field of view. Do not use the rejected 1280×800 artwork.
- Investigate world-background, HUD, and unit display boundaries. HUD visual redesign and turret retaliation against off-screen enemies are on hold.
- Design a lock-on feature in which tapping an enemy makes a nearby mech fire Crossfire. Each mech has a 3-second interval; the existing special attack remains.

## Validation and writing

- Record and compare original firing, production, impact, and mission-clear audio.
- Test repeated measurements under matching conditions, first-shot and simultaneous fire, long-duration heat/memory behavior, and 120Hz devices.
- Directly check an installed iPhone PWA, return after photos/alarms/calls/locking, and rotation/input in both portrait orientations.
- Investigate correction, initial loading, menus, and combat for local saves containing invalid levels. This is separate from valid-code recovery tests.
- Further compare port name entry, price composition, English selection, filtering, MapInfo, and combat under matching random conditions.
- Write “How I tested it” and “How I directed the work” from the author's material and validation evidence. Organize Sass writing sources separately.
- Prepare a contact channel and English thank-you draft for Mesabloo. The author reviews it before sending.

Detailed requirements and historical records are in the [development TODO](../../usability-todo.md). [Documentation](README.md)
