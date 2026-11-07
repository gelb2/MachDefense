# Roadmap

<!-- language-nav:start -->
[한국어](../../public/roadmap.md) | [English](roadmap.md)
<!-- language-nav:end -->

As of 2026-11-07. See [implementation status](implementation-status.md) for what is released and working. The following items need research or development.

## Distribution and platforms

- Fix previous tables running on the first PWA update. Test reuse of unchanged files, resource-bundle splitting, and recovery from corruption or interruption.
- Develop Tauri Mac window/input handling, file saves, code recovery, exit, offline execution, and signed updates.
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

- Compare HeavyMach1’s license documents, check whether the same distinctions apply, then update README.

Detailed requirements and historical records are in the [development TODO](../../usability-todo.md). [Documentation](README.md)
