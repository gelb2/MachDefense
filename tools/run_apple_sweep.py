#!/usr/bin/env python3
"""Run the accelerated production-game XCTest sweeps on a named simulator."""
import argparse
import datetime
import json
from pathlib import Path
import re
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
TESTS = [
    'testPortObservedPurchasesThroughGameButtonsAndDiskSave',
    'testEightyMissionFailureVictoryNextAndDiskReloadSweep',
    'testAllProductionAndSpecialLevelsWithGraphicsAndAudio',
]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--simulator', required=True, help='Simulator UDID; never selects a physical device')
    a = p.parse_args()
    devices = json.loads(subprocess.check_output(['xcrun', 'simctl', 'list', 'devices', 'available', '-j']))
    if not any(d['udid'] == a.simulator for group in devices['devices'].values() for d in group):
        p.error('Simulator UDID is not available')
    stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f')
    output = ROOT / '.local' / 'apple-sweep' / stamp
    output.mkdir(parents=True)
    command = ['xcodebuild', '-workspace', 'Apple/MachDefense.xcworkspace',
               '-scheme', 'MachDefenseRendererTests', '-configuration', 'Debug',
               '-destination', f'platform=iOS Simulator,id={a.simulator}',
               '-derivedDataPath', str(ROOT / '.local' / 'parity-derived'),
               '-resultBundlePath', str(output / 'result.xcresult'),
               'CODE_SIGNING_ALLOWED=NO', 'test']
    command += [f'-only-testing:MachDefenseRendererTests/MenuRenderTests/{name}' for name in TESTS]
    start = time.monotonic()
    print(f'Running {len(TESTS)} sweeps; evidence: {output}', flush=True)
    with (output / 'xcodebuild.log').open('w') as log:
        result = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    text = (output / 'xcodebuild.log').read_text(errors='replace')
    passed = [name for name in TESTS if re.search(r"Test Case '-\[MenuRenderTests " + name + r"\]' passed", text)]
    report = dict(simulator=a.simulator, exit_code=result.returncode,
                  build_and_test_seconds=round(time.monotonic()-start, 3),
                  expected=TESTS, passed=passed,
                  counters=[line for line in text.splitlines() if re.search(r'MD_(PORT_PURCHASE|CAMPAIGN|CONTENT)_SWEEP ', line)])
    (output / 'summary.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))
    return 0 if result.returncode == 0 and len(passed) == len(TESTS) else 1

if __name__ == '__main__':
    sys.exit(main())
