# Creators and resources

<!-- language-nav:start -->
[한국어](CREDITS.md) | [English](CREDITS.en.md)
<!-- language-nav:end -->

## Original game

MachDefense's artwork, music, and game design are the work of its original creators. This project restores the game and makes it playable on other platforms.

## Restoration and additions

I played the game myself, set the restoration's direction, preservation requirements, and new features, and directed fixes for incorrect behavior. Codex implemented the new source and platform connections.

The work followed HeavyMach1's restoration procedure and skills, shared-code/platform-host structure, and web/PWA expansion. Records of HeavyMach1's external web contribution and subsequent changes remain in that repository. [HeavyMach1](https://github.com/gelb2/HeavyMach)

## An earlier restoration attempt

[mesabloo](https://github.com/Mesabloo/hm-defense) attempted to restore MachDefense before I did. His attempt had a major influence on the HeavyMach1 and MachDefense restorations.

## Libraries

OpenAL Soft plays sound effects; stb decodes images. Their sources and licenses are in `Modules/ThirdParty/`. [Third-party notices](THIRD_PARTY_NOTICES.en.md) · [Code license](LICENSE-SCOPE.en.md) · [Asset terms](ASSET-LICENSES.en.md).

Thank you to [fastutlego](https://github.com/fastutlego) for contributing the HeavyMach web port. The WebAssembly host, WebGL2 renderer, input, audio and saves in [HeavyMach PR #24](https://github.com/gelb2/HeavyMach/pull/24), together with the subsequent PWA and Tauri work, also laid the groundwork for bringing MachDefense to the web and desktop. I used that architecture for the MachDefense PWA and used the same web host in the Tauri Mac app.

[Game overview](README.en.md) · [Documentation](docs/en/public/README.md)
