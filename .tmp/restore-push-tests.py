#!/usr/bin/env python3
"""Restore the Push Setup TEST(...) block in test_simple_objects.cpp to its
HEAD version. The Push Setup test region is bounded by the namespace closer
right before TEST(CosemPushSetupObject, V1ExposesAllAttributes) and by the
next "namespace {" block that introduces the Disconnect Control fixtures.

Strategy: locate the same anchors in both HEAD and current contents and
swap the slice. This leaves S-FSK MAC and IC 51 edits (further down the
file) untouched."""
import subprocess
from pathlib import Path

repo = Path("E:/work/dlms")
rel = "lib/dlms-cosem/test/cosem/test_simple_objects.cpp"
target = repo / rel

head_text = subprocess.check_output(
    ["git", "-C", str(repo), "show", f"HEAD:{rel}"], text=True, encoding="utf-8"
)
cur_text = target.read_text(encoding="utf-8")

# Anchors: first TEST in Push Setup suite, and the test name that follows
# the Push Setup suite. In HEAD the Push Setup tests are named V1Exposes...
# In current text they're V2Exposes... We bracket by the *first* TEST in the
# suite (which has identical-ish prefix) and by the first test name that
# clearly belongs to the next IC: "TEST(CosemDisconnectControlObject".

NEXT = "TEST(CosemDisconnectControlObject"

# Start anchor: first "TEST(CosemPushSetupObject"
def slice_block(text):
    s = text.index("TEST(CosemPushSetupObject")
    e = text.index(NEXT, s)
    return s, e

hs, he = slice_block(head_text)
cs, ce = slice_block(cur_text)

head_block = head_text[hs:he]
cur_block = cur_text[cs:ce]

assert head_block != cur_block
new_text = cur_text[:cs] + head_block + cur_text[ce:]
target.write_text(new_text, encoding="utf-8")
print(f"replaced {len(cur_block)} chars with {len(head_block)} chars")
