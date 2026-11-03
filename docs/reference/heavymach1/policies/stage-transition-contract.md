# Stage transition contracts

This is isolated transition-contract coverage, not a GameSession journey, enemy defeat, map clear or continuous campaign. `Tests/StageTransitionTests.cpp` calls production Motion, World and GlobalInfo functions with explicit initial boundary/state fixtures. It adds no game API and changes no damage, HP rule, map or enemy implementation. The later camera-inclusive journey result is23/35; earlier27/35 used zero camera offset. See [journey conditions](stage-journeys.md).

## Independent original oracle

The test contains41 literal rows: current mission/stage/map, expected next mission/stage/map, and missionEnd. Expected rows are not calculated by calling the production next-stage function. The original evidence is:

| Original address | Contract |
| --- | --- |
| GlobalInfo.init0xfd44 | mission lengths4/5/5/5/5/5/6/6 and41 map indices; maps25..29 precede15..24 |
| GobHvMach.tick0x479c | strict exit X>right−30; positive speed; player; visible/not paused; state not3; mode not1; bossCount<1; frontLine gating occurs first |
| GobWorld.makeEnemy0xd6b8, killBoss0xd848 | encounter count and frontLine barrier; final encounter removal restores right+10 |
| GobWorld.setNextStage0xb8f0 | increment stage, mission end, loading asset, then unconditional mission>6 clamp to6/0; save request |
| GobWorld.handleButton0xe7e0 | ready step2 and gameState nonzero; buttons1000/1001;1000 forces missionEnd |
| GobWorld.processStageClear0xc228 | loading/result phases, clearing, menu4 or mapped next-stage open, fade/unpause |
| GlobalInfo.updateLastStage0x10a0c | forward-only saved progression; replay/clamp cannot rewind saved farthest |

Source exports are `Analysis/functions/<address>.pseudo.c` and `.asm`. The loading and final clamp ordering was reread directly from0xb8f0; its lines143..152 clamp before updateLastStage. The result button export confirms1001 does not clear an existing missionEnd flag.

Normal mission end transitions are Stage04→05,09→10,14→25,29→15,19→20,24→30,35→30. The final Stage35 transition clamps to mission6/stage0 but retains missionEnd=true and returns menu4. Its saved farthest/current position do not rewind from the unlocked final stage.

Mission7/direct-only maps36..41 are reported separately. Because the original clamp is unconditional, maps36..40 transition to mission6/stage0/Stage30 with missionEnd=false; map41 does so with missionEnd=true. This is not evidence that normal gameplay reaches these six maps.

## Tested consumers

Each row loads its real original map data and dimensions. Motion fixtures use that map's actual right boundary and test the immediately adjacent representable floats below/at/above right−30. They also test positive/zero/negative speed; HP positive/zero/negative; model/land/fly/terminal states; player/nonplayer; visible/hidden; pause; scenario/survival; positive/zero/negative boss count; and encounter/frontLine behavior with two explicit boss-state fixtures.

HP is deliberately not invented as a Motion exit gate: the original function does not read it at this decision. An unpaused HP0 fixture can emit NextStage; a real player's GameOver caller pauses the world separately. This test does not describe that fixture as a living player or override the caller's death handling. Likewise, bossEncounter alone is not a direct Motion condition. The production spawn/kill methods change frontLine, which blocks exit while an encounter remains. The original bossCount may become negative; the test preserves it.

The recording host receives real World events. SaveProgress calls production GlobalInfo.updateLastStage. OpenStage resolves the production mapping and calls World.openMap on the actual next map, then checks the expected map bounds and player-spawn callback. The host does not implement a parallel copy of the state machine. The literal expected table checks the mapping independently.

Checks include:

- RemoveLoadingScreen→TitleIn→SaveProgress order; pause/gameState and persistent world time.
- Result not ready before13 presentation ticks; single quake/title sound at readiness.
- Invalid/early/duplicate button rejection;1001 preserving missionEnd;1000's explicit menu alternative on every row.
- ClearObjects then ClearTileResources; menu4 on mission end with no OpenStage.
- In-mission mapped OpenStage, player creation, fade completion, unpause and no menu event.
- Forward save advancement, final/direct clamp non-rewind, actual next-map generation callback.

## Verification and limits

Final normal build/run: exit0, **35 normal +6 direct-only transition contracts,820 Motion gate assertions**. `build/stage-transition/contracts-final.jsonl` explicitly marks every row `journey_clear:false`. The test uses runtime checks that remain active with NDEBUG.

The parent owns CMake and added `HeavyMachStageTransitionTests` linked to HeavyMachCore. Direct evidence is under `build/stage-transition/`: build/run logs, row results and source/evidence hashes. The full production core was also rebuilt with `-O1 -ffp-contract=off -fsanitize=address,undefined`; that run exited0 without diagnostics and produced byte-identical41 result rows. Logs are `build-asan.log` and `run-asan.log`; `manifest.json` records source/evidence hashes.

This does not cover actual combat success, UI hit testing, exact rendered transitions, file-system persistence, a continuous campaign, or real-device execution. Loading callbacks and in-memory save rules are covered; native menu construction and disk writes are outside this host. No new original-code mismatch was found in the tested contracts.
