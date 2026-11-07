# Troubleshooting

<!-- language-nav:start -->
[한국어](../../public/troubleshooting.md) | [English](troubleshooting.md)
<!-- language-nav:end -->

## Refusing a drawing-edit instruction without explaining the cause

Codex refused the user's work instructions without knowing why the request had been blocked. Codex also refused the instruction to remove or modify the text in the user's supplied sketch and create the requested drawing. It did not call a tool to remove or modify the text, and it did not create the requested drawing. The image tool returned `moderation_blocked / violence` for Cheyenne without providing the specific cause. When the user instructed it to remove all text from the drawing and continue, Codex interpreted that instruction as circumventing the block and refused.

The draft incident report also presented Codex's own refusal as a claim made by the user. Codex corrected that wording after the user objected again. Codex did not actually call a tool to remove the text. There was no tool result rejecting that edit itself. It repeatedly said it did not know the cause and acknowledged its response was wrong, but did not perform the instructed edit. No text-free drawing or new Cheyenne artwork was produced, and the AH-10 proportion discrepancy remained. The [incident report](../../evidence/aircraft-instruction-refusal-20261108.md) records the user's instructions, the actual replies, the error and the unfinished work.

### Refusal to use the Gemini drawing and perform the follow-up edit

Codex refused the instruction to work from the Gemini-generated helicopter drawing supplied by the user. It also refused the follow-up instruction to delete or modify the text in that sketch and create the requested drawing. Without establishing the actual cause of the block, Codex interpreted the follow-up edit as an attempt to circumvent it. It did not call a text-editing tool or create the requested drawing. Its subsequent draft incident report presented its own refusal as the user's claim, shifting responsibility to the user.

## Errors in new unit artwork

Codex said it would apply the approved designs, then substituted other artwork or used the wrong body-size boundary. It nevertheless reported the approved appearance as applied. The user identified repeated disregard of approval instructions and false completion reports. This record explicitly documents both failures.

Codex produced level-extension turrets with colors, surfaces and weapon proportions that differed from the originals. During helicopter and VTOL work, it replaced approved artwork with another generated image and used the wrong reference mech level and the wrong boundary for the gunship body. The user repeatedly pointed out differences in color, texture, size, rotors and banking in installed builds and previews. Muzzles also remained inside barrels after resizing, and production icons overlapped price labels.

Behavior tests and whole-image dimension checks missed the incorrect central fuselage size. The latest web cheat capture again showed that the target width included the gunship's side supports, leaving its central body too small. The visual correction is unfinished.

A request to regenerate the central body returned `HTTP 400`, `moderation_blocked` and `violence` in a single error response. It was blocked at the input stage; the specific triggering input was not disclosed. The [incident record, capture and original response](../../evidence/new-unit-art-errors-20261108.md) preserve the details in Korean with the raw error JSON.

## Frame drops during machine-gun fire

On iPad, the screen stuttered when the base machine gun fired. Enemy tank shells did not show the same stutter. I instructed Codex to check whether firing wrote save files, repeatedly allocated projectiles, or recreated assets, and to measure it with Instruments.

Codex's Apple audio code set the OpenAL context for each sound effect, then restored the previous context on return. The previous context was empty after initialization, so each shot attached and detached the context. The original kept its initialized context active.

Time Profiler showed `MDAudio play → Scope → AudioUnitGraph → RemoteIO Start/Stop`. The game's main thread waited for system responses when starting and stopping audio output. The handling added to repeated gunfire delayed screen updates too.

The fix keeps the context active while the game is active and avoids setting it again when already attached. Effect buffers and playback sources are reused. After the fix, a 248.82-second, 14,912-frame run recorded 644 effects. Total audio-call time in frames with effects had a p99 of 0.204ms and a maximum of 0.340ms. Game-save writes: 0.

7 frames still exceeded 25ms. The longest, 93.083ms, included first uploads of 35 textures; audio calls took 0.135ms. This initial resource-preparation delay was recorded separately from repeated-fire stutter. [Measurements and fix](../../troubleshooting.md#기지-기관총-효과음마다-오디오-출력을-중단한-오류--2026-11-04)

## Audio-session calls on the main thread

Even after fixing the firing path, Codex still called `AVAudioSession.setCategory` and `setActive` on the main thread from screen attachment, activation changes, and audio-interruption handling in `MDSceneDelegate` and `MDGameView`. Input and drawing could wait together for the OS audio service. Xcode reported main-thread warnings.

Session configuration, activation, and deactivation moved to a serial queue. Requests carry generation numbers so a late activation cannot overwrite a newer inactive state. Players stop before session deactivation; music and effects resume after successful activation. Game input does not wait for session completion.

On a physical iPad, 6 audio, resume, and combat checks passed with 0 runtime warnings. Across 1,800 frames covering idle, machine-gun fire, upgraded weapons, and simultaneous combat, frame-interval p99 was 16.667ms, with 0 frames above 25ms. [Main-thread fix](../../evidence/audio-session-20261105.md)

## Missing effects after returning from the background

In that build, returning from the background still restored only music; machine-gun and production-completion sounds were silent. I found this while playing.

Codex used `alcSuspendContext` and `alcProcessContext` as though they stopped and restarted output. Under the OpenAL Soft configuration in use, both functions returned immediately without restarting the output device. `alSourcePlay` calls increased, but effect playback did not advance. The earlier test checked only call counts and incorrectly passed this state.

Reading effect `AL_SEC_OFFSET` reproduced the failure. After resume, the machine-gun and production-completion playback positions stayed at 0 seconds. After connecting `alcDevicePauseSOFT` and `alcDeviceResumeSOFT`, the same check showed positions advancing by 0.192–0.213 seconds after a 0.2-second wait. [Reproduction and fix](../../evidence/effects-resume-20261105.md)

## Missing prompts and effects, and incorrect icons

Turret-placement prompts, dimming of unaffordable production icons, production-completion sounds, next-mission illustrations, and several panel sounds were missing. Checks focused on production, victory/defeat, and saves had not verified those outputs.

Production/reward icon orientation and completion labels were also wired incorrectly. Codex compared conditions, resources, coordinates, and effect calls in original functions against the new code and corrected them. Base upgrades were changed to match the original: health and weapons apply first, while the body and radar update in the next stage. [Visual/audio fixes](../../evidence/source-ui-audio-lifecycle-20261104.md) · [Upgrade appearance and resume handling](../../evidence/base-audio-followup-20261104.md)

## Android rendering and audio errors

The first Android implementation needed fixes for system-bar configuration before Surface creation, access to released buffers during audio resume, and connector parts without images. Changes were confined to the Android host and Android build output. Existing Apple and shared game code were unchanged. [Android implementation record](../../evidence/android-native-20261106.md)

## Web saves and offline checks

The web version waits for save completion, rejects writes from stale tabs, and backs up saves before restoration. Closed-database write failures and invalid recovery codes were checked for preservation of existing progress. Corrupt PWA resources trigger a download retry instead of installation success. Progress preservation after resource-cache deletion/updates and offline relaunch were also checked. [Web checks](../../evidence/web-pwa-20261106.md)

[Game overview](../../../README.en.md) · [Documentation](README.md)

## PWA installation address

The first GitHub Pages installation check assigned the site root as the app identity instead of `/MachDefense/`. The manifest's relative `id` was resolved against the origin. Removing the explicit id made the game start address the installation identity. [Live-deployment checks](../../evidence/pwa-release-20261106.md).

[How I directed the work](work-method.md) · [Testing with cheat builds and direct play](testing-method.md)
