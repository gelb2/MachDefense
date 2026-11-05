# Third-party sources

- `stb/`: PNG/JPEG loader retained from HeavyMach, original dual MIT/public-domain notice in the header.
- `openal-soft/`: OpenAL Soft 1.22.2, copied byte for byte from MachDefense's tested touchHLE host. `openal-soft-source.json` records every source file hash. LGPL terms remain in `openal-soft/COPYING`; per-file notices are preserved.

`tools/build_openal.py` builds arm64 libraries for the selected Apple SDK, using the host's static-library configuration and CoreAudio output. Generated libraries stay in `.local/openal/`. No game package or extracted original executable is a build input.
