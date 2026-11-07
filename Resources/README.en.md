# Resources

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

MachDefense 1.0.3 game resources have been prepared for source builds. App builds read this folder. Preparation tools are not part of the build steps.

The counts below record the initial resource preparation. Subsequent artwork is recorded in the [art manifest](../Art/Level10/20261107/runtime-art.json).

| Path | Contents |
|---|---|
| `Images/` | 1,109 images: 948 CgBI PNGs converted to regular RGBA PNG; 161 JPEGs copied unchanged |
| `OriginalImages/` | Images before conversion, for pixel/hash comparison |
| `Audio/` | 77 original WAV files and 3 MP3 files |
| `RawData/` | 3 MCB files, 94 encrypted tables, and 1 plain table |
| `Tables/` | 94 UTF-8 TSV files. `MachBuild_Parts.tbl` and `.tble` decode identically and use one file |
| `Data/` | 3 JSON files recording MCB node addresses, types, raw fields, and child relationships |
| `Generated/` | ASCII glyph atlas and layout/width data for the original `TrebuchetMS-Bold` |

`inventory.json` links original resources to prepared-file SHA-256 hashes. `images.json` records dimensions, alpha, and conversion paths; `tables.json` records table headers and row counts. `preparation-validation.json` contains resource-check results.

Converted images, audio, tables, and the font atlas are included in Git. Do not run the following tools to build the app after cloning.

Commands for preparing original resources again:

```sh
python3 tools/prepare_resources.py --source /outside/repository/MachDefense.ipa
xcrun swift tools/prepare_font.swift Resources/Generated
```

The preparation tool checks the specified original file's SHA-256. It allows only game-resource extensions and does not copy the original IPA, executable, NIB, signatures, or ad SDKs. Keep originals outside the repository.

PNG conversion handles BGRA channels, premultiplied alpha, and Adam7 interlacing. Conversion results for 6 original interlaced images and 4 regular images were compared with macOS ImageIO; all RGBA pixels matched. Original RGB values in translucent pixels are already quantized in CgBI, so pre-conversion files are retained too.

Korean comments in 80 spawn tables use CP949 and were converted to UTF-8. The English description table is `Description_us.tsv`. This comment conversion does not change mission, weapon, or production values.

The glyph atlas uses `TrebuchetMS-Bold`, read from CFString `0x6aaac` in the reference ARMv7 executable. It was generated offline after checking the installed PostScript name. No font file is copied, and no glyph images are generated during gameplay. Original numeric-font images remain in `Images/` too.

## Level-10 extension resources

`tools/apply_level10.py` reads the reviewed `docs/research/level10/proposal.json` and adds level rows and references. It does not change the original `PartsInfo.mcb` or `MachInfo.mcb`. `RawData/Level10Parts.mcb` and `Level10Machines.mcb` are additional definitions generated in the same Chunk format. Original attachment points and sounds are reused. In v0.5, turret weapons at Lv.6–10 use original Lv.5 artwork; bodies at Lv.6–10 and base parts at Lv.8–10 use accepted new artwork.

`inventory.json`, `tables.json`, and `preparation-validation.json` record the initial extraction. Extended TSV hashes and row counts are recorded separately in `level10-extension.json`. Among original table values, only the next-upgrade price in 8 cap-level rows changes; other existing rows remain. `Tests/level10_tables_test.py` compares original encrypted tables with current TSVs.

After preparing original resources again, run `python3 tools/apply_level10.py`. This tool is also excluded from app execution and build steps.
