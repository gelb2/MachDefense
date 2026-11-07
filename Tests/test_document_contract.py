import importlib.util
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location('document_contract', Path(__file__).resolve().parents[1] / 'tools/document_contract.py')
contract = importlib.util.module_from_spec(spec)
spec.loader.exec_module(contract)


class DocumentLinksTest(unittest.TestCase):
    def test_nested_missing_file_and_section_are_detected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'docs/public').mkdir(parents=True)
            page = root / 'docs/public/install.md'
            (root / 'docs/policy.md').write_text('# 저장 정책\n')
            page.write_text('[missing](lost.md) [wrong](../policy.md#없는-절) [valid](../policy.md#저장-정책)')
            errors, count = contract.check_links(root, [page])
            self.assertEqual(count, 3)
            self.assertEqual(len(errors), 2)
            self.assertTrue(errors[0].startswith('Broken link:'))
            self.assertTrue(errors[1].startswith('Missing section:'))

    def test_fences_external_links_duplicates_and_encoded_targets(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            page = root / 'readme.md'
            (root / '한 글.md').write_text('# 저장\n## 저장\n')
            page.write_text('```md\n[example](missing.md)\n```\n[web](https://example.com)\n[mail](mailto:a@example.com)\n[local](%ED%95%9C%20%EA%B8%80.md#저장-1)\n')
            self.assertEqual(contract.check_links(root, [page]), ([], 1))

    def test_markup_and_local_anchor(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            page = root / 'readme.md'
            page.write_text('# v0.5 **현행** · 저장\n[ok](#v05-현행--저장)\n<a id="manual"></a>\n[ok](#manual)\n')
            self.assertEqual(contract.check_links(root, [page]), ([], 2))


if __name__ == '__main__':
    unittest.main()
