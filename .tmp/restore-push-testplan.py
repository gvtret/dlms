#!/usr/bin/env python3
"""Restore the Push Setup test-plan section in
lib/dlms-cosem/docs/03_cosem_test_plan.md to its HEAD version. Leaves all
other content (S-FSK MAC edits) untouched."""
import subprocess
from pathlib import Path

repo = Path("E:/work/dlms")
rel = "lib/dlms-cosem/docs/03_cosem_test_plan.md"
target = repo / rel

head_text = subprocess.check_output(
    ["git", "-C", str(repo), "show", f"HEAD:{rel}"], text=True, encoding="utf-8"
)

START = "Push Setup tests:"
NEXT = "Disconnect Control tests:"


def grab(text):
    s = text.index(START)
    e = text.index(NEXT, s)
    return text[s:e]


head_block = grab(head_text)
cur_text = target.read_text(encoding="utf-8")
cur_block = grab(cur_text)

assert cur_block != head_block
new_text = cur_text.replace(cur_block, head_block, 1)
target.write_text(new_text, encoding="utf-8")
print(f"replaced {len(cur_block)} chars with {len(head_block)} chars")
