#!/usr/bin/env python3
import argparse, hashlib, json, re, unicodedata
from urllib.parse import unquote, urlsplit
from pathlib import Path

def markdown_body(text):
    return re.sub(r'^\s*(`{3,}|~{3,})[^\n]*\n.*?^\s*\1\s*$', '', text, flags=re.M|re.S)

def anchors(text):
    body=markdown_body(text)
    result=set(re.findall(r'(?:id|name)=["\']([^"\']+)["\']',body))
    counts={}
    for title in re.findall(r'^ {0,3}#{1,6}\s+(.+?)\s*#*$',body,re.M):
        explicit=re.search(r'\s*\{#([^}]+)\}\s*$',title)
        if explicit:
            result.add(explicit[1]);title=title[:explicit.start()]
        title=re.sub(r'!?\[([^]]+)\]\([^)]+\)',r'\1',title)
        title=re.sub(r'<[^>]+>','',title).lower().replace('`','')
        title=''.join(c for c in title if c in '-_ ' or unicodedata.category(c)[0] in 'LNM')
        slug=title.replace(' ','-');n=counts.get(slug,0);counts[slug]=n+1
        result.add(slug+(f'-{n}' if n else ''))
    return result

def maintained_documents(root):
    files=[root/name for name in ['README.md','AGENTS.md','HANDOVER-CURRENT.md']]
    for directory in ['docs','Apple','Modules','Sources','Resources','Compatibility','Android','Web','Desktop','Distribution','Tests']:
        files.extend(p for p in (root/directory).rglob('*.md')
                     if not any(part in ('reference','ThirdParty','.local') for part in p.relative_to(root).parts))
    return sorted(set(p for p in files if p.is_file()))

def check_links(path):
    errors=[]
    for link in re.findall(r'\]\(([^)]+)\)',markdown_body(path.read_text())):
        link=link.strip().strip('<>')
        parsed=urlsplit(link)
        if parsed.scheme or parsed.netloc:continue
        target=path.parent/unquote(parsed.path) if parsed.path else path
        if not target.is_file():
            if not target.exists():errors.append('Broken link: '+str(path)+': '+link)
        elif parsed.fragment and target.suffix=='.md' and unquote(parsed.fragment) not in anchors(target.read_text()):
            errors.append('Broken anchor: '+str(path)+': '+link)
    return errors

def main():
    cli(Path(__file__).resolve().parents[1])

def cli(root):
    p=argparse.ArgumentParser()
    p.add_argument('action',choices=['check','plan'])
    p.add_argument('--task',default='restoration')
    a=p.parse_args()
    m=json.loads((root/'docs/document-map.json').read_text())
    if a.action=='plan':
        if a.task not in m['domains']:raise SystemExit('Tasks: '+', '.join(m['domains']))
        print('\n'.join(dict.fromkeys(m['base_read']+m['domains'][a.task])))
    else:
        errors=[]
        for name in m['base_read']+[x for values in m['domains'].values() for x in values]:
            if not (root/name).is_file():errors.append('Missing route: '+name)
        for item in json.loads((root/'docs/skill-source-manifest.json').read_text())['files']:
            path=root/item['destination']
            if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=item['sha256']:errors.append('Source copy mismatch: '+str(path))
        documents=maintained_documents(root)
        for path in documents:errors.extend(check_links(path))
        if errors:raise SystemExit('\n'.join(errors))
        print(f'PASS: {len(documents)} maintained documents, local links/anchors, routes and source-copy hashes')

if __name__=='__main__':main()
