import hashlib
import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from check_translations import check


class TranslationChecks(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        (self.root / 'docs/translations').mkdir(parents=True)
        self.ko = '# 안내\n\n<!-- language-nav:start -->\n[한국어](README.md) | [English](README.en.md)\n<!-- language-nav:end -->\n\n```sh\nbuild game\n```\n'
        self.en = self.ko.replace('# 안내', '# Guide')
        self.write()

    def write(self):
        (self.root / 'README.md').write_text(self.ko)
        (self.root / 'README.en.md').write_text(self.en)
        pair = {'ko': 'README.md', 'en': 'README.en.md'}
        (self.root / 'docs/translations/scope.json').write_text(json.dumps({'pairs': [pair]}))
        record = dict(pair, ko_sha256=hashlib.sha256(self.ko.encode()).hexdigest(), en_sha256=hashlib.sha256(self.en.encode()).hexdigest())
        (self.root / 'docs/translations/manifest.json').write_text(json.dumps({'pairs': [record]}))

    def test_valid_pair(self):
        self.assertEqual(check(self.root)[0], [])

    def test_source_change_requires_review(self):
        (self.root / 'README.md').write_text(self.ko + 'Changed\n')
        self.assertTrue(any('Review required' in e for e in check(self.root)[0]))

    def test_missing_translation(self):
        (self.root / 'README.en.md').unlink()
        self.assertTrue(any('Missing translation' in e for e in check(self.root)[0]))

    def test_wrong_language_navigation(self):
        self.en = self.en.replace('[English](README.en.md)', '[English](README.md)')
        self.write()
        self.assertTrue(any('Incorrect language' in e for e in check(self.root)[0]))

    def test_english_route_and_missing_anchor(self):
        self.en += '[Back](README.md) [Section](README.en.md#absent)\n'
        self.write()
        errors = check(self.root)[0]
        self.assertTrue(any('English link points' in e for e in errors))
        self.assertTrue(any('Missing section' in e for e in errors))

    def test_executable_command_must_match(self):
        self.en = self.en.replace('build game', 'delete game')
        self.write()
        self.assertTrue(any('blocks differ' in e for e in check(self.root)[0]))


if __name__ == '__main__':
    unittest.main()
