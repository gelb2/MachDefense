# Added features and design intentions

<!-- language-nav:start -->
[한국어](../../public/features.md) | [English](features.md)
<!-- language-nav:end -->

## Recurring emergency resupply

The original purchase button now opens an emergency-resupply window. Credit, cell, and mixed supplies become ready every 30, 45, and 60 seconds of game time. Unclaimed supplies remain available across intervals. If cell capacity is insufficient, the player receives only what fits; the remainder stays in the window.

Credits have no capacity limit equivalent to cells. Credit grants therefore use the mission-completion reward, while cell grants use current capacity and natural production. Under the default conditions of mission 1, credit supply grants 20Cr, cell supply grants 48 cells, and mixed supply grants 10Cr and 24 cells.

| Supply | Interval | Calculation |
|---|---:|---|
| Credits | 30 seconds | 4% of the mission-completion reward, rounded down |
| Cells | 45 seconds | The smaller of 10% of current capacity and 4 seconds of natural production, rounded down |
| Mixed | 60 seconds | 2% of the mission reward and half the cell grant, each rounded down |

Pausing the game also pauses resupply time. The calculation uses shared game time without a separate OS timer. Windows, text, and buttons use the original assets and game renderer.

I specified the intervals, recurring claims, and capacity-aware supplies. Codex proposed the formulas and implemented the amounts I chose in shared C++. I confirmed receipt in the actual app. [Calculations and examples](../../emergency-resupply-economy.md) · [Implementation and device checks](../../evidence/emergency-resupply-20261105.md)

## Four helicopter units

Scout, Cheyenne, Beluga, and AH-10 were added as allied units absent from the original game. I designed circling flight and emergency evasion to avoid unguided projectiles, with brief invulnerability and chaff and flares during a dodge. They reuse existing mech and turret weapons, firing effects, and sounds. All four have a maximum upgrade level of 10.

The [Stage 55 gameplay capture](gameplay-gallery.md#stage-55-helicopters-and-mechs-in-combat) shows helicopters fighting alongside mechs and the production list. Cheyenne and Beluga health, purchase costs, and upgrade costs follow the [per-level adjustments (Korean)](../../evidence/aircraft-hp-price-hotfix-20261110.md).

The helicopters are unfinished. Paint, texture, depth, banking appearance, and range issues remain, and improvement work is on hold. I recommend using them only if you want to. [Helicopter work record and reasons for the hold](unease-and-typewriters.md)

## Level-10 upgrades and production-list scrolling

59 upgrade steps were added across 6 turret types, base attack and defense, and 5 special attacks, bringing permanent upgrade caps to 10. Existing levels retain their statistics, art, and sounds. The new turret levels use original Lv.5 weapon art and the original final-level sounds. Accepted new artwork is used for turret bodies at Lv.6–10 and base parts at Lv.8–10. [v0.5 artwork and checks](../../evidence/v05-release-20261107.md). Turret damage, health, and range increase, as do base attack and defense. Special attacks increase damage, projectile count, radius, or freeze duration according to their type. [All values](../../research/level10-plan-20261107.md)

All INSTALLed units can be selected by scrolling the right-hand production list. Prices, icons, frames, and touch areas move together. INSTALL/UNINSTALL and the 7-item production queue remain. On the web, mouse-wheel and trackpad scrolling also work over the list. List reordering is not yet available.

## Expanded screen areas

Battlefield and HUD display areas were adjusted for current devices. World rendering above the right HUD and pathfinding around round structures remain separate tasks. The exposed black area at the top of the iPhone HUD was covered with an original HUD piece; the iPad appearance was preserved. [Screen changes](../../evidence/iphone-hud-top-20261106.md)

## Platforms and recovery codes

When adding Android, I instructed Codex not to change files used by the Apple app. Game rules stayed in shared C++, while Android gained rendering, input, audio, and storage connections. The web version builds the same game code as WebAssembly.

On iPhone, iPad, Android, and web/PWA, GET CODE exports all 5 profiles as one line. ENTER CODE validates the input and restores the same slot states after confirmation. It transfers game-save data without a server or account. Every platform uses the shared codec and C++ menu. [Cross-platform recovery checks](../../evidence/save-transfer-all-platforms-20261106.md).

## Planned lock-on feature

The design includes all enemy units, including turrets and tanks. The mech nearest the tapped enemy would attack using the damage, projectile count, speed, and effects of the current Crossfire level, at a 3-second interval per mech. The reticle would hide when no enemy is within range, and release the lock when the target dies. Original spawns, attack power, behavior, and the existing Crossfire skill would remain. This feature is not implemented yet. [Feature TODO](../../usability-todo.md#md-lockon-strike-01--락온한-적을-터치해-처치)

[Game overview](../../../README.en.md) · [Documentation](README.md)
