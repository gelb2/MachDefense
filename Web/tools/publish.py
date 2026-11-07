#!/usr/bin/env python3
"""Publish verified product files to gh-pages, following HeavyMach's packaging flow."""
import hashlib
import json
import pathlib
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[2]
DIST = ROOT / 'build/web-normal/dist'
STAGE = ROOT / 'build/pages-publish'
REMOTE = 'git@github.com:gelb2/MachDefense.git'


def run(*args, cwd=ROOT):
    return subprocess.run(args, cwd=cwd, check=True, text=True, capture_output=True).stdout.strip()


def main():
    build = json.loads((DIST / 'build.json').read_text())
    if build['cheat']:
        raise SystemExit('Refusing to publish the cheat application')
    manifest = json.loads((DIST / 'asset-manifest.json').read_text())
    allowed = {'index.html', 'style.css', 'app.js', 'storage.js', 'audio.js',
               'manifest.webmanifest', 'icon.png', 'title.jpg', 'social-preview.jpg', 'game.js', 'game.wasm', 'game.data'}
    if {f['url'] for f in manifest['files']} != allowed:
        raise SystemExit('Unexpected or missing product files')
    for f in manifest['files']:
        path = DIST / f['url']
        if path.is_symlink() or hashlib.sha256(path.read_bytes()).hexdigest() != f['sha256']:
            raise SystemExit('Product checksum mismatch: ' + f['url'])
    run('python3', 'tools/original_guard.py', '--history')
    if not (STAGE / '.git').exists():
        STAGE.mkdir(parents=True, exist_ok=True)
        run('git', 'init', '--initial-branch=gh-pages', str(STAGE))
        run('git', 'remote', 'add', 'origin', REMOTE, cwd=STAGE)
    if run('git', 'remote', 'get-url', 'origin', cwd=STAGE) != REMOTE:
        raise SystemExit('Unexpected deployment remote')
    if run('git', 'ls-remote', '--heads', 'origin', 'gh-pages', cwd=STAGE):
        run('git', 'fetch', 'origin', 'gh-pages', cwd=STAGE)
        run('git', 'checkout', '-B', 'gh-pages', 'FETCH_HEAD', cwd=STAGE)
    for path in STAGE.iterdir():
        if path.name == '.git':
            continue
        shutil.rmtree(path) if path.is_dir() else path.unlink()
    chunks={part['url']:part for file in manifest['files'] for part in file.get('parts',[])}
    for name,part in chunks.items():
        if name != 'chunks/'+part['sha256']+'.bin':
            raise SystemExit('Invalid chunk path')
        source=DIST/name
        if source.is_symlink() or len(source.read_bytes())!=part['size'] or hashlib.sha256(source.read_bytes()).hexdigest()!=part['sha256']:
            raise SystemExit('Chunk checksum mismatch: '+name)
        (STAGE/name).parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(source,STAGE/name)
    for name in allowed | {'sw.js', 'asset-manifest.json', 'build.json'}:
        shutil.copy2(DIST / name, STAGE / name)
    (STAGE / '.nojekyll').touch()
    run('git', 'add', '--all', cwd=STAGE)
    if subprocess.run(['git', 'diff', '--cached', '--quiet'], cwd=STAGE).returncode:
        run('gitleaks', 'dir', '--no-banner', '--redact', str(STAGE))
        run('git', 'commit', '-m', 'Deploy MachDefense web ' + build['version'], cwd=STAGE)
    print(run('git', 'push', 'origin', 'HEAD:gh-pages', cwd=STAGE))
    print('Published product version ' + build['version'])


if __name__ == '__main__':
    main()
