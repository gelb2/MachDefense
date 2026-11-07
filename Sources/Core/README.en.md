# MachDefense shared core

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

C++20 game code. It does not call Apple SDKs, OpenAL, or graphics APIs directly.

| Folder | Responsibility |
|---|---|
| Contracts | Platform connections, frame commands, coordinate contracts, and original random-number generation |
| Data | Tables and MCB resource loading |
| Simulation | Movement, collisions, targeting, range, projectiles, damage, production, spawning, and effect state |
| Session | Profiles, menu transitions, save serialization, and rewards |
| Presentation | Menus, HUD, buttons, fonts, and world drawing commands |
| Validation | Cheat-condition preparation and testing panels |

ResourceAccess and HostServices in `Contracts/Game.hpp` connect platforms. Execution starts with createGame in `Session/GameSession.cpp`. `Simulation/World.cpp` controls world-update order; WorldTargeting handles targets, WorldMovement movement, WorldTerrain terrain, and WorldCombat firing and damage. MachineModels supplies part assembly and muzzle calculations.

```sh
cmake -S . -B .local/source-build -DCMAKE_BUILD_TYPE=Release
cmake --build .local/source-build
ctest --test-dir .local/source-build --output-on-failure
```

Build and test with the prepared Resources. [Current status](../../docs/current-status.md), [Before/after module comparison](../../docs/evidence/module-work-20261105.md).
