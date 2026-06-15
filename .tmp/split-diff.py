#!/usr/bin/env python3
"""Split a unified diff (from `git diff`) into two patches by hunk start line.

Reads diff from stdin, writes hunks whose +new-start < THRESHOLD to LOW path,
the rest to HIGH path. Per-file headers are emitted into whichever side
actually receives at least one hunk for that file.
"""
import sys
import re

if len(sys.argv) != 4:
    print("usage: split-diff.py THRESHOLD LOW HIGH", file=sys.stderr)
    sys.exit(2)

threshold = int(sys.argv[1])
low_path = sys.argv[2]
high_path = sys.argv[3]

text = sys.stdin.read()

# Tokenize by file: a "file block" starts at 'diff --git'.
file_blocks = re.split(r"(?m)^(?=diff --git )", text)
file_blocks = [b for b in file_blocks if b.strip()]

hunk_start_re = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@", re.MULTILINE)

low_out = []
high_out = []

for block in file_blocks:
    # Split file block into header + hunks.
    m = hunk_start_re.search(block)
    if not m:
        # No hunks (binary or pure header); keep on low side.
        low_out.append(block)
        continue
    header = block[:m.start()]
    # Split remainder into hunks at each @@.
    rest = block[m.start():]
    hunks = []
    positions = [mm.start() for mm in hunk_start_re.finditer(rest)] + [len(rest)]
    for i in range(len(positions) - 1):
        chunk = rest[positions[i]:positions[i+1]]
        hm = hunk_start_re.match(chunk)
        new_start = int(hm.group(1))
        hunks.append((new_start, chunk))

    low_hunks = [h for s, h in hunks if s < threshold]
    high_hunks = [h for s, h in hunks if s >= threshold]

    if low_hunks:
        low_out.append(header + "".join(low_hunks))
    if high_hunks:
        high_out.append(header + "".join(high_hunks))

with open(low_path, "w", encoding="utf-8", newline="\n") as f:
    f.write("".join(low_out))
with open(high_path, "w", encoding="utf-8", newline="\n") as f:
    f.write("".join(high_out))

print(f"wrote {low_path} ({sum(1 for _ in low_out)} file blocks)")
print(f"wrote {high_path} ({sum(1 for _ in high_out)} file blocks)")
