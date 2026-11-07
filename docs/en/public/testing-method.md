# How I tested the game

<!-- language-nav:start -->
[한국어](../../public/testing-method.md) | [English](testing-method.md)
<!-- language-nav:end -->

I applied the method used for HeavyMach1 to MachDefense. First came a compatibility build that ran the original game, followed by a source reimplementation checked against its behavior. I then required a cheat app with freely adjustable levels, items, funds, and mission state. We used the running original, the regular app intended for distribution, and the cheat app for preparing conditions quickly.

## Why I required a separate cheat app

I have never worked as a developer in the game industry. In iOS development, I learned that unit tests should be quick and easy to run frequently. I also thought about why so many games have cheat codes. Testing every weapon and upgrade becomes slow if levels and funds prevent access to the feature being checked.

I therefore asked Codex for a build that could exercise game functions and interactions regardless of player progression. The regular and cheat apps used the same game code with separate storage. Checks used temporary saves or backed-up copies. My original iPhone save was preserved before tests that moved my actual progress to other platforms.

## Prepare the state, then exercise the actual behavior

After using cheats to prepare funds and equipment, the checks followed the actual purchase, production, firing, hit, victory, defeat, and save paths. They checked rejection with insufficient funds as well as successful purchases. Saves were reread to check upgrade persistence, then transferred through recovery codes for comparison.

On November 7, 2026, the level-10 extension and production-list scrolling work included these checks in the cheat app on a physical iPad.

| Target | Checks performed |
|---|---|
| Production, turret placement, special attacks | 280 / 120 / 100 cases; this test took 5.595 seconds |
| Base upgrades | 120 cases, 552 purchases, 42 appearance transitions |
| Actual upgrade-screen buttons | 250 rows × 2 conditions; 225 purchases, 275 rejections, 250 disk reloads |
| Weapon firing and hits | 267 definitions × 2 conditions; 6,048 shots and 6,048 hits |
| Added levels, recovery, and scrolling | 260 cases, 130 restores, 133 GPU outputs |

[Test conditions and results](../../evidence/level10-hud-20261107.md)

The 80-mission progression checks examined results after victory and defeat, next missions, rewards, and saves. On the Android emulator, each of the 80 missions was entered, its resources loaded, and its screen rendered. The records distinguish what each “80 missions” check actually covered. [Campaign checks](../../evidence/source-cheat-campaign-20261104.md) · [Android checks](../../evidence/android-native-20261106.md)

## The sound failure the cheat-app checks missed

While playing the modularized app, I found that returning from the background restored music but left shell fire and production-complete effects silent. The existing check counted calls to the sound playback function. The count increased even while the output device was stopped.

Codex added a check of the sound's sample playback position. Before the fix, it stayed at 0 seconds after returning. After connecting output-device pause and resume, the position advanced to 0.192–0.213 seconds during a 0.2-second wait. Recording speaker output and comparing it with the original remained a separate task. [Reproduction and fix](../../evidence/effects-resume-20261105.md)

## What the frame measurements mean

**The frame test recorded p99 16.67ms, with 0 intervals above 25ms.** This was measured on November 7, 2026 in the cheat app on an iPad Air 13-inch (M2), in a full game session with 14 installed unit types and level-10 base, units, and turrets. Combat ran for 300 frames, followed by another 300 frames of the same combat while scrolling the HUD. Consecutive CADisplayLink timestamps were recorded without a debugger attached.

At 60fps, the display updates 60 times per second, about 16.67ms apart. The p99 is the value around the 99% point when measured intervals are sorted from shortest to longest. Zero intervals above 25ms means no interval longer than 25ms was observed in that segment.

The p95 CPU time for game calculation and rendering submission was 5.24ms in combat and 5.94ms during combat with scrolling. There were 0 game-save writes in those measured segments. A separate 1,800-frame world test recorded CPU time of 40.85ms and 1 interval above 25ms when 6 turrets were created together. After the range fix, another run of 299 combat frames and 300 scrolling frames also recorded p99 16.67ms and 0 intervals above 25ms; CPU p95 was 5.31ms and 5.52ms respectively. [Measurements by scene](../../evidence/level10-hud-20261107.md)

Getting there involved fixing code that detached the audio context on each base-machine-gun shot and moving audio-session calls off the main thread. In the production HUD, dragging over a disabled unit had cancelled scrolling; that was fixed too. Fast functional checks were followed by real-time combat and dragging to measure frame behavior. Performance and audio measurements ran separately from other load tests. [Audio-session fix and measurements](../../evidence/audio-session-20261105.md)

## I moved my own progress between platforms

The checks also used an established profile. Progress, unlocks, upgrades, and funds accumulated on my iPhone were transferred and restored through a code, with all 42,496 bytes compared. I used an iPhone-generated code to restore the iPad PWA and cleared a mission. I then generated a new code there, restored it on the iPhone, and confirmed entry into a stage. [Recovery checks by platform](../../evidence/save-transfer-all-platforms-20261106.md)

The web cheat app tested purchases, recovery, combat, and input in Chrome and WebKit. Regular apps and the deployed PWA were checked with actual saves and restarts. Post-deployment checks found that an older PWA could run the previous tables on its first update; that remains a recorded task. [Post-deployment v0.5 checks](../../evidence/v05-release-20261107.md)

The cheat app let Codex bring up features regardless of my equipment or level. We used it to reproduce failures I found while playing and rerun the same conditions after a fix.

[How I directed the work](work-method.md) · [Validation results](validation.md) · [Troubleshooting](troubleshooting.md)
