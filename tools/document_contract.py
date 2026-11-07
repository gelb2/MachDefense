#!/usr/bin/env python3
"""Check maintained Markdown paths, section links, routes and frozen skill copies."""
import argparse
import hashlib
import html
import json
import re
import subprocess
import unicodedata
from pathlib import Path
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
# Imported sources retain upstream-relative links. Their hashes are checked separately.
PRESERVED = ('docs/reference/', '.agents/skills/', 'Modules/ThirdParty/')


def prose(text):
    return re.sub(r'^\s*(`{3,}|~{3,})[^\n]*\n.*?^\s*\1\s*$', '', text, flags=re.M | re.S)


def anchors(text):
    result = set(re.findall(r'<(?:a|[hH][1-6])\b[^>]*(?:id|name)=[\"\']([^\"\']+)', text))
    used = set()
    for heading in re.findall(r'^ {0,3}#{1,6}\s+(.+?)\s*#*$', prose(text), re.M):
        # The report publisher also accepts explicit heading IDs.
        custom = re.search(r'\s+\{#([^}]+)\}$', heading)
        if custom:
            result.add(custom[1])
        heading = re.sub(r'\[([^]]+)\]\([^)]*\)', r'\1', heading)
        heading = re.sub(r'<[^>]+>', '', html.unescape(heading)).lower()
        slug = ''.join(c for c in heading if c in ' -_' or unicodedata.category(c)[0] in 'LN').replace(' ', '-')
        unique, n = slug, 0
        while unique in used:
            n += 1
            unique = f'{slug}-{n}'
        used.add(unique)
        result.add(unique)
    return result


def check_links(root, paths):
    errors, count, cache = [], 0, {}
    for path in paths:
        for match in re.finditer(r'\]\((<[^>]+>|[^)\n]+)\)', prose(path.read_text())):
            link = match[1]
            dest = link[1:-1] if link.startswith('<') else link.split(' "')[0].split(" '")[0]
            if re.match(r'[a-zA-Z][\w+.-]*:', dest) or dest.startswith('//'):
                continue
            count += 1
            target, _, fragment = dest.partition('#')
            target_path = path.parent / unquote(target) if target else path
            label = f'{path.relative_to(root)}: {link}'
            if not target_path.exists():
                errors.append('Broken link: ' + label)
            elif fragment and target_path.suffix == '.md':
                key = target_path.resolve()
                if key not in cache:
                    cache[key] = anchors(target_path.read_text())
                if unquote(fragment) not in cache[key]:
                    errors.append('Missing section: ' + label)
    return errors, count


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['check', 'plan'])
    parser.add_argument('--task', default='restoration')
    args = parser.parse_args()
    mapping = json.loads((ROOT / 'docs/document-map.json').read_text())
    if args.action == 'plan':
        if args.task not in mapping['domains']:
            raise SystemExit('Tasks: ' + ', '.join(mapping['domains']))
        print('\n'.join(dict.fromkeys(mapping['base_read'] + mapping['domains'][args.task])))
        return
    errors = []
    for name in mapping['base_read'] + [x for values in mapping['domains'].values() for x in values]:
        if not (ROOT / name).is_file():
            errors.append('Missing route: ' + name)
    for item in json.loads((ROOT / 'docs/skill-source-manifest.json').read_text())['files']:
        path = ROOT / item['destination']
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != item['sha256']:
            errors.append('Source copy mismatch: ' + str(path))
    names = subprocess.check_output(['git', 'ls-files', '-z', '--cached', '--others', '--exclude-standard', '--', '*.md'], cwd=ROOT).decode().split('\0')
    paths = [ROOT / n for n in sorted(set(names)) if n and not n.startswith(PRESERVED) and (ROOT / n).is_file()]
    link_errors, count = check_links(ROOT, paths)
    errors.extend(link_errors)
    if errors:
        raise SystemExit('\n'.join(errors))
    print(f'PASS: routes, {len(paths)} Markdown files, {count} local links/sections and source-copy hashes')


if __name__ == '__main__':
    main()
