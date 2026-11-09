#!/usr/bin/env python3
"""Read-only audit of helicopter definition, assembly and image consumption.

A resource's existence is not evidence that a machine consumes it. This audit
reports authored and prepared definitions separately, including invisible mounts.
It does not claim visual quality, bank continuity or original-process parity.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PREFIXES = ('ALight', 'AHeavy', 'ADuct', 'ATransport')
MODELS = {'AircraftLight': 'Scout', 'AircraftDuct': 'Cheyenne',
          'AircraftTransport': 'Beluga', 'AircraftHeavy': 'AH-10'}

def sha(data):
    return hashlib.sha256(data).hexdigest()

def chunk(blob, pos=0):
    start = pos
    size, kind, n = struct.unpack_from('<III', blob, pos)
    pos += 12
    data = blob[pos:pos+n]
    pos += n
    count, = struct.unpack_from('<I', blob, pos)
    pos += 4
    children = []
    for _ in range(count):
        c, pos = chunk(blob, pos)
        children.append(c)
    # Match Chunk::parse: original declared sizes can differ from consumed bytes.
    assert pos <= len(blob)
    return {'kind': kind, 'data': data, 'children': children}, pos

def name(node):
    return node['data'][:24].split(b'\0')[0].decode('ascii')

def nodes(tree, kind):
    result = {}
    def walk(n):
        if n['kind'] == kind and n['data']:
            result[name(n)] = n
        for c in n['children']:
            walk(c)
    walk(tree)
    return result

def table(blob):
    lines = blob.decode().splitlines()
    keys = [x.strip() for x in lines[0].split('\t')]
    return {cells[0]: dict(zip(keys, cells)) for line in lines[1:]
            if line.strip() and not line.startswith('//')
            and (cells := [x.strip() for x in line.split('\t')])}

def png(blob):
    pos, result = 8, {'cgbi': False}
    assert blob[:8] == b'\x89PNG\r\n\x1a\n'
    while pos + 12 <= len(blob):
        length, = struct.unpack_from('>I', blob, pos)
        kind = blob[pos+4:pos+8]
        data = blob[pos+8:pos+8+length]
        if kind == b'CgBI': result['cgbi'] = True
        if kind == b'IHDR': result['size'] = list(struct.unpack_from('>II', data))
        pos += 12 + length
    return result

def classify_authored_usage(authored_names, base_rows, level_rows):
    base_used = {r['part'] for rows in base_rows for r in rows}
    level_used = {r['part'] for rows in level_rows for r in rows}
    authored = set(authored_names)
    return sorted(authored - base_used - level_used), sorted((authored & level_used) - base_used)

def audit():
    inputs = {}
    def read(path):
        b = (ROOT/path).read_bytes()
        inputs[path] = sha(b)
        return b
    spec = json.loads(read('Art/Aircraft/runtime/assembly.json'))
    trees = {}
    for f in ('PartsInfo', 'MachInfo', 'AircraftParts', 'AircraftMachines',
              'Level10Parts', 'Level10Machines'):
        b = read(f'Resources/RawData/{f}.mcb')
        trees[f], end = chunk(b)
        assert end == len(b)
    parts = nodes(trees['PartsInfo'], 1) | nodes(trees['Level10Parts'], 1)
    machines = nodes(trees['MachInfo'], 3) | nodes(trees['Level10Machines'], 3)
    aliases = table(read('Resources/Tables/PartsList.tsv'))
    tile_grids = table(read('Resources/Tables/TileList.tsv'))
    loader = read('Sources/Core/Simulation/MachineModels.cpp').decode()
    begin = loader.index('BattleModels BattleModels::load(')
    end = loader.index('\nvoid BattleModels::appendPartAlias', begin)
    runtime_variants = 'appendMachineVariant(' in loader[begin:end]
    read('Sources/Core/Simulation/AircraftVisual.hpp')
    textures = {}
    def describe(part_name):
        p = parts.get(part_name)
        if p is None and runtime_variants:
            p = parts.get(part_name.rsplit('_', 1)[0])
        assert p is not None, part_name
        typ, tiles, sockets = p['data'][24:27]
        image = name(p)+'.png' if tiles else 'PartsDummy.png'
        grid = [max(tiles, 1), 1]
        alias = aliases.get(part_name)
        if alias and alias['TileName'] not in ('', '-'):
            image = alias['TileName']+'.png'
            grid = [int(tile_grids[image]['X']), int(tile_grids[image]['Y'])]
        path = 'Resources/Images/'+image
        exists = (ROOT/path).is_file()
        if exists and image not in textures:
            b = read(path)
            textures[image] = {'path': path, 'sha256': sha(b), **png(b)}
        return {'part': part_name, 'type': typ, 'tileCount': tiles,
                'sockets': sockets, 'image': image, 'grid': grid,
                'firstTile': int(alias['No']) if alias else 0,
                'imageExists': exists, 'visible': exists}
    def describe_machine(machine_name):
        tree = machines[machine_name]
        rows = []
        def walk(n, parent=None):
            if n['kind'] == 4:
                row = describe(name(n))
                row.update(parent=parent, parentSocket=n['data'][24], zOrder=n['data'][25])
                rows.append(row)
                parent = name(n)
            for c in n['children']: walk(c, parent)
        walk(tree)
        return rows
    summaries = []
    base_rows, level_rows = [], []
    for model, label in MODELS.items():
        base = describe_machine(model)
        base_rows.append(base)
        levels = []
        for level in range(1, 11):
            model_name = f'{model}_{level:02d}'
            persisted = model_name in machines
            rows = describe_machine(model_name) if persisted else base
            level_rows.append(rows)
            levels.append({'level': level, 'machine': model_name,
                           'nodeCount': len(rows), 'visibleNodeCount': sum(r['visible'] for r in rows),
                           'nodes': rows,
                           'persistedDefinition': persisted,
                           'renderedImages': [r['image'] for r in rows if r['visible']]})
        signatures = {tuple(l['renderedImages']) for l in levels}
        summaries.append({'unit': label, 'model': model,
                          'nodeCount': len(base), 'visibleNodeCount': sum(r['visible'] for r in base),
                          'nodes': base, 'levels': levels,
                          'distinctLevelImageSequences': len(signatures),
                          'levelAppearanceChangesPresent': len(signatures) > 1})
    unused, level_only = classify_authored_usage([p['name'] for p in spec['parts']], base_rows, level_rows)
    for p in unused: describe(p)
    return {'scope': 'Static consumption audit; no game assets or runtime code changed by this audit',
            'runtimeMachineVariantCallInLoader': runtime_variants,
            'persistedAircraftMachineCount': len(nodes(trees['AircraftMachines'], 3)),
            'originalPartDefinitions': len(nodes(trees['PartsInfo'], 1)),
            'originalNamedMachines': len(nodes(trees['MachInfo'], 3)),
            'unusedAuthoredParts': unused, 'levelOnlyAuthoredParts': level_only, 'aircraft': summaries,
            'textures': textures, 'inputSHA256': inputs,
            'limitations': ['No visual-quality acceptance', 'No web or device rendering performed by this audit',
                           'No performance equivalence measurement',
                           'Same file format or parser does not establish full original production-process parity']}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, default=ROOT/'docs/evidence/aircraft-assembly-followup-20261109/art-audit.json')
    args = parser.parse_args()
    result = audit()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2)+'\n')
    print(json.dumps({k: result[k] for k in ('runtimeMachineVariantCallInLoader', 'persistedAircraftMachineCount', 'unusedAuthoredParts')}))
    for a in result['aircraft']:
        print(a['unit'], a['nodeCount'], a['visibleNodeCount'], 'level image sequences', a['distinctLevelImageSequences'])
if __name__ == '__main__': main()
