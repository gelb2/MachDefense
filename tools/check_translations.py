#!/usr/bin/env python3
"""Check reviewed visitor-document pairs; internal development docs are excluded."""
import hashlib
import json
import os
import re
from pathlib import Path
from document_contract import check_links, prose

ROOT = Path(__file__).resolve().parents[1]
NAV = re.compile(r'<!-- language-nav:start -->\n(.*?)\n<!-- language-nav:end -->', re.S)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def commands(text):
    blocks = re.findall(r'^```[^\n]*\n(.*?)^```', text, re.M | re.S)
    # Shell comments and placeholder labels may be translated; executable lines may not.
    return ['\n'.join(re.sub(r'<[^<>\n]+>', '<ARG>', line) for line in block.splitlines()
                      if not line.lstrip().startswith('#')) for block in blocks]


def check(root):
    scope = json.loads((root / 'docs/translations/scope.json').read_text())
    manifest = json.loads((root / 'docs/translations/manifest.json').read_text())
    records = {p['ko']: p for p in manifest['pairs']}
    mapping = {p['ko']: p['en'] for p in scope['pairs']}
    errors, paths = [], []
    if len(records) != len(manifest['pairs']) or len(mapping) != len(scope['pairs']):
        errors.append('Duplicate document pair')
    if set(records) != set(mapping):
        errors.append('Scope/manifest pairs differ')
    for ko, en in mapping.items():
        record = records.get(ko, {})
        if record.get('en') != en:
            errors.append(f'Pair mismatch: {ko}')
        texts = {}
        for lang, name in [('ko', ko), ('en', en)]:
            path = root / name
            if not path.is_file():
                errors.append(f'Missing translation file: {name}')
                continue
            paths.append(path)
            text = path.read_text()
            texts[lang] = text
            if digest(path) != record.get(lang + '_sha256'):
                errors.append(f'Review required after edit: {name}')
            expected = f'[한국어]({os.path.relpath(root / ko, path.parent)}) | [English]({os.path.relpath(root / en, path.parent)})'
            nav = NAV.findall(text)
            if nav != [expected] or '<!-- language-nav:start -->' not in '\n'.join(text.splitlines()[:6]):
                errors.append(f'Incorrect language links: {name}')
            if lang == 'en':
                body = NAV.sub('', prose(text))
                for dest in re.findall(r'\]\(([^)\n]+)\)', body):
                    if re.match(r'[a-zA-Z][\w+.-]*:', dest):
                        continue
                    target = dest.split('#')[0]
                    key = os.path.relpath(path.parent / target, root) if target else name
                    if key in mapping:
                        errors.append(f'English link points to Korean visitor page: {name}: {dest}')
        if len(texts) == 2 and commands(texts['ko']) != commands(texts['en']):
            errors.append(f'Code/command blocks differ: {ko}')
    link_errors, count = check_links(root, paths)
    errors.extend(link_errors)
    return errors, len(mapping), count


def main():
    errors, pairs, links = check(ROOT)
    if errors:
        raise SystemExit('\n'.join(errors))
    print(f'PASS: {pairs} reviewed Korean/English pairs, hashes, language navigation, commands, {links} local links/sections')


if __name__ == '__main__':
    main()
