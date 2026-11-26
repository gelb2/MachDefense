# Why MachDefense has enemy air raids

<!-- language-nav:start -->
[한국어](../../public/enemy-air-raids.md) | [English](enemy-air-raids.md)
<!-- language-nav:end -->

## Moonlight and Lux Lunae

The author of the HeavyMach 1 restoration essay wrote that moonlight became a personal symbol after a motorcycle trip along unfamiliar roads at night. The original HeavyMach helped shape his decision to become a developer. Ace Combat 8 gave him a release to look forward to during a difficult period. Lux Lunae, Latin for “moonlight,” connected those memories. The F-22, Su-57, and MiG-31 added to the HeavyMach 1 restoration recalled the black jets and gold trim as well as the danger and tension of facing the squadron. [HeavyMach 1 essay “Why Lux Lunae?”](https://github.com/gelb2/HeavyMach/blob/e6be24957d8ca12db6c9e2846b41fee88c800fcc/docs/en/public/lux-lunae-intent-20261028.md).

MachDefense carries the livery over to an enemy strike aircraft. The familiar approach, bomb drop, and explosion now come from the enemy's side. Black paint, gold lines, and gold cockpit glazing distinguish the two aircraft at battle scale. A red reticle tells the player where the strike is headed.

## The F-15 STOL/MTD and candidate 09

The design started from the F-15E family and used the F-15 STOL/MTD demonstrator for its shape. The top view shows the close-coupled canards, long tandem canopy, gaps between the wings and tailplanes, and rectangular exhausts. The approved HeavyMach 1 aircraft informed the livery. MachDefense's allied strike sprite set the reference for shading and edges at game size.

Early variants read as green-gray, and the gold lines and cockpit were hard to see in battle. The revisions made black and gold legible, and candidate 09 was selected after comparing the shape, livery, and cockpit together. The restoration author then pointed out that the upper surface looked like stacked plywood. The final edit kept the approved outline and gold markings and smoothed the upper surface into a continuous matte finish like paint on a modern aircraft. [Approved candidate and edit instructions](../../../Art/EnemyAirStrike/README.md) · [Variant comparisons and game integration (Korean)](../../research/enemy-airstrike-art-20261127.md).

![Enemy aircraft over a red reticle alongside the allied aircraft](../../evidence/enemy-airstrike-20261127/ios-cheat-enemy-air-raid-and-allied-airstrike.png)

## When the raids appear

Enemy raids begin in stage 11 and continue through stage 80. Each stage starts with a wave at 60 seconds of battle time, and later waves follow at intervals of 20 to 45 seconds. Each wave's count is drawn in advance from one, two, or three aircraft with equal probability and stored alongside its time. Additional aircraft enter one at a time at 1.5-second intervals. In stage 11's second wave, for example, the three aircraft enter at 81, 82.5, and 84 seconds. [Raid schedule](../../../Resources/Tables/EnemyAirRaidSchedule.tsv).

On entry, each aircraft fixes its target on the location of the foremost living allied mech. The aircraft is skipped if no mech is alive. The red reticle stays at the chosen location, and moving mechs can leave the target area. The same schedule plays on a retry. Pausing stops the battle clock, and ending the battle cancels remaining launches. [Battle code](../../../Sources/Core/Simulation/WorldCombat.cpp) · [Shared flight and schedule](../../../Sources/Core/Simulation/SpecialAttack.cpp).

## How damage is set per aircraft

The original design left the weakest mech with at least half its health after receiving every bomb. The restoration author later set the cap for **one aircraft at 80% of the lowest maximum HP**. The game computes max HP for all deployable mech chassis, levels, and part combinations with its existing loadout calculation. The same stage uses the same cap regardless of the player's current roster.

Let `H` be the lowest available maximum HP and `s` the stage number. The per-aircraft cap is `C = floor(4H / 5)`. A single aircraft can deal up to `floor(C × (69 + s − 11) / 138)` damage in stages 11 through 80. It starts at half of `C` and reaches `C` at stage 80. The current weakest mech is `MachineGun_01` with 300 HP. One aircraft can deal up to 120 damage in stage 11 and 240 damage in stage 80.

Each aircraft divides its damage among the ten existing cluster bombs, assigning any remainder one point at a time to the earliest drops. Distance from each impact reduces actual damage. If several aircraft hit the same mech, their damage adds up and can exceed 80%. [Design and rules (Korean)](../../game-design.md#적-공습기) · [Collision tests](../../../Tests/WorldSpecialTests.cpp).
