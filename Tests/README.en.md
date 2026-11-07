# Tests

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

Shared C++ game checks and resource/document-tool checks live here. They cover production, damage, upgrades, progression across 80 missions, saves, and recovery codes. Run commands are in the [shared core guide](../Sources/Core/README.en.md).

Platform rendering, input, audio, and lifecycle checks run through [Apple](../Apple/README.en.md), [Android](../Android/README.en.md), and [Web](../Web/README.en.md). Completed coverage and remaining checks are recorded in [validation results](../docs/en/public/validation.md).

Document links and Korean/English pairing checks:

```sh
python3 tools/document_contract.py check
python3 tools/check_translations.py
python3 -m unittest discover -s Tests -p 'test_*translations*.py'
```
