# Apple

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

`MachDefense.xcworkspace` connects the universal iPhone/iPad app to the shared core and resource projects. The regular app uses `com.gelb2.machdefense.source`; the testing app uses `com.gelb2.machdefense.source.cheat`. Both are separate from the compatibility port and its saves.

Use an Apple Silicon Mac, Xcode with iOS Simulator, Python 3, and CMake 3.20 or later. During the first build, `tools/build_openal.py` compiles the included OpenAL Soft source. Homebrew users can prepare with `brew install cmake`. XcodeGen is not required to open and build the existing projects.

Regenerate the projects:

```sh
Apple/generate-projects.sh
```

Open the workspace in Xcode and select `MachDefense` or `MachDefenseCheat` with an iPhone/iPad simulator. The product uses the `createGame` implementation in `Sources/Core/Contracts/Game.hpp` and the prepared `Resources`. The IPA and compatibility runtime are not build inputs.

`Sources/Platform/Apple` handles OpenGL ES 1.1 texture rendering, input-coordinate conversion, the audio session, PCM loading, BGM, UIKit windows and lifecycle, name/recovery-code text entry, and explicit atomic file saves. Game buttons and text are drawn from the sprite list produced by Core. UIKit does not create game buttons.

Sound effects use the port's OpenAL Soft 1.22.2 through `MachDefenseAudio`. Apple's system OpenAL.framework is not linked. The original PCM format and positional attenuation are used.

The `MachDefenseRendererTests` scheme checks image orientation, cropping, alpha, clipping, input conversion, and texture reuse in simulator OpenGL ES. MenuRenderTests connects the shared `Menu` to real resources, renders the title, English profile creation, and stage selection, and saves PNGs. AudioTests checks 77 WAV effects, looping effects, and loop/pause/resume behavior for 3 music tracks.

The 3 MP3 music tracks use AudioFile/AudioQueue, as in the original. Requesting the same track does not restart it; deactivating the app pauses it. Recreating the audio device restores the track name and volume.

FrameLayout, BattleGeometry, and GameHUD expand the iPhone/iPad display area. Device screen and input conversion follows the [coordinate policy](../docs/coordinate-policy.md). An original piece covers the black exposed area above the iPhone HUD. HUD visual redesign is on hold; title-edge improvements remain.

A process sample investigating slow iPad simulator execution showed `GLRendererFloat` drawing pixels on the CPU. Physical iPad performance is measured separately on the device.

The testing app's `TEST` screen uses the same game core and save structure. Create an empty test slot, then select mission, credits, upgrades, equipment, special attacks, and base-destruction conditions. The regular app has no TEST entry button. Direct input checks use Device Hub to view the screen; captures use Device Hub or device-capture tools. Simulator screenshots use `xcrun simctl io <simulator UUID> screenshot <PNG path>`. Check the save, close only the testing app, and confirm the home screen.

The renderer's `lastFrameMetrics` retains the last frame's sprite count, draw count, texture/filter/blend/clip changes, and CPU durations of submission/presentation calls in memory. It does not write files. These timings add no separate wait for GPU completion. Original projectile trails are triangle strips interleaved with sprites according to `TextureMesh.beforeSpriteIndex`. `RendererTests` checks trail/foreground ordering and color-state restoration.

## Fast automated functional checks

```sh
python3 tools/run_apple_sweep.py --simulator <시뮬레이터-UDID>
```

Run from the repository root. It connects 80-mission results and saves, purchases at every upgrade level, all current weapon-table definitions, production, and special attacks to the actual Apple renderer and audio. It also includes a 1,800-frame CADisplayLink check. `--mode soak` checks 10 minutes of combat, 20 entries, and audio/render recovery. [Earlier checks](../docs/evidence/apple-automated-sweep-20261104.md), [All upgrades, weapons, and device measurements](../docs/evidence/full-upgrade-weapon-device-20261104.md).

For a physical device, select `MachDefenseRendererTests` and the connected device in Xcode, then run Test. The existing cheat app is the XCTest host; tests avoid initializing normal game-screen audio twice. Saves use a temporary directory. After testing, use Clean Build Folder and build the `MachDefense` and `MachDefenseCheat` schemes separately. Install the final apps without the test plugin.
