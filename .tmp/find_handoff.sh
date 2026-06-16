#!/usr/bin/env bash
set -euo pipefail
grep -n '^## ' /e/work/dlms/handoff.md | head -10
