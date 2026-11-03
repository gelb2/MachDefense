#!/usr/bin/env python3
"""Create local test saves for verified MachDefense variants; never edit the input.

Evidence: GlobalInfo openDataFile 0x18f6c, slotData: 0x16330,
initSlotWithName:Country: 0x19e20, DlgUpgradeBase setUpgrade 0x39cf4.
Darwin random() is intentional: this runs on the Mac that hosts the port.
WeMade 1.0.3: openDataFile 0x1b404 checks 0xa600 bytes; slotData:
0x1b3e0 uses stride 0x2064; setUpgrade 0x345ac/0x345ea reads/writes
credits at slot+0x3c. These saves must not be exchanged between variants.
"""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import struct
import sys

LAYOUTS = {0xA5EC: ('indie-1.0.5', 0x2060), 0xA600: ('wemade-1.0.3', 0x2064)}
SLOT_START = 0x408
SLOT_COUNT = 5

def layout(data):
    if len(data) not in LAYOUTS: raise ValueError(f'Unexpected save size {len(data)}')
    return LAYOUTS[len(data)]

def checksum(data):
    return sum(x if x < 128 else x - 256 for x in data[4:-4])

def crypt(data):
    if sys.platform != 'darwin':
        raise ValueError('Requires Darwin random(); do not substitute another PRNG')
    result = bytearray(data)
    lib = ctypes.CDLL('/usr/lib/libSystem.B.dylib')
    lib.srandom.argtypes = [ctypes.c_uint]
    lib.random.restype = ctypes.c_long
    lib.srandom(struct.unpack_from('<I', result)[0])
    for i in range(4, len(result) - 4):
        result[i] ^= lib.random() % 256
    return result

def decode(data):
    layout(data)
    result = crypt(data)
    if struct.unpack_from('<i', result, len(result)-4)[0] != checksum(result):
        raise ValueError('Save checksum mismatch')
    return result

def encode(data):
    layout(data)
    result = bytearray(data)
    struct.pack_into('<i', result, len(result)-4, checksum(result))
    return bytes(crypt(result))

def slots(data):
    _, stride = layout(data)
    result = []
    for i in range(SLOT_COUNT):
        at = SLOT_START + i*stride
        result.append(dict(slot=i, active=bool(data[at+8]),
            name=bytes(data[at+0x10:at+0x30]).split(b'\0')[0].decode('utf-8',errors='replace'),
            credits=struct.unpack_from('<i',data,at+0x3c)[0]))
    return result

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('source',type=Path)
    p.add_argument('--slot',type=int,choices=range(5))
    p.add_argument('--credits',type=int)
    p.add_argument('--output',type=Path)
    a = p.parse_args()
    original = a.source.read_bytes(); data = decode(original)
    if a.output is None:
        if a.credits is not None: p.error('--credits requires --output')
        print(json.dumps(slots(data),ensure_ascii=False,indent=2)); return
    if a.slot is None or a.credits is None: p.error('--output requires --slot and --credits')
    if not 0 <= a.credits <= 10_000_000: p.error('credits outside test range 0..10000000')
    root=Path(__file__).resolve().parents[2]
    dest=a.output.resolve()
    if root/'.local' not in dest.parents: p.error('Output must be under repository .local; installation is separate')
    if dest == a.source.resolve() or dest.exists(): p.error('Refusing to overwrite an existing save')
    if not slots(data)[a.slot]['active']: p.error('Create the test profile in the original app first')
    before=slots(data)
    variant,stride=layout(data)
    offset=SLOT_START+a.slot*stride+0x3c
    struct.pack_into('<i',data,offset,a.credits)
    output=encode(data)
    assert decode(output)[offset:offset+4] == struct.pack('<i',a.credits)
    dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_bytes(output)
    manifest=dict(variant=variant, source_sha256=hashlib.sha256(original).hexdigest(),
        output_sha256=hashlib.sha256(output).hexdigest(),slot=a.slot,
        change={'field':'credits','offset':hex(offset),'before':before[a.slot]['credits'],'after':a.credits},
        changed_plaintext_offsets=[hex(i) for i,(x,y) in enumerate(zip(decode(original),decode(output))) if x!=y],
        slots=slots(decode(output)))
    dest.with_suffix('.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps(manifest,ensure_ascii=False,indent=2))
if __name__=='__main__': main()
