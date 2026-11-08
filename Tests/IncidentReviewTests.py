#!/usr/bin/env python3
"""Exercise the gate against isolated Git repositories and real file changes."""
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

TOOL = Path(__file__).resolve().parents[1] / 'tools/incident_review.py'


class IncidentReviewTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.git('init', '-q')
        self.git('config', 'user.name', 'Gate Test')
        self.git('config', 'user.email', 'gate@example.invalid')
        self.write('.agents/incident-review.json', json.dumps({'version': 1, 'always': ['instructions'], 'categories': {'instructions': ['docs/instructions.md'], 'code': ['docs/code.md'], 'graphics': ['docs/graphics.md']}}))
        for name in ('instructions', 'code', 'graphics'):
            self.write('docs/' + name + '.md', '# ' + name + '\nPreserve originals and verify the actual output.\n')
        self.write('Sources/test.cpp', 'original\n')
        self.git('add', '.')
        self.git('commit', '-qm', 'baseline')
        self.notes = self.root / '.git/notes.json'

    def tearDown(self):
        self.temp.cleanup()

    def git(self, *args):
        return subprocess.check_output(['git', *args], cwd=self.root, stderr=subprocess.STDOUT).decode()

    def write(self, name, data):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data)

    def run_gate(self, *args, ok=True):
        r = subprocess.run([sys.executable, str(TOOL), '--root', str(self.root), *args], capture_output=True, text=True)
        self.assertEqual(r.returncode == 0, ok, r.stdout + r.stderr)
        return r.stdout + r.stderr

    def begin(self, kinds=('code',)):
        args = ['read', '--task', 'Preserve behavior while adding a checked change']
        for kind in kinds:
            args += ['--category', kind]
        self.run_gate(*args)
        self.requirements = {'docs/' + k + '.md': ['Preserve original behavior and validate changed paths.'] for k in set(kinds) | {'instructions'}}
        self.notes.write_text(json.dumps(self.requirements))
        self.run_gate('acknowledge', '--notes', str(self.notes))

    def edit(self):
        self.write('Sources/test.cpp', 'checked change\n')
        self.git('add', 'Sources/test.cpp')

    def review_notes(self, result='pass'):
        return {name: [{'requirement': item, 'result': result, 'detail': 'Compared the exact final file and regression evidence.', 'evidence': ['Sources/test.cpp']}] for name, items in self.requirements.items() for item in items}

    def finish(self):
        self.run_gate('read', '--after')
        self.notes.write_text(json.dumps(self.review_notes()))
        self.run_gate('review', '--notes', str(self.notes))
        self.run_gate('check', '--staged')

    def test_missing_read_fails(self):
        self.edit()
        self.assertIn('Missing pre-work', self.run_gate('check', '--staged', ok=False))

    def test_late_read_fails(self):
        self.edit()
        self.assertIn('Changes already exist', self.run_gate('read', '--task', 'late', ok=False))

    def test_read_without_acknowledgement_fails(self):
        self.run_gate('read', '--category', 'code', '--task', 'test')
        self.edit()
        self.run_gate('check', ok=False)

    def test_missing_post_read_fails(self):
        self.begin(); self.edit()
        self.run_gate('check', ok=False)

    def test_missing_review_fails(self):
        self.begin(); self.edit(); self.run_gate('read', '--after')
        self.run_gate('check', ok=False)

    def test_complete_review_passes(self):
        self.begin(); self.edit(); self.finish()

    def test_changed_staged_code_invalidates_review(self):
        self.begin(); self.edit(); self.finish()
        self.write('Sources/test.cpp', 'later change\n'); self.git('add', '.')
        self.run_gate('check', '--staged', ok=False)

    def test_unstaged_edit_invalidates_review(self):
        self.begin(); self.edit(); self.finish()
        self.write('Sources/test.cpp', 'unstaged later change\n')
        self.run_gate('check', '--staged', ok=False)

    def test_changed_incident_requires_reread(self):
        self.begin(); self.edit(); self.finish()
        self.write('docs/code.md', '# Updated incident\nAdditional preservation requirement.\n')
        self.git('add', '.')
        self.run_gate('check', ok=False)
        self.finish()

    def test_new_graphics_requires_graphics_reading(self):
        self.begin(); self.edit()
        self.write('Art/test.png', 'new-art'); self.git('add', '.')
        self.assertIn('graphics', self.run_gate('read', '--after', ok=False))

    def test_graphics_and_code_reading_passes(self):
        self.begin(('code', 'graphics')); self.edit()
        self.write('Art/test.png', 'new-art'); self.git('add', '.')
        self.finish()

    def test_failed_finding_blocks(self):
        self.begin(); self.edit(); self.run_gate('read', '--after')
        self.notes.write_text(json.dumps(self.review_notes('failed')))
        self.run_gate('review', '--notes', str(self.notes), ok=False)

    def test_missing_evidence_blocks(self):
        self.begin(); self.edit(); self.run_gate('read', '--after')
        data = self.review_notes()
        next(iter(data.values()))[0]['evidence'] = ['missing.txt']
        self.notes.write_text(json.dumps(data))
        self.run_gate('review', '--notes', str(self.notes), ok=False)

    def test_evidence_change_invalidates_review(self):
        self.begin(); self.edit()
        self.write('.git/evidence.txt', 'first proof')
        self.run_gate('read', '--after')
        data = self.review_notes()
        for checks in data.values(): checks[0]['evidence'] = ['.git/evidence.txt']
        self.notes.write_text(json.dumps(data))
        self.run_gate('review', '--notes', str(self.notes))
        self.write('.git/evidence.txt', 'changed proof')
        self.run_gate('check', ok=False)

    def test_next_commit_requires_new_reading(self):
        self.begin(); self.edit(); self.finish()
        self.git('commit', '-qm', 'completed task')
        self.run_gate('check', ok=False)

    def test_policy_edit_requires_current_rereading(self):
        self.begin(); self.edit(); self.finish()
        policy = json.loads((self.root / '.agents/incident-review.json').read_text())
        policy['version'] = 2
        self.write('.agents/incident-review.json', json.dumps(policy))
        self.git('add', '.')
        self.run_gate('check', ok=False)
        self.finish()

    def test_instruction_analysis_without_edit_can_be_reviewed(self):
        self.begin(('instructions',)); self.finish()


if __name__ == '__main__':
    unittest.main()
