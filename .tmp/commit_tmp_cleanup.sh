#!/usr/bin/env bash
set -euo pipefail
cd /e/work/dlms
git add -A .tmp/
git status --short
echo ---
git commit -m "chore(tmp): prune historical one-shot scripts, keep live wrappers

Drops 73 stale .tmp/ files left over from past sessions: build/cxx/manual
logs, ic-specific patch/restore python scripts, run-* test wrappers,
audit batches, message drafts and a stray dbg.obj.

Keeps the seven reusable wrappers that current work depends on
(build_ninja.sh, test_all.sh, test_prime.sh, env_check.sh,
compile_one.sh, inspect.sh, rename_netstats.sh) and the patches/wip
subdirs.

Adds ic_inventory4.sh which extends the previous inventory scripts to
recognise diapason-form InvokeMethod bodies (methodId >= kX && methodId
<= kY), so future audits do not flag classes like Clock,
AssociationLn or SecuritySetup as missing methods when they are in
fact handled by a range comparison."
echo ---
git log --oneline -3
