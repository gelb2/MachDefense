# How I directed the work

<!-- language-nav:start -->
[한국어](../../public/work-method.md) | [English](work-method.md)
<!-- language-nav:end -->

I came to MachDefense with the experience of restoring HeavyMach1. I had challenged and made Codex fix saving on every shot, repeated image generation through UIKit, and platform-specific buttons added despite an existing in-game button system. For the next game, I told it to carry over the procedures and test tools developed through that work.

## Comparison with the compatibility build

First, I had the compatibility build run. On my iPhone, I checked production, combat, base destruction, mission victory, and defeat. Then I directed Codex to rebuild the original features in new source code and compare them with that build.

The compatibility build was our reference for original behavior. The comparison had to cover production-icon orientation, prices, turret placement instructions, sounds, and result screens. Whenever I found rotated unit icons or a missing production-complete sound in the rebuilt version, I told Codex to check the original. Starting the game and destroying an enemy was not enough to call the restoration finished. [Comparisons and fixes](troubleshooting.md)

## Development plans and change scope

I required reports and a development plan before implementation. I looked for missing requirements and choices I could not understand. For large tasks, I asked which parts would change and whether the core game rules were involved. I required separate branches and commits so that a bad change could be undone.

To put it in my own terms, I beat Codex into line to stop its bizarre development choices, its attacks on the architecture, and its habit of fixating on one thing, ignoring my instructions, and burning tokens on whatever it wanted to do. Telling it to remember the mistakes from HeavyMach1 was not enough.

## Stutter during base machine-gun fire

On the iPad, the screen stuttered when the base machine gun fired. I did not see the same behavior from enemy tank shells. I asked whether it created objects or redrew resources on every shot, or repeated the saving mistake from HeavyMach1, and told it to investigate with Instruments.

Codex's sound-effect implementation attached and detached the audio context on every shot. The original retained its initialized context. Time Profiler showed a path waiting for audio output to start and stop. Codex changed it to reuse the context and sound buffers.

Later, code that configured the audio session on the main thread during screen setup and activation was also found. Waiting for the OS response could delay both game input and rendering. Those calls moved to a serial work queue. Even then, when I returned from the background, music played but effects were silent. This time, the output device had not been restarted. Each cause and fix is recorded in [troubleshooting](troubleshooting.md).

## Scope of the Android port

I told Codex not to change the working iOS and iPadOS files while building Android. Movement, damage, production, upgrades, saves, menus, and the HUD were to use the shared code. Android would supply display, touch, file access, lifecycle, and audio connections.

During the initial Android port, the hashes of 2,978 Apple, shared-source, module, resource, test, and root build-configuration files matched before and after the work. Android's screen-initialization and audio-resume errors were fixed on the Android side. My iPhone progress was transferred to Galaxy, where Stage 33, production, and equipment were checked. [Android implementation and save transfer](../../evidence/android-native-20261106.md)

Features I later approved for all platforms, such as extended upgrades and recovery codes, went into shared modules. I distinguished changes made merely to attach a platform from changes that extended the game itself.

## Level-10 upgrades, the production HUD, and turret artwork

When raising turret, base, and special-attack caps to level 10, I required preservation of the original table structure. I first asked for the added levels' damage, durability, range, projectile counts, and costs. In the installed app, I found that range was not increasing and had it fixed. The production list also had to preserve INSTALL and UNINSTALL while allowing all installed units to be selected by scrolling.

I reviewed the artwork separately. I chose to retain the original level 5 turret weapons and change only the bodies to the approved designs. Adding features did not give Codex permission to change the original scale, art style, or texture at will. [Added features](features.md) · [Upgrade and HUD checks](../../evidence/level10-hud-20261107.md)

I required reports to include the changed code and actual test results. I played the regular app, while Codex recreated the conditions in the cheat app. We compared those observations before deciding the next task.

[How I tested the game](testing-method.md) · [Validation results](validation.md)
