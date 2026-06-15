#!/usr/bin/env python3
"""Replace the IC `40` row in docs/ic_support_matrix.md with the HEAD version,
leaving every other line untouched."""
import subprocess
from pathlib import Path

repo = Path("E:/work/dlms")
matrix = repo / "docs/ic_support_matrix.md"

head_text = subprocess.check_output(
    ["git", "-C", str(repo), "show", "HEAD:docs/ic_support_matrix.md"],
    text=True,
    encoding="utf-8",
)
head_row = next(
    (line for line in head_text.splitlines() if line.startswith("| `40`")),
    None,
)
assert head_row is not None, "no IC 40 row at HEAD"

current_bytes = matrix.read_bytes()
current_lines = current_bytes.decode("utf-8").splitlines(keepends=True)
out = []
replaced = 0
for line in current_lines:
    if line.startswith("| `40`"):
        nl = "\n" if line.endswith("\n") else ""
        out.append(head_row + nl)
        replaced += 1
    else:
        out.append(line)

assert replaced == 1, f"expected 1 IC 40 row, replaced {replaced}"
matrix.write_bytes("".join(out).encode("utf-8"))
print("ok")
