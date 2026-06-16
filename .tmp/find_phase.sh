#!/usr/bin/env bash
set -euo pipefail
grep -n '^## Phase\|^## 0\.95\|^## 0\.96' /e/work/dlms/handoff.md | tail -20
