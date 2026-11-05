#!/usr/bin/env python3
"""Export local REA Evidence for one immutable MachDefense binary, outside Git."""
import argparse, hashlib, json, os, queue, subprocess, threading, time, tomllib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

def write(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(data, indent=2, ensure_ascii=False) + '\n')
    temporary.replace(path)

def unwrap(raw):
    if isinstance(raw, dict):
        if 'evidence_id' in raw: return raw
        for key in ['structuredContent', 'data']:
            if isinstance(raw.get(key), dict):
                found = unwrap(raw[key])
                if found: return found
        for content in raw.get('content', []):
            if content.get('type') == 'text':
                try:
                    found = unwrap(json.loads(content['text']))
                    if found: return found
                except ValueError: pass
    return None

class Client:
    def __init__(self, out):
        cfg = tomllib.loads((Path.home()/'.codex/config.toml').read_text())['mcp_servers']['rea']
        env = os.environ.copy(); env.update(cfg.get('env', {}))
        self.sequence = 0; self.messages = queue.Queue()
        self.stderr = (out/'rea-server.stderr').open('a')
        self.process = subprocess.Popen([cfg['command'], *cfg.get('args', [])], stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=self.stderr, text=True, bufsize=1, env=env)
        def reader():
            for line in self.process.stdout:
                try: self.messages.put(json.loads(line))
                except ValueError: continue
            self.messages.put(None)
        threading.Thread(target=reader, daemon=True).start()
        self.request('initialize', {'protocolVersion':'2024-11-05','capabilities':{},
            'clientInfo':{'name':'machdefense-reconstruction-export','version':'1'}})
        self.send({'jsonrpc':'2.0','method':'notifications/initialized'})
    def send(self, message):
        self.process.stdin.write(json.dumps(message)+'\n'); self.process.stdin.flush()
    def request(self, method, params, timeout=900):
        self.sequence += 1; seq = self.sequence
        self.send({'jsonrpc':'2.0','id':seq,'method':method,'params':params})
        deadline = time.monotonic()+timeout
        while True:
            response = self.messages.get(timeout=max(.01, deadline-time.monotonic()))
            if response is None: raise RuntimeError('REA child closed')
            if response.get('id') == seq:
                if 'error' in response: raise RuntimeError(response['error'])
                return response['result']
    def call(self, name, arguments):
        return self.request('tools/call', {'name':name,'arguments':arguments})
    def close(self):
        self.process.stdin.close()
        try: self.process.wait(timeout=15)
        except subprocess.TimeoutExpired:
            self.process.terminate(); self.process.wait(timeout=10)
        self.stderr.close()

PRIORITY = [
    ('GlobalInfo','initSlotWithName:Country:'), ('GlobalInfo','selectSlot:'),
    ('GlobalInfo','updateBaseUpgrade'), ('GlobalInfo','init'), ('GlobalInfo','saveDataFile'),
    ('GlobalInfo','openDataFile'), ('GameMain','start'), ('GameMain','tick'),
    ('GuiTitle','init'), ('GuiTitle','tick'), ('GuiTitle','onTap:State:ID:'),
    ('GuiSelectSlot','init'), ('GuiSelectSlot','tick'), ('GuiSelectSlot','onTap:State:ID:'),
    ('GuiSelectStage','init'), ('GuiSelectStage','onTap:State:ID:'),
    ('GobWorld','setStage:'), ('GobWorld','processGameRule'), ('GobWorld','tick'),
    ('GobWorld','processEnemyGen'), ('GobWorld','onClearStage'), ('GobWorld','setGameOver'),
    ('GobHvMach','tick'), ('GobHvMach','processMove'), ('GobHvMach','onDamage:From:'),
    ('GobHvM_Enemy','processAI_Attack'), ('GobHvM_Enemy','processAI_Vehicle'),
    ('GobBullet','tick'), ('WndBuildSlot','tick'), ('GuiGame','handleButton:'),
    ('DlgUpgradeBase','onTap:State:ID:'), ('MachBuildSet','setLevel:'),
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--metadata', type=Path, required=True)
    parser.add_argument('--limit', type=int)
    parser.add_argument('--classes', nargs='*')
    args = parser.parse_args()
    binary, out = args.binary.resolve(), args.output.resolve()
    if binary.is_relative_to(ROOT) or out.is_relative_to(ROOT):
        raise SystemExit('Binary and analysis output must stay outside repository')
    out.mkdir(parents=True, exist_ok=True)
    sha = hashlib.sha256(binary.read_bytes()).hexdigest()
    metadata = json.loads(args.metadata.read_text())
    if metadata['sha256'] != sha: raise SystemExit('Metadata source digest mismatch')
    methods = [dict(m, owner=c['name'], class_method=class_method)
        for c in metadata['classes'] for key,class_method in [('methods',False),('class_methods',True)] for m in c[key]]
    priority = {key:i for i,key in enumerate(PRIORITY)}
    methods.sort(key=lambda m:(priority.get((m['owner'],m['selector']),len(PRIORITY)),int(m['address'],16)))
    if args.classes: methods = [m for m in methods if m['owner'] in args.classes]
    if args.limit: methods = methods[:args.limit]
    client=Client(out); records=[]
    try:
        write(out/'catalog.json',client.request('tools/list',{}))
        opened=client.call('open_binary',{'path':str(binary),'provider_id':'ghidra'})
        write(out/'open.json',opened)
        if opened.get('isError'): raise RuntimeError('open_binary failed')
        for method in methods:
            record=dict(method);record['exports']={}
            for operation, kind, extension in [('procedure_pseudo_code','pseudo','pseudo.c'),('procedure_assembly','assembly','asm')]:
                address=method['address']; ep=out/'evidence'/f'{address}.{kind}.json'
                try:
                    raw=json.loads(ep.read_text()) if ep.exists() else client.call(operation,{'procedure':address})
                    if not ep.exists(): write(ep,raw)
                    evidence=unwrap(raw)
                    if not evidence: raise ValueError('No Evidence')
                    if evidence.get('subject',{}).get('digest',{}).get('sha256') != sha: raise ValueError('Source digest mismatch')
                    value=evidence.get('normalized_result')
                    if not isinstance(value,str) or not value.strip(): raise ValueError('No code text')
                    (out/'functions').mkdir(exist_ok=True)
                    (out/'functions'/f'{address}.{extension}').write_text(value)
                    record['exports'][kind]={'status':'exported','evidence_id':evidence['evidence_id'],'limitations':evidence.get('limitations',[])}
                except Exception as exc:
                    record['exports'][kind]={'status':'failed','error':str(exc)}
            records.append(record)
            write(out/'export-manifest.json',{'source_sha256':sha,'selected_methods':len(methods),'completed_methods':len(records),'methods':records})
            print(f"{len(records)}/{len(methods)} {method['owner']} {method['selector']} {record['exports']['pseudo']['status']}",flush=True)
        inventory=client.call('list_procedures',{})
        write(out/'procedure-inventory.json',inventory)
    finally:
        try: write(out/'close.json',client.call('close_binary',{}))
        finally: client.close()

if __name__=='__main__': main()
