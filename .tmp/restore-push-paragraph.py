#!/usr/bin/env python3
"""Restore the Push Setup paragraph in lib/dlms-cosem/docs/01_cosem_api.md
to its HEAD version. Leaves all other content (including S-FSK MAC edits
further down the file) untouched."""
import subprocess
from pathlib import Path

repo = Path("E:/work/dlms")
target = repo / "lib/dlms-cosem/docs/01_cosem_api.md"
rel = "lib/dlms-cosem/docs/01_cosem_api.md"

head_text = subprocess.check_output(
    ["git", "-C", str(repo), "show", f"HEAD:{rel}"], text=True, encoding="utf-8"
)

# Find the Push Setup paragraph boundaries (start: header line; end: blank line
# before the next "`simple_objects.hpp` also exposes" or other top-level chunk).
START = "`simple_objects.hpp` also exposes a partial Push Setup IC `40`"
# The paragraph ends at the next "`simple_objects.hpp` also exposes" intro
# (the next IC block: Disconnect Control).
NEXT = "`simple_objects.hpp` also exposes a partial Disconnect Control"


def grab(text):
    s = text.index(START)
    e = text.index(NEXT, s)
    return text[s:e]


head_para = grab(head_text)
cur_text = target.read_text(encoding="utf-8")
cur_para = grab(cur_text)

assert cur_para != head_para, "no diff in Push Setup paragraph"

new_text = cur_text.replace(cur_para, head_para, 1)
target.write_text(new_text, encoding="utf-8")
print(f"replaced {len(cur_para)} chars with {len(head_para)} chars")
