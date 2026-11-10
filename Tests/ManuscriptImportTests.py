import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("manuscript_import", Path(__file__).resolve().parents[1] / "tools/import_saved_manuscript.py")
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class ManuscriptImportTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.target = self.root / module.DOCUMENTS["article-ko"]
        self.target.parent.mkdir(parents=True)
        self.target.write_bytes("처음 원문\n".encode())
        (self.root / "docs/editorial").mkdir()
        self.source = hashlib.sha256(self.target.read_bytes()).hexdigest()

    def tearDown(self):
        self.temporary.cleanup()

    def payload(self, content='수정 "원문"\n\n`코드`\n', version=1):
        return {"documentId": "article-ko", "repository": "gelb2/MachDefense", "path": module.DOCUMENTS["article-ko"], "version": version, "content": content, "contentSha256": hashlib.sha256(content.encode()).hexdigest(), "sourceSha256": self.source, "sourceCommit": "0" * 40, "savedAt": "2026-11-10T00:00:00Z"}

    def test_exact_text_dry_run_then_apply_and_next_revision(self):
        payload = self.payload()
        self.assertEqual(module.import_revision(payload, self.root)["status"], "ready")
        self.assertEqual(self.target.read_text(), "처음 원문\n")
        self.assertEqual(module.import_revision(payload, self.root, True)["status"], "applied")
        self.assertEqual(self.target.read_bytes(), payload["content"].encode())
        self.assertEqual(module.import_revision(payload, self.root, True)["status"], "unchanged")
        next_payload = self.payload("두 번째 수정\n", 2)
        module.import_revision(next_payload, self.root, True)
        self.assertEqual(self.target.read_text(), next_payload["content"])
        with self.assertRaises(ValueError):
            module.import_revision(payload, self.root, True)

    def test_external_change_is_preserved(self):
        self.target.write_text("다른 곳에서 수정한 원고\n")
        with self.assertRaises(ValueError):
            module.import_revision(self.payload(), self.root, True)
        self.assertEqual(self.target.read_text(), "다른 곳에서 수정한 원고\n")
        self.assertFalse((self.root / module.STATE_PATH).exists())

    def test_wrong_path_repository_hash_and_unsaved_revision_are_rejected(self):
        for changes in [{"path": "../../AGENTS.md"}, {"repository": "another/repository"}, {"contentSha256": "f" * 64}, {"version": 0}, {"version": True}]:
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                module.import_revision({**self.payload(), **changes}, self.root, True)
        self.assertEqual(self.target.read_text(), "처음 원문\n")

    def test_symlink_target_is_rejected(self):
        other = self.root / "other.md"
        other.write_text("보존")
        self.target.unlink()
        self.target.symlink_to(other)
        with self.assertRaises(ValueError):
            module.import_revision(self.payload(), self.root, True)
        self.assertEqual(other.read_text(), "보존")


if __name__ == "__main__":
    unittest.main()
