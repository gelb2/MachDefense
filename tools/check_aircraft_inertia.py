#!/usr/bin/env python3
"""Check the actual WebGL validation scene's per-tick velocity recording."""
import json
import math
import sys
from pathlib import Path

trace = json.loads(Path(sys.argv[1]).read_text())
result = []
for kind, name in enumerate(('AircraftLight', 'AircraftHeavy', 'AircraftDuct', 'AircraftTransport')):
    samples = [next(a for a in s['aircraft'] if a['name'] == name) for s in trace]
    last, stopped, reversals = 0, False, 0
    for sample in samples:
        sign = 0 if sample['vx'] == 0 else 1 if sample['vx'] > 0 else -1
        if sign == 0 and sample['speed'] == 0:
            stopped = True
        if sign:
            if last and sign != last:
                assert stopped, (name, 'reversed without stopping', sample)
                reversals += 1
            stopped, last = False, sign
    change = max(math.hypot(b['vx']-a['vx'], b['vy']-a['vy']) for a, b in zip(samples, samples[1:]))
    assert reversals > 0
    assert change < (2, 1.8, 1.6, 1.4)[kind] / 24 + .0001, (name, change)
    result.append(dict(name=name, reversals=reversals, stoppedFrames=sum(a['speed'] == 0 for a in samples), maximumVelocityChangePerTick=change, maximumSpeed=max(a['speed'] for a in samples)))
output = dict(frames=len(trace), ok=True, aircraft=result)
Path(sys.argv[2]).write_text(json.dumps(output, indent=2) + '\n')
print(json.dumps(output))
