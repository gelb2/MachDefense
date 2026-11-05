#!/usr/bin/env python3
"""Compare the compiled compatibility RNG with Darwin using 5,000 outputs."""
import ctypes, json, subprocess, tempfile
from pathlib import Path
root = Path(__file__).resolve().parents[1]
config = json.loads((root / '.local/compatibility.json').read_text())
source = (Path(config['runtime_source']) / 'src/libc/stdlib.rs').read_text()
start = source.index('#[derive(Debug)]\nstruct DarwinRandom')
end = source.index('fn srandom(env:', start)
code = source[start:end] + '''
fn main() {
    for seed in [0u32, 1, 0xb27f7, 2147483648, 4294967295] {
        let mut rng = DarwinRandom::seeded(seed);
        for _ in 0..1000 { println!("{} {}", seed, rng.next()); }
    }
}
'''
rustc = Path(config['rustup_home']) / 'toolchains/1.99.1-aarch64-apple-darwin/bin/rustc'
with tempfile.TemporaryDirectory(prefix='machdefense-rng-') as td:
    td = Path(td)
    (td / 'check.rs').write_text(code)
    subprocess.run([str(rustc), str(td / 'check.rs'), '-o', str(td / 'check')], check=True)
    lines = subprocess.check_output([str(td / 'check')], text=True).splitlines()
lib = ctypes.CDLL('/usr/lib/libSystem.B.dylib')
lib.srandom.argtypes = [ctypes.c_uint]
lib.random.restype = ctypes.c_long
last = None
for line in lines:
    seed, actual = map(int, line.split())
    if seed != last:
        lib.srandom(seed)
        last = seed
    expected = lib.random()
    assert actual == expected, (seed, actual, expected)
print(f'PASS: {len(lines)} random() results match Darwin, including the original table seed.')
