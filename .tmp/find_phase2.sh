#!/usr/bin/env bash
set -euo pipefail
grep -n '^## Phase ' /e/work/dlms/handoff.md | tail -20
