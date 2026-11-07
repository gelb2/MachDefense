# Restoration and development

<!-- language-nav:start -->
[한국어](../../public/development-history.md) | [English](development-history.md)
<!-- language-nav:end -->

## Where I found the original

When I began restoring HeavyMach1, Google's search Gemini pointed me to the [Internet Archive](https://archive.org). I downloaded the original from its [HeavyMach series collection](https://archive.org/details/heavy-mach-archive_202406). The MachDefense original came from the same collection.

## Comparing the original runtime with new source

First, the original ran through a compatibility host. On my iPhone, I checked production, combat, destruction of the enemy base, victory, defeat, and returning to the mission menu. Codex then connected the original resources to new C++ code. Production costs, upgrades, movement, saves, visuals, and audio were compared under the same conditions.

Differences I found while playing the installed app were also corrected. Prices and production prompts were missing, and unit icons faced right. Machine-gun fire caused frame drops, and sound effects stopped after resuming. Output and lifecycle conditions missing from the earlier tests were added. [Troubleshooting](troubleshooting.md)

## Automated runs and the cheat app

I instructed Codex to use automated runs and a cheat app to compare the port with the redevelopment. I wanted it to change funds, unlocks, upgrades, and mission conditions directly, creating the conditions needed to test every feature without playing through the prerequisites.

The port and the new source version received the same test save. After purchasing 6 base upgrades, 4 initial unit types, and 3 special attacks, their prices, balances, levels, equipment order, and saved values were compared. These observations became expected values in the automated checks. [Port comparison](../../evidence/port-source-parity-20261104.md)

The rebuilt game's TEST screen controls profiles, all 80 missions, credits and cells, equipment levels and installation, special-attack quantities, allied and enemy spawns and health, and base destruction. It runs the same game code as the regular app with separate storage.

Automated checks ran through defeat, retry, victory, rewards, the next mission, and save reloads across 80 missions. Combat checks using level-10 equipment and cell replenishment destroyed the enemy base in all 80 missions through actual unit AI, projectiles, and damage. They exercised 110 production definitions and 27 special-attack levels at phone and tablet screen sizes. Physical-device checks covered input, frames, audio, and app resume. [Cheat app and campaign checks](../../evidence/source-cheat-campaign-20261104.md) · [Apple automated checks](../../evidence/apple-automated-sweep-20261104.md)

## Modules and Android

Core contains combat, movement, production, upgrades, progression, saves, menus, and HUD. Sound-effect handling moved into Audio. Platforms render drawing commands and connect OS input, audio, and file storage. Results before and after the split were compared across 160 conditions and 144,000 ticks.

Android added a Kotlin/JNI host. During that work, 2,978 existing Apple, shared-code, resource, and test files retained identical hashes. An iPhone save at the start of Stage 33 was transferred to Galaxy, where equipment, upgrades, funds, and persistence after relaunch were checked. [Android record](../../evidence/android-native-20261106.md)

## Web / PWA

The same C++ was built with Emscripten and connected to WebGL2 rendering, browser input and audio, IndexedDB saves, and PWA caching. The structure and procedure followed HeavyMach1's expansion from a contributed web version to PWA and Tauri.

Connecting the recovery codec and menu added service contracts, buttons, and wiring to 4 existing files. Game calculations and existing platform hosts were retained. Native results were compared before and after the changes; native and Wasm builds also produced matching game states and save results. [Web implementation and checks](../../evidence/web-pwa-20261106.md)

HeavyMach web contributor [fastutlego](https://github.com/fastutlego) provided the WebAssembly runtime in [PR #24](https://github.com/gelb2/HeavyMach/pull/24). That contribution and the later PWA/Tauri integration also gave MachDefense a basis for its web and Mac hosts. Thank you for making that expansion possible.

[Game overview](../../../README.en.md) · [Documentation](README.md)
