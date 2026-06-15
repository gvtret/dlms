#!/usr/bin/env python3
"""Split a unified diff (from `git diff`) into up to N patches by hunk
start line in the +new file.

Usage:
  split-diff3.py SPEC OUT_A OUT_B [OUT_C ...]

SPEC is a comma-separated list of thresholds describing bucket boundaries.
  E.g. "1000,2000" means three buckets:  [<1000], [1000..2000), [>=2000]
  E.g. "500"      means two   buckets:  [<500], [>=500]

Per-file headers are emitted into every bucket that received at least one
hunk for that file.
"""
import re
import sys

if len(sys.argv) < 4:
    print(__doc__, file=sys.stderr)
    sys.exit(2)

thresholds = [int(x) for x in sys.argv[1].split(',') if x.strip()]
out_paths = sys.argv[2:]
assert len(out_paths) == len(thresholds) + 1, (
    f"need {len(thresholds)+1} output paths for {len(thresholds)} thresholds, "
    f"got {len(out_paths)}"
)


def bucket_of(start: int) -> int:
    for i, t in enumerate(thresholds):
        if start < t:
            return i
    return len(thresholds)


text = sys.stdin.read()
file_blocks = re.split(r"(?m)^(?=diff --git )", text)
file_blocks = [b for b in file_blocks if b.strip()]

hunk_re = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@", re.MULTILINE)

buckets = [[] for _ in out_paths]

for block in file_blocks:
    m = hunk_re.search(block)
    if not m:
        buckets[0].append(block)
        continue
    header = block[:m.start()]
    rest = block[m.start():]
    positions = [mm.start() for mm in hunk_re.finditer(rest)] + [len(rest)]
    file_buckets = [[] for _ in out_paths]
    for i in range(len(positions) - 1):
        chunk = rest[positions[i]:positions[i+1]]
        hm = hunk_re.match(chunk)
        new_start = int(hm.group(1))
        b = bucket_of(new_start)
        file_buckets[b].append(chunk)
    for i, hunks in enumerate(file_buckets):
        if hunks:
            buckets[i].append(header + "".join(hunks))

for path, blocks in zip(out_paths, buckets):
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(blocks))
    print(f"wrote {path}: {len(blocks)} file blocks", file=sys.stderr)
