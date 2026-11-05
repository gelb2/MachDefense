import importlib.util
from pathlib import Path
import tempfile
import unittest

spec=importlib.util.spec_from_file_location('document_contract',Path(__file__).resolve().parents[2]/'tools/document_contract.py')
contract=importlib.util.module_from_spec(spec)
spec.loader.exec_module(contract)

class DocumentContractTests(unittest.TestCase):
    def test_nested_report_missing_asset_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);report=root/'docs/editorial/report.md'
            report.parent.mkdir(parents=True);report.write_text('![capture](assets/missing.webp)\n')
            self.assertIn(report,contract.maintained_documents(root))
            self.assertEqual(len(contract.check_links(report)),1)
            asset=report.parent/'assets/missing.webp';asset.parent.mkdir();asset.write_bytes(b'fixture')
            self.assertEqual(contract.check_links(report),[])

    def test_renamed_korean_heading_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);target=root/'todo.md';target.write_text('## 이전 작업 순서 — 검사\n')
            doc=root/'readme.md';doc.write_text('[작업](todo.md#현재-작업-순서--검사)')
            self.assertEqual(len(contract.check_links(doc)),1)
            doc.write_text('[작업](todo.md#이전-작업-순서--검사)')
            self.assertEqual(contract.check_links(doc),[])

    def test_duplicate_explicit_and_code_headings(self):
        anchors=contract.anchors('# Same\n## Same\n## 한국어 {#manual}\n```md\n# Fake\n```\n<a id="html-id"></a>')
        self.assertTrue({'same','same-1','한국어','manual','html-id'}<=anchors)
        self.assertNotIn('fake',anchors)

    def test_external_urls_and_example_code_are_not_local_files(self):
        with tempfile.TemporaryDirectory() as directory:
            doc=Path(directory)/'readme.md'
            doc.write_text('[site](https://example.com/a#b)\n[mail](mailto:a@example.com)\n```md\n[example](missing.md)\n```\n')
            self.assertEqual(contract.check_links(doc),[])

    def test_preserved_upstream_is_not_rewritten(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            for relative in ['docs/reference/heavymach1/a.md','Modules/ThirdParty/a.md','docs/evidence/ours.md']:
                p=root/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('# Text')
            self.assertEqual(contract.maintained_documents(root),[root/'docs/evidence/ours.md'])

if __name__=='__main__':unittest.main()
