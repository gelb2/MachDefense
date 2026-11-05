#!/usr/bin/env python3
"""Compare the runtime's decimal formatter with Darwin snprintf."""
import ctypes
import itertools
import json
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
config = json.loads((root / '.local/compatibility.json').read_text())
source = (Path(config['runtime_source']) / 'src/libc/stdio/printf.rs').read_text()
start = source.index('fn format_decimal(')
end = source.index('/// String formatting implementation', start)
cases = list(itertools.product(
    [-2147483648, -17, 0, 23, 2147483647], [False, True],
    [None, 0, 3, 12], [0, 2, 8, 15], [False, True],
))
calls = []
for value, plus, precision, width, zero in cases:
    p = 'None' if precision is None else f'Some({precision})'
    calls.append(f'println!("{{}}", format_decimal({value}, true, {str(plus).lower()}, {p}, {width}, {str(zero).lower()}));')
code = source[start:end] + '\nfn main() {\n' + '\n'.join(calls) + '\n}\n'
rustc = Path(config['rustup_home']) / 'toolchains/1.99.1-aarch64-apple-darwin/bin/rustc'
with tempfile.TemporaryDirectory(prefix='machdefense-decimal-') as td:
    td = Path(td)
    (td / 'check.rs').write_text(code)
    subprocess.run([str(rustc), str(td / 'check.rs'), '-o', str(td / 'check')], check=True)
    actual = subprocess.check_output([str(td / 'check')], text=True).splitlines()
lib = ctypes.CDLL('/usr/lib/libSystem.B.dylib')
lib.snprintf.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_char_p]
lib.snprintf.restype = ctypes.c_int
assert len(actual) == len(cases)
for case, got in zip(cases, actual):
    value, plus, precision, width, zero = case
    fmt = '%' + ('+' if plus else '') + ('0' if zero else '')
    fmt += str(width) if width else ''
    fmt += '' if precision is None else f'.{precision}'
    fmt += 'd'
    buffer = ctypes.create_string_buffer(100)
    lib.snprintf(buffer, len(buffer), fmt.encode(), ctypes.c_int(value))
    expected = buffer.value.decode()
    assert got == expected, (fmt, value, got, expected)
print(f'PASS: {len(cases)} signed decimal cases match Darwin snprintf, including %+d.')
