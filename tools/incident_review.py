#!/usr/bin/env python3
"""Require incident reading, task constraints and evidence-backed final review.

Records live in this worktree's Git metadata, never in published game assets.
The gate validates records and file identity, not a person's understanding.
"""
import argparse
import hashlib
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path

POLICY = '.agents/incident-review.json'


def git(root, *args):
    return subprocess.check_output(['git', *args], cwd=root).decode().strip()


def digest(data):
    return hashlib.sha256(data).hexdigest()


def sha(path):
    return digest(path.read_bytes())


def fail(message):
    raise ValueError(message)


def state_path(root):
    p = Path(git(root, 'rev-parse', '--git-path', 'incident-review-state.json'))
    return p if p.is_absolute() else root / p


def changed(root, staged=False):
    args = ['diff', '--name-only', '-z', '--no-renames']
    args += ['--cached'] if staged else ['HEAD']
    names = subprocess.check_output(['git', *args], cwd=root).decode().split('\0')
    if not staged:
        names += subprocess.check_output(['git', 'ls-files', '--others', '--exclude-standard', '-z'], cwd=root).decode().split('\0')
    return sorted(set(n for n in names if n))


def categories_for(paths):
    kinds = {'instructions'}
    for name in paths:
        p = Path(name)
        if (name.startswith(('Sources/', 'Modules/', 'Tests/', 'tools/', '.githooks/', 'Apple/', 'Android/', 'Web/', 'Distribution/', 'Desktop/', 'Compatibility/'))
                or p.suffix.lower() in {'.cpp', '.hpp', '.h', '.c', '.mm', '.m', '.py', '.js', '.mjs', '.sh', '.swift', '.kt', '.cmake'}
                or p.name == 'CMakeLists.txt' or name == POLICY):
            kinds.add('code')
        if (name.startswith(('Art/', 'Resources/Images/', 'Resources/RawData/'))
                or p.suffix.lower() in {'.png', '.webp', '.jpg', '.jpeg', '.svg', '.psd', '.blend', '.gif'}
                or 'render' in name.lower() or name.startswith('Sources/Core/Presentation/')):
            kinds.update(('graphics', 'code'))
    return kinds


def reports_for(policy, categories):
    unknown = set(categories) - policy['categories'].keys()
    if unknown:
        fail('Unknown category: ' + ', '.join(sorted(unknown)))
    return list(dict.fromkeys(p for c in sorted(set(categories) | set(policy['always'])) for p in policy['categories'][c]))


def fingerprint(root):
    files = {}
    for name in changed(root):
        p = root / name
        if p.is_symlink():
            files[name] = 'symlink:' + str(p.readlink())
        elif p.is_file():
            files[name] = sha(p)
        else:
            files[name] = 'deleted'
    return {'index_tree': git(root, 'write-tree'), 'files': files}


def load_state(root):
    if not state_path(root).is_file():
        fail('Missing pre-work incident reading. Run read before starting work.')
    return json.loads(state_path(root).read_text())


def save(root, state):
    path = state_path(root)
    temp = path.with_suffix('.tmp')
    temp.write_text(json.dumps(state, ensure_ascii=False, indent=2) + '\n')
    temp.replace(path)


def validate_read(root, policy, state):
    if state.get('base_head') != git(root, 'rev-parse', 'HEAD'):
        fail('Reading belongs to another commit. Start the next work item with read.')
    if state.get('phase') == 'presented' and state.get('policy_sha256') != sha(root / POLICY):
        fail('Incident routing changed before acknowledgement. Read again.')
    required = list(state['reports'])
    for name in required:
        # Keep the pre-work version even when the task edits this incident.
        try:
            before = subprocess.check_output(['git', 'show', state['base_head'] + ':' + name], cwd=root, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError:
            fail('Missing pre-work incident: ' + name)
        if state.get('reports', {}).get(name) != digest(before) or not (root / name).is_file():
            fail('Unread or missing incident: ' + name)
    return required


def show_reports(root, names):
    for name in names:
        print('\n' + name + '\n' + (root / name).read_text())


def requirements_valid(data, names):
    if set(data) != set(names):
        fail('Provide requirements for every listed incident, with no missing entries.')
    for name, items in data.items():
        if not isinstance(items, list) or not items or any(not isinstance(s, str) or len(s.strip()) < 8 for s in items):
            fail('Write concrete task requirements for: ' + name)


def evidence_file(root, name):
    path = Path(name)
    if path.is_absolute() or '..' in path.parts:
        fail('Evidence must be a repository-relative file: ' + name)
    path = root / path
    if not path.is_file() or not path.resolve().is_relative_to(root.resolve()):
        fail('Missing evidence file: ' + name)
    return sha(path)


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    p.add_argument('action', choices=['plan', 'read', 'acknowledge', 'review', 'check'])
    p.add_argument('--category', action='append', default=[])
    p.add_argument('--task')
    p.add_argument('--notes', type=Path)
    p.add_argument('--after', action='store_true')
    p.add_argument('--staged', action='store_true')
    a = p.parse_args(argv)
    root = a.root.resolve()
    policy = json.loads((root / POLICY).read_text())
    categories = set(a.category) | set(policy['always'])
    if a.action == 'plan':
        categories |= categories_for(changed(root, a.staged))
        print('\n'.join(reports_for(policy, categories)))
        return
    if a.action == 'read' and not a.after:
        # A new start cannot retroactively certify edits already made.
        if changed(root):
            fail('Changes already exist. Preserve them and start in a clean worktree; do not certify them retrospectively.')
        if not a.task or not a.task.strip():
            fail('Describe the work with --task.')
        names = reports_for(policy, categories)
        for name in names:
            if not (root / name).is_file():
                fail('Missing incident: ' + name)
        show_reports(root, names)
        state = {'version': 1, 'task': a.task, 'categories': sorted(categories),
                 'base_head': git(root, 'rev-parse', 'HEAD'), 'policy_sha256': sha(root / POLICY),
                 'read_at': datetime.now(timezone.utc).isoformat(),
                 'reports': {n: sha(root / n) for n in names}, 'phase': 'presented'}
        save(root, state)
        print('Record task requirements with acknowledge before editing.')
        return
    state = load_state(root)
    names = validate_read(root, policy, state)
    if a.action == 'acknowledge':
        if state['phase'] != 'presented' or changed(root):
            fail('Acknowledge the displayed incidents before editing.')
        if not a.notes:
            fail('Supply --notes containing per-incident requirement arrays.')
        data = json.loads(a.notes.read_text())
        requirements_valid(data, names)
        state.update(requirements=data, phase='working')
        save(root, state)
        print('PASS: pre-work reading and task requirements recorded')
        return
    requirements_valid(state.get('requirements', {}), names)
    needed = categories_for(changed(root, a.staged))
    if not needed <= set(state['categories']):
        fail('Unread work categories: ' + ', '.join(sorted(needed - set(state['categories']))))
    if a.action == 'read' and a.after:
        names = list(dict.fromkeys(names + reports_for(policy, state['categories'])))
        final_requirements = state['requirements']
        if set(names) != set(final_requirements):
            if not a.notes:
                fail('New incident routes need --notes with requirements for every incident; retain earlier requirements.')
            final_requirements = json.loads(a.notes.read_text())
            requirements_valid(final_requirements, names)
            for name, original in state['requirements'].items():
                if final_requirements[name][:len(original)] != original:
                    fail('Do not discard pre-work requirements: ' + name)
        show_reports(root, names)
        state.update(post_read=fingerprint(root), post_reports={n: sha(root / n) for n in names},
                     post_policy_sha256=sha(root / POLICY), final_requirements=final_requirements, phase='post-read')
        state.pop('review', None)
        save(root, state)
        print('Compare every recorded requirement with the actual changes and evidence using review.')
        return
    names = list(dict.fromkeys(names + reports_for(policy, state['categories'])))
    if state.get('post_policy_sha256') != sha(root / POLICY):
        fail('Incident routing changed after rereading. Run read --after again.')
    if state.get('post_reports') != {n: sha(root / n) for n in names}:
        fail('Incidents changed after rereading. Run read --after again.')
    if state.get('post_read') != fingerprint(root):
        fail('Missing or stale post-work rereading. Run read --after on the final changes.')
    if a.action == 'review':
        if not a.notes:
            fail('Supply --notes with per-incident checks and evidence files.')
        data = json.loads(a.notes.read_text())
        if set(data) != set(names):
            fail('Every read incident needs a final review.')
        evidence = {}
        for name in names:
            checks = data[name]
            if not isinstance(checks, list) or [c.get('requirement') for c in checks] != state['final_requirements'][name]:
                fail('Review every pre-work requirement in order: ' + name)
            for check in checks:
                if check.get('result') not in ('pass', 'not-applicable') or len(check.get('detail', '').strip()) < 8:
                    fail('Unresolved or unexplained review finding: ' + name)
                if not check.get('evidence'):
                    fail('Missing actual evidence: ' + name)
                for entry in check['evidence']:
                    evidence[entry] = evidence_file(root, entry)
        state.update(review=data, evidence=evidence, phase='reviewed')
        save(root, state)
    if state.get('phase') != 'reviewed':
        fail('Post-work comparison is missing. Reading alone does not pass.')
    for name, expected in state['evidence'].items():
        if evidence_file(root, name) != expected:
            fail('Review evidence changed: ' + name)
    print('PASS: incident reading, task requirements, final rereading and evidence review match the current changes')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, OSError, json.JSONDecodeError) as e:
        raise SystemExit('FAIL: ' + str(e))
