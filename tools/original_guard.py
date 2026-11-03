#!/usr/bin/env python3
"""Reject original packages/binaries before committing or sending Git objects."""
import argparse, hashlib, io, json, re, subprocess, sys, zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIMIT = 256 * 1024 * 1024


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def hashes():
    manifest = json.loads((ROOT / 'docs/originals-manifest.json').read_text())
    result = set()
    for f in manifest['files']:
        result.add(f['sha256'])
        result.update(a['executable_sha256'] for a in f.get('apps', []) if a.get('executable_sha256'))
    return result


def forbidden_path(name):
    parts = name.replace('\\', '/').lower().split('/')
    return bool(re.search(r'\.ipa(?:$|[./])', '/'.join(parts))) or any(p in ('originals', 'payload') for p in parts)


def inspect(name, data, known, depth=0, budget=None):
    if forbidden_path(name):
        raise ValueError(f'original package path: {name}')
    digest = hashlib.sha256(data).hexdigest()
    if digest in known:
        raise ValueError(f'original package/executable hash: {name}')
    # Only inspectable ZIP containers are accepted at bootstrap.
    if data.startswith((b'\x1f\x8b', b'BZh', b'\xfd7zXZ\x00', b'7z\xbc\xaf\x27\x1c', b'Rar!')) or data[257:262] == b'ustar':
        raise ValueError(f'archive format requires explicit local inspection before tracking: {name}')
    if data.startswith(b'version https://git-lfs.github.com/spec/v1'):
        # No LFS objects are needed at bootstrap. Prevent inspecting only a pointer.
        raise ValueError(f'LFS object requires a materialized asset audit before enabling LFS: {name}')
    if not data.startswith((b'PK\x03\x04', b'PK\x05\x06', b'PK\x07\x08')) and not zipfile.is_zipfile(io.BytesIO(data)):
        return
    if depth >= 4:
        raise ValueError(f'archive nesting limit: {name}')
    budget = budget if budget is not None else [LIMIT]
    try:
        with zipfile.ZipFile(io.BytesIO(data)) as z:
            for member in z.infolist():
                if forbidden_path(member.filename):
                    raise ValueError(f'original package archive member: {name}!{member.filename}')
                if member.is_dir():
                    continue
                budget[0] -= member.file_size
                if budget[0] < 0:
                    raise ValueError(f'archive inspection limit: {name}')
                inspect(name + '!' + member.filename, z.read(member), known, depth+1, budget)
    except (zipfile.BadZipFile, RuntimeError, NotImplementedError) as e:
        raise ValueError(f'unreadable archive: {name}: {type(e).__name__}') from e


def staged():
    for entry in git('ls-files', '--stage', '-z').split(b'\0'):
        if not entry:
            continue
        meta, path = entry.split(b'\t', 1)
        mode, oid, stage = meta.decode().split()
        if stage != '0':
            raise ValueError('unmerged index')
        if mode in ('120000', '160000'):
            raise ValueError('symlink/submodule needs explicit source review: ' + path.decode())
        yield path.decode(), oid


def history(extra=()):
    # Enumerate every historical path as well as each unique blob, including later-deleted files.
    commits = git('rev-list', '--all', *extra).decode().splitlines()
    seen = set()
    for commit in commits:
        for item in git('ls-tree', '-r', '-z', commit).split(b'\0'):
            if not item:
                continue
            meta, path = item.split(b'\t', 1)
            mode, kind, oid = meta.decode().split()
            name = path.decode()
            if forbidden_path(name):
                raise ValueError('original path in history: ' + name)
            if mode in ('120000', '160000'):
                raise ValueError('unreviewed symlink/submodule in history: ' + name)
            if kind == 'blob' and oid not in seen:
                seen.add(oid)
                yield name, oid


def check(entries):
    known = hashes()
    count = 0
    for name, oid in entries:
        size = int(git('cat-file', '-s', oid))
        if size > LIMIT:
            raise ValueError('blob inspection limit: ' + name)
        inspect(name, git('cat-file', 'blob', oid), known)
        count += 1
    return count


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--history', action='store_true')
    p.add_argument('--revision', action='append', default=[])
    args = p.parse_args()
    try:
        count = check(history(args.revision) if args.history else staged())
    except (ValueError, subprocess.CalledProcessError) as e:
        print('BLOCKED:', e, file=sys.stderr)
        return 1
    print(f'PASS: {count} Git blobs; no original package/binary or unaudited LFS pointer')
    return 0


if __name__ == '__main__':
    sys.exit(main())
